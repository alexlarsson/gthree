#include <math.h>

#include "gthreeanimationaction.h"
#include "gthreeanimationmixer.h"
#include "gthreeprivate.h"


typedef struct {
  GthreeAnimationMixer *mixer;
  GthreeAnimationClip *clip;
  GthreeObject *local_root;
  GPtrArray *interpolants; // GthreeInterpolant
  GPtrArray *property_bindings; // GthreePropertyMixer

  GthreeInterpolantSettings *interpolant_settings;
  GthreeInterpolant *time_scale_interpolant;
  GthreeInterpolant *weight_interpolant;
  GthreeLoopMode loop_mode;
  int loop_count;

  // global mixer time when the action is to be started
  // unset upon start of the action
  float start_time;
  gboolean start_time_set;

  // scaled local time of the action
  // gets clamped or wrapped to 0..clip.duration according to loop
  float time;

  float time_scale;
  float effective_time_scale;

  float weight;
  float effective_weight;

  // no. of repetitions when looping
  int repetitions; // <0 == infinite

  gboolean paused; // true -> zero effective time scale
  gboolean enabled; // false -> zero effective weight

  gboolean clamp_when_finished; // keep feeding the last frame?
  gboolean zero_slope_at_start; // for smooth interpolation w/o separate
  gboolean zero_slope_at_end;   // clips for start, loop and end

  /*
    _cacheIndex = null; // for the memory manager
    _byClipCacheIndex = null; // for the memory manager
  */


} GthreeAnimationActionPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeAnimationAction, gthree_animation_action, G_TYPE_OBJECT)

static void
maybe_object_unref (GObject *object)
{
  if (object)
    g_object_unref (object);
}

static void
gthree_animation_action_init (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  priv->interpolants = g_ptr_array_new_with_free_func (g_object_unref);
  priv->property_bindings = g_ptr_array_new_with_free_func ((GDestroyNotify)maybe_object_unref);

  priv->loop_mode = GTHREE_LOOP_MODE_REPEAT;
  priv->loop_count = -1;

  priv->start_time = 0;
  priv->start_time_set = FALSE;

  priv->time_scale = 1.0;
  priv->effective_time_scale = 1.0;

  priv->weight = 1.0;
  priv->effective_weight = 1.0;

  priv->repetitions = -1;

  priv->paused = FALSE;
  priv->enabled = TRUE;

  priv->clamp_when_finished = FALSE;

  priv->zero_slope_at_end = TRUE;
  priv->zero_slope_at_start = TRUE;

  priv->interpolant_settings = gthree_interpolant_settings_new ();
  gthree_interpolant_settings_set_start_ending_mode (priv->interpolant_settings, GTHREE_ENDING_MODE_ZERO_CURVATURE);
  gthree_interpolant_settings_set_end_ending_mode (priv->interpolant_settings, GTHREE_ENDING_MODE_ZERO_CURVATURE);
}

static void
gthree_animation_action_finalize (GObject *obj)
{
  GthreeAnimationAction *action = GTHREE_ANIMATION_ACTION (obj);
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  g_clear_object (&priv->mixer);
  g_clear_object (&priv->clip);
  g_clear_object (&priv->local_root);

  g_clear_object (&priv->weight_interpolant);
  g_clear_object (&priv->time_scale_interpolant);

  g_ptr_array_unref (priv->interpolants);
  g_ptr_array_unref (priv->property_bindings);

  G_OBJECT_CLASS (gthree_animation_action_parent_class)->finalize (obj);
}

static void
gthree_animation_action_class_init (GthreeAnimationActionClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_animation_action_finalize;
}

GthreeAnimationAction *
gthree_animation_action_new (GthreeAnimationMixer *mixer,
                             GthreeAnimationClip *clip,
                             GthreeObject *local_root)
{
  GthreeAnimationAction *action = g_object_new (GTHREE_TYPE_ANIMATION_ACTION, NULL);
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  int n_tracks, i;

  priv->mixer = g_object_ref (mixer);
  priv->clip = g_object_ref (clip);
  if (local_root)
    priv->local_root = g_object_ref (local_root);

  n_tracks = gthree_animation_clip_get_n_tracks (clip);

  for (i = 0; i < n_tracks; i++)
    {
      GthreeKeyframeTrack *track = gthree_animation_clip_get_track (clip, i);
      GthreeInterpolant *interpolant = gthree_keyframe_track_create_interpolant (track);
      gthree_interpolant_set_settings (interpolant, priv->interpolant_settings);
      g_ptr_array_add (priv->interpolants, interpolant);
    }

  g_ptr_array_set_size (priv->property_bindings, n_tracks);

  return action;
}

