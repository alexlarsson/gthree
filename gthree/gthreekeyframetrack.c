#include <math.h>

#include "gthreekeyframetrack.h"
#include "gthreeattribute.h"
#include "gthreediscreteinterpolant.h"
#include "gthreelinearinterpolant.h"
#include "gthreecubicinterpolant.h"


typedef struct {
  char *name;
  GthreeAttributeArray *times;
  GthreeAttributeArray *values;
  GthreeInterpolationMode interpolation;
} GthreeKeyframeTrackPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeKeyframeTrack, gthree_keyframe_track, G_TYPE_OBJECT)

static void
gthree_keyframe_track_init (GthreeKeyframeTrack *keyframe_track)
{
}

static void
gthree_keyframe_track_finalize (GObject *obj)
{
  GthreeKeyframeTrack *track = GTHREE_KEYFRAME_TRACK (obj);
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);

  g_free (priv->name);
  gthree_attribute_array_unref (priv->times);
  gthree_attribute_array_unref (priv->values);

  G_OBJECT_CLASS (gthree_keyframe_track_parent_class)->finalize (obj);
}

const char *
gthree_keyframe_track_get_name (GthreeKeyframeTrack     *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  return priv->name;
}

GthreeAttributeArray *
gthree_keyframe_track_get_times (GthreeKeyframeTrack     *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  return priv->times;
}
GthreeAttributeArray *
gthree_keyframe_track_get_values (GthreeKeyframeTrack     *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  return priv->values;
}

static GthreeInterpolant *
create_discrete_interpolant (GthreeKeyframeTrack *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);

  return gthree_discrete_interpolant_new (priv->times,
                                          priv->values);
}

static GthreeInterpolant *
create_linear_interpolant (GthreeKeyframeTrack *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);

  return gthree_linear_interpolant_new (priv->times,
                                        priv->values);
}

static GthreeInterpolant *
create_smooth_interpolant (GthreeKeyframeTrack *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);

  return gthree_cubic_interpolant_new (priv->times,
                                       priv->values);
}

static void
gthree_keyframe_track_class_init (GthreeKeyframeTrackClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_keyframe_track_finalize;

  klass->default_interpolation_mode = GTHREE_INTERPOLATION_MODE_LINEAR;
  klass->create_discrete_interpolant = create_discrete_interpolant;
  klass->create_linear_interpolant = create_linear_interpolant;
  klass->create_smooth_interpolant = create_smooth_interpolant;
}

void
gthree_keyframe_track_set_interpolation (GthreeKeyframeTrack *track,
                                         GthreeInterpolationMode interpolation)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  priv->interpolation = interpolation;
}

void
gthree_keyframe_track_shift (GthreeKeyframeTrack *track,
                             float time_offset)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  int i, n_times;
  float *times;

  times = gthree_attribute_array_peek_float (priv->times);
  n_times = gthree_attribute_array_get_count (priv->times);

  for (i = 0; i < n_times; i++)
    times[i] += time_offset;
}

void
gthree_keyframe_track_scale (GthreeKeyframeTrack *track,
                             float time_scale)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  int i, n_times;
  float *times;

  times = gthree_attribute_array_peek_float (priv->times);
  n_times = gthree_attribute_array_get_count (priv->times);

  for (i = 0; i < n_times; i++)
    times[i] *= time_scale;
}

void
gthree_keyframe_track_trim (GthreeKeyframeTrack *track,
                            float start_time,
                            float end_time)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  GthreeAttributeArray *new_times, *new_values;
  float *times;
  int from, to, n_times;

  times = gthree_attribute_array_peek_float (priv->times);
  n_times = gthree_attribute_array_get_count (priv->times);

  from = 0;
  to = n_times - 1;

  while (from != n_times && times[from] < start_time )
    ++from;

  while (to != -1 && times[to] > end_time)
    --to;

  ++to; // inclusive -> exclusive bound

  if (from != 0 || to != n_times)
    {
      // empty tracks are forbidden, so keep at least one keyframe
      if (from >= to)
        {
          to = MAX (to, 1);
          from = to - 1;
        }

      new_times = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_FLOAT, to - from, 1);
      gthree_attribute_array_copy_at (new_times, 0, 0,
                                      priv->times, from, 0,
                                      1, to - from);
      gthree_attribute_array_unref (priv->times);
      priv->times = new_times;

      new_values = gthree_attribute_array_new (gthree_attribute_array_get_attribute_type (priv->values),
                                               to - from,
                                               gthree_attribute_array_get_stride (priv->values));
      gthree_attribute_array_copy_at (new_values, 0, 0,
                                      priv->values, from, 0,
                                      gthree_attribute_array_get_stride (priv->values), to - from);
      gthree_attribute_array_unref (priv->values);
      priv->values = new_values;
    }
}

// removes equivalent sequential keys as common in morph target sequences
// (0,0,0,0,1,1,1,0,0,0,0,0,0,0) --> (0,0,1,1,0,0)
void
gthree_keyframe_track_optimize (GthreeKeyframeTrack *track)
{
  g_warning ("TODO: gthree_keyframe_optimize()");
}

GthreeInterpolant *
gthree_keyframe_track_create_interpolant (GthreeKeyframeTrack *track)
{
  GthreeKeyframeTrackPrivate *priv = gthree_keyframe_track_get_instance_private (track);
  GthreeKeyframeTrackClass *class = GTHREE_KEYFRAME_TRACK_GET_CLASS(track);

  switch (priv->interpolation)
    {
    case GTHREE_INTERPOLATION_MODE_DISCRETE:
      return class->create_discrete_interpolant (track);
    case GTHREE_INTERPOLATION_MODE_LINEAR:
      return class->create_linear_interpolant (track);
    case GTHREE_INTERPOLATION_MODE_SMOOTH:
      return class->create_smooth_interpolant (track);
    default:
      g_assert_not_reached ();
    }
}

GthreeKeyframeTrack *
gthree_keyframe_track_create  (GType type,
                               const char *name,
                               GthreeAttributeArray *times,
                               GthreeAttributeArray *values)
{
  GthreeKeyframeTrack *track;
  GthreeKeyframeTrackPrivate *priv;

  track = g_object_new (type, NULL);
  priv = gthree_keyframe_track_get_instance_private (track);

  /* TODO: Convert arrays to right types */
  priv->name = g_strdup (name);
  priv->times = gthree_attribute_array_ref (times);
  priv->values = gthree_attribute_array_ref (values);

  priv->interpolation = GTHREE_KEYFRAME_TRACK_GET_CLASS (track)->default_interpolation_mode;

  return track;
}