void
gthree_animation_action_play (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  gthree_animation_mixer_activate_action (priv->mixer, action);
}


void
gthree_animation_action_stop (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  gthree_animation_mixer_deactivate_action (priv->mixer, action);
}

void
gthree_animation_action_reset (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  priv->paused = FALSE;
  priv->enabled = TRUE;
  priv->time = 0; // restart_clip
  priv->loop_count = -1;
  priv->start_time = 0;
  priv->start_time_set = FALSE;

  gthree_animation_action_stop_fading (action);
  gthree_animation_action_stop_warping (action);
}

gboolean
gthree_animation_action_is_running (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  return
    priv->enabled &&
    !priv->paused &&
    priv->time_scale != 0 &&
    !priv->start_time_set &&
    gthree_animation_mixer_is_active_action (priv->mixer, action);
}

gboolean
gthree_animation_action_is_scheduled (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  return
    gthree_animation_mixer_is_active_action (priv->mixer, action);
}

void
gthree_animation_action_start_at (GthreeAnimationAction *action,
                                  float time)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  priv->start_time = time;
  priv->start_time_set = TRUE;
}

/* < 0 repetitions == infinite */
void
gthree_animation_action_set_loop_mode (GthreeAnimationAction *action,
                                       GthreeLoopMode loop_mode,
                                       int repetitions)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  priv->loop_mode = loop_mode;
  priv->repetitions = repetitions;
}

void
gthree_animation_action_set_effective_weight (GthreeAnimationAction *action,
                                              float weight)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  priv->weight = weight;
  priv->effective_weight = priv->enabled ? weight : 0;

  gthree_animation_action_stop_fading (action);
}

float
gthree_animation_action_get_effective_weight (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  return priv->effective_weight;
}

void
gthree_animation_action_fade_in (GthreeAnimationAction *action, float duration)
{
  _gthree_animation_action_schedule_fading (action, duration, 0, 1);
}

void
gthree_animation_action_fade_out (GthreeAnimationAction *action, float duration)
{
  _gthree_animation_action_schedule_fading (action, duration, 1, 0);
}

void
gthree_animation_action_cross_fade_from (GthreeAnimationAction *action,
                                         GthreeAnimationAction *fade_out_action,
                                         float duration,
                                         gboolean warp)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  GthreeAnimationActionPrivate *fade_out_priv = gthree_animation_action_get_instance_private (fade_out_action);

  gthree_animation_action_fade_out (fade_out_action, duration);
  gthree_animation_action_fade_in (action, duration);

  if (warp)
    {
      float fade_in_duration = gthree_animation_clip_get_duration (priv->clip);
      float fade_out_duration = gthree_animation_clip_get_duration (fade_out_priv->clip);

      float start_end_ratio = fade_out_duration / fade_in_duration;
      float end_start_ratio = fade_in_duration / fade_out_duration;

      gthree_animation_action_warp (fade_out_action, 1.0, start_end_ratio, duration);
      gthree_animation_action_warp (action, end_start_ratio, 1.0, duration);
    }
}

void
gthree_animation_action_cross_fade_to (GthreeAnimationAction *action,
                                       GthreeAnimationAction *fade_in_action,
                                       float duration,
                                       gboolean warp)
{
  gthree_animation_action_cross_fade_from (fade_in_action, action, duration, warp);
}

void
gthree_animation_action_stop_fading (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  GthreeInterpolant *weight_interpolant = priv->weight_interpolant;

  if (weight_interpolant != NULL)
    {
      priv->weight_interpolant = NULL;
      gthree_action_mixer_take_back_control_interpolant (priv->mixer, weight_interpolant);
    }
}

void
gthree_animation_action_set_effective_time_scale (GthreeAnimationAction *action,
                                                  float time_scale)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  priv->time_scale = time_scale;
  priv->effective_time_scale = priv->paused ? 0 : time_scale;

  gthree_animation_action_stop_warping (action);
}

// return the time scale considering warping and .paused
float
gthree_animation_action_get_effective_time_scale (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  return priv->effective_time_scale;
}

void
gthree_animation_action_set_duration (GthreeAnimationAction *action,
                                      float duration)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  priv->time_scale = gthree_animation_clip_get_duration (priv->clip) / duration;

  gthree_animation_action_stop_warping (action);
}

void
gthree_animation_action_sync_with (GthreeAnimationAction *action,
                                   GthreeAnimationAction *other_action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  GthreeAnimationActionPrivate *other_priv = gthree_animation_action_get_instance_private (action);

  priv->time = other_priv->time;
  priv->time_scale = other_priv->time_scale;

  gthree_animation_action_stop_warping (action);
}

void
gthree_animation_action_halt (GthreeAnimationAction *action,
                              float duration)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  gthree_animation_action_warp (action, priv->effective_time_scale, 0, duration);
}

void
gthree_animation_action_warp (GthreeAnimationAction *action,
                              float start_time_scale,
                              float end_time_scale,
                              float duration)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  float now = gthree_action_mixer_get_time (priv->mixer);
  GthreeInterpolant *interpolant = priv->time_scale_interpolant;
  float time_scale = priv->time_scale;
  float *times, *values;

  if (interpolant == NULL)
    {
      interpolant = gthree_action_mixer_lend_control_interpolant (priv->mixer);
      priv->time_scale_interpolant = interpolant;
    }

  times = gthree_attribute_array_peek_float (gthree_interpolant_get_parameter_positions (interpolant));
  values = gthree_attribute_array_peek_float (gthree_interpolant_get_sample_values (interpolant));

  times[0] = now;
  times[1] = now + duration;
  values[0] = start_time_scale / time_scale;
  values[1] = end_time_scale / time_scale;
}

void
gthree_animation_action_stop_warping (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  GthreeInterpolant *time_scale_interpolant = priv->time_scale_interpolant;

  if (time_scale_interpolant)
    {
      priv->time_scale_interpolant = NULL;
      gthree_action_mixer_take_back_control_interpolant (priv->mixer, time_scale_interpolant);
    }

}

GthreeAnimationMixer *
gthree_animation_action_get_mixer (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  return priv->mixer;
}

GthreeAnimationClip *
gthree_animation_action_get_clip (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  return priv->clip;
}

GthreeObject *
gthree_animation_action_get_root (GthreeAnimationAction *action)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  return priv->local_root ? priv->local_root : gthree_action_mixer_get_root (priv->mixer);
}

static void
gthree_property_mixer_accumulate (GthreePropertyMixer *mixer, int accu_index, float weight)
{
  g_warning ("TODO");
}

void
_gthree_animation_action_update (GthreeAnimationAction *action,
                                 float time,
                                 float delta_time,
                                 float time_direction,
                                 int accu_index)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  float start_time;
  float clip_time;
  float weight;
  int i;

  // called by the mixer

  if (!priv->enabled)
    {
      // call ._updateWeight() to update ._effectiveWeight
      _gthree_animation_action_update_weight (action, time);
      return;
    }

  start_time = priv->start_time;
  if (priv->start_time_set)
    {
      // check for scheduled start of action
      float time_running = (time - start_time) * time_direction;
      if (time_running < 0 || time_direction == 0)
        return; // yet to come / don't decide when delta = 0

      // start
      priv->start_time = 0;
      priv->start_time_set = FALSE;
      delta_time = time_direction * time_running;
    }

  // apply time scale and advance time
  delta_time *= _gthree_animation_action_update_time_scale (action, time);
  clip_time = _gthree_animation_action_update_time (action, delta_time);

  // note: update_time may disable the action resulting in
  // an effective weight of 0
  weight = _gthree_animation_action_update_weight (action, time);

  if (weight > 0)
    {
      for (i = 0; i < priv->interpolants->len; i++)
        {
          GthreeInterpolant *interpolant = g_ptr_array_index (priv->interpolants, i);
          GthreePropertyMixer *property_mixer = g_ptr_array_index (priv->property_bindings, i);

          gthree_interpolant_evaluate (interpolant, clip_time);
          gthree_property_mixer_accumulate (property_mixer, accu_index, weight);
        }
    }
}

float
_gthree_animation_action_update_weight (GthreeAnimationAction *action,
                                        float time)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  float weight = 0;
  GthreeInterpolant *interpolant;

  if (priv->enabled)
    {
      weight = priv->weight;
      interpolant = priv->weight_interpolant;
      if (interpolant != NULL)
        {
          GthreeAttributeArray *interpolant_value_array = gthree_interpolant_evaluate (interpolant, time);
          float interpolant_value = gthree_attribute_array_get_float_at (interpolant_value_array, 0, 0);
          GthreeAttributeArray *parameter_positions = gthree_interpolant_get_parameter_positions (interpolant);

          weight *= interpolant_value;

          if (time > gthree_attribute_array_get_float_at (parameter_positions, 1, 0))
            {
              gthree_animation_action_stop_fading (action);
              if (interpolant_value == 0)
                priv->enabled = FALSE; // faded out, disable
            }
        }
    }

  priv->effective_weight = weight;
  return weight;
}

float
_gthree_animation_action_update_time_scale (GthreeAnimationAction *action,
                                            float time)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  float time_scale = 0;
  GthreeInterpolant *interpolant;

  if (!priv->paused)
    {
      time_scale = priv->time_scale;
      interpolant = priv->time_scale_interpolant;
      if (interpolant != NULL)
        {
          GthreeAttributeArray *interpolant_value_array = gthree_interpolant_evaluate (interpolant, time);
          float interpolant_value = gthree_attribute_array_get_float_at (interpolant_value_array, 0, 0);
          GthreeAttributeArray *parameter_positions = gthree_interpolant_get_parameter_positions (interpolant);

          time_scale *= interpolant_value;

          if (time > gthree_attribute_array_get_float_at (parameter_positions, 1, 0))
            {
              gthree_animation_action_stop_warping (action);
              if (time_scale == 0)
                priv->paused = TRUE; // motion has halted, pause
              else
                priv->time_scale = time_scale; // warp done - apply final time scale
            }
        }
    }

  priv->effective_time_scale = time_scale;
  return time_scale;
}

float
_gthree_animation_action_update_time (GthreeAnimationAction *action,
                                      float delta_time)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  float time = priv->time + delta_time;
  float duration = gthree_animation_clip_get_duration (priv->clip);
  GthreeLoopMode loop = priv->loop_mode;
  int loop_count = priv->loop_count;
  gboolean ping_pong = loop == GTHREE_LOOP_MODE_PINGPONG;

  if (delta_time == 0)
    {
      if (loop_count == -1)
        return time;
      return ( ping_pong && (loop_count & 1) == 1) ? duration - time : time;
    }

  if (loop == GTHREE_LOOP_MODE_ONCE)
    {
      if (loop_count == -1)
        {
          // just started
          priv->loop_count = 0;
          _gthree_animation_action_set_endings(action, TRUE, TRUE, FALSE);
        }

      {
        gboolean handled_stop = FALSE;
        if (time >= duration)
          {
            time = duration;
          }
        else if ( time < 0 )
          {
            time = 0;
          }
        else
          {
            priv->time = time;
            handled_stop = TRUE;
          }

        if (!handled_stop)
          {
            if (priv->clamp_when_finished)
              priv->paused = TRUE;
            else
              priv->enabled = FALSE;
            priv->time = time;

            gthree_action_mixer_displatch_event (priv->mixer, "finished"
                                                 // action: this,
                                                 // direction: deltaTime < 0 ? - 1 : 1
                                                 );
          }
      }
    }
  else
    {
      // repetitive Repeat or PingPong
      if (loop_count == -1)
        {
          // just started
          if (delta_time >= 0)
            {
              loop_count = 0;
              _gthree_animation_action_set_endings (action, TRUE, priv->repetitions == 0, ping_pong);
            }
          else
            {
              // when looping in reverse direction, the initial
              // transition through zero counts as a repetition,
              // so leave loopCount at -1
              _gthree_animation_action_set_endings (action, priv->repetitions == 0, TRUE, ping_pong);
            }
        }

      if (time >= duration || time < 0)
        {
          int pending;
          // wrap around
          float loop_delta = floorf (time / duration); // signed
          time -= duration * loop_delta;

          loop_count += (int) fabsf(loop_delta);

          if (priv->repetitions < 0)
            pending = G_MAXINT; // priv->repetitions == -1 => infinite repetitions
          else
            pending = priv->repetitions - loop_count;
          if (pending <= 0)
            {
              // have to stop (switch state, clamp time, fire event)
              if (priv->clamp_when_finished)
                priv->paused = TRUE;
              else
                priv->enabled = FALSE;

              time = delta_time > 0 ? duration : 0;
              priv->time = time;

            gthree_action_mixer_displatch_event (priv->mixer, "finished"
                                                 // action: this,
                                                 // direction: deltaTime > 0 ? 1 : - 1
                                                 );
            }
          else
            {
              // keep running
              if (pending == 1)
                {
                  // entering the last round
                  gboolean at_start = delta_time < 0;
                  _gthree_animation_action_set_endings (action, at_start, !at_start, ping_pong);
                }
              else
                {
                  _gthree_animation_action_set_endings (action, FALSE, FALSE, ping_pong);
                }

              priv->loop_count = loop_count;
              priv->time = time;
              gthree_action_mixer_displatch_event (priv->mixer, "loop"
                                                 // action: this,
                                                 // loopDelta: loopDelta
                                                   );
            }
        }
      else
        {
          priv->time = time;
        }

      if ( ping_pong && (loop_count & 1) == 1)
        {
          // invert time for the "pong round"
          return duration - time;
        }
    }

  return time;
}

void
_gthree_animation_action_set_endings (GthreeAnimationAction *action,
                                      gboolean at_start,
                                      gboolean at_end,
                                      gboolean ping_pong)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  GthreeInterpolantSettings *settings = priv->interpolant_settings;

  if (ping_pong)
    {
      gthree_interpolant_settings_set_start_ending_mode (settings, GTHREE_ENDING_MODE_ZERO_SLOPE);
      gthree_interpolant_settings_set_end_ending_mode (settings, GTHREE_ENDING_MODE_ZERO_SLOPE);
    }
  else
    {
      // assuming for LoopOnce atStart == atEnd == true
      if (at_start)
        gthree_interpolant_settings_set_start_ending_mode (settings,
                                                           priv->zero_slope_at_start ? GTHREE_ENDING_MODE_ZERO_SLOPE : GTHREE_ENDING_MODE_ZERO_CURVATURE);
      else
        gthree_interpolant_settings_set_start_ending_mode (settings,
                                                           GTHREE_ENDING_MODE_WRAP_AROUND);

      if (at_end)
        gthree_interpolant_settings_set_end_ending_mode (settings,
                                                         priv->zero_slope_at_end ? GTHREE_ENDING_MODE_ZERO_SLOPE : GTHREE_ENDING_MODE_ZERO_CURVATURE);
      else
        gthree_interpolant_settings_set_end_ending_mode (settings,
                                                         GTHREE_ENDING_MODE_WRAP_AROUND);
    }
}

void
_gthree_animation_action_schedule_fading (GthreeAnimationAction *action,
                                          float duration,
                                          float weight_now,
                                          float weight_then)
{
  GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
  GthreeInterpolant *interpolant = priv->weight_interpolant;
  float now = gthree_action_mixer_get_time (priv->mixer);
  float *times, *values;

  if (interpolant == NULL)
    {
      interpolant = gthree_action_mixer_lend_control_interpolant (priv->mixer);
      priv->weight_interpolant = interpolant;
    }

  times = gthree_attribute_array_peek_float (gthree_interpolant_get_parameter_positions (interpolant));
  values = gthree_attribute_array_peek_float (gthree_interpolant_get_sample_values (interpolant));

  times[0] = now;
  times[1] = now + duration;
  values[0] = weight_now;
  values[1] = weight_then;
}
