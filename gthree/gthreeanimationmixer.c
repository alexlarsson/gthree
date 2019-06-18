#include <math.h>

#include "gthreeanimationmixer.h"
#include "gthreeanimationaction.h"
#include "gthreeanimationclip.h"
#include "gthreepropertymixerprivate.h"
#include "gthreelinearinterpolant.h"
#include "gthreeprivate.h"


typedef struct {
  GPtrArray *known_actions; // Array< AnimationAction > - used as prototypes
  GHashTable *action_by_root; // AnimationAction - lookup
} ClipInfo;

static ClipInfo *
clip_info_new (void)
{
  ClipInfo *info = g_new0 (ClipInfo, 1);
  info->known_actions = g_ptr_array_new_with_free_func (g_object_unref);
  info->action_by_root = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, g_object_unref);
  return info;
}

static void
clip_info_free (ClipInfo *info)
{
  g_ptr_array_unref (info->known_actions);
  if (info->action_by_root)
    g_hash_table_unref (info->action_by_root);
  g_free (info);
}

typedef struct {
  GthreeObject *root;
  int accu_index;
  float time;
  float time_scale;

  /* memory manager */
  GPtrArray *actions;  // 'n_active_actions' followed by inactive ones
  int n_active_actions;
  GHashTable *actions_by_clip; // ClipInfo

  GPtrArray *bindings;  // n_active_binding GthreePropertyMixer followed by inactive ones
  int n_active_bindings;
  GHashTable *bindings_by_root_and_name; // root->(string->GthreePropertyMixer)

  GPtrArray *control_interpolants;
  int n_active_control_interpolants;
} GthreeAnimationMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeAnimationMixer, gthree_animation_mixer, G_TYPE_OBJECT)

void
_gthree_animation_mixer_init_action_data (GthreeAnimationActionMixerData *data)
{
  data->cache_index = -1;
  data->by_clip_cache_index = -1;
}

static void
gthree_animation_mixer_init (GthreeAnimationMixer *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  priv->time_scale = 1.0;
  priv->time = 0;
  priv->accu_index = 0;

  priv->actions = g_ptr_array_new_with_free_func (g_object_unref);
  priv->actions_by_clip = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, (GDestroyNotify) clip_info_free);

  priv->bindings = g_ptr_array_new_with_free_func (g_object_unref);
  priv->bindings_by_root_and_name = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, (GDestroyNotify)g_hash_table_unref);

  priv->control_interpolants = g_ptr_array_new_with_free_func (g_object_unref);
}

static void
gthree_animation_mixer_finalize (GObject *obj)
{
  GthreeAnimationMixer *mixer = GTHREE_ANIMATION_MIXER (obj);
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  g_clear_object (&priv->root);

  g_ptr_array_unref (priv->actions);
  g_hash_table_unref (priv->actions_by_clip);

  g_ptr_array_unref (priv->bindings);
  g_hash_table_unref (priv->bindings_by_root_and_name);

  g_ptr_array_unref (priv->control_interpolants);

  G_OBJECT_CLASS (gthree_animation_mixer_parent_class)->finalize (obj);
}

static void
gthree_animation_mixer_class_init (GthreeAnimationMixerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_animation_mixer_finalize;
}

GthreeAnimationMixer *
gthree_animation_mixer_new (GthreeObject *root)
{
  GthreeAnimationMixer *mixer = g_object_new (GTHREE_TYPE_ANIMATION_MIXER, NULL);
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  priv->root = g_object_ref (root);

  return mixer;
}

gboolean
_gthree_animation_mixer_is_active_action (GthreeAnimationMixer  *mixer,
                                          GthreeAnimationAction *action)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GthreeAnimationActionMixerData *action_data = _gthree_animation_action_get_mixer_data (action);
  int index = action_data->cache_index;

  return index != -1 && index < priv->n_active_actions;
}

static void
_gthree_animation_mixer_add_inactive_binding (GthreeAnimationMixer  *mixer,
                                              GthreePropertyMixer *binding,
                                              GthreeObject *root,
                                              const char *track_name)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GHashTable *bindings_by_name;

  bindings_by_name = g_hash_table_lookup (priv->bindings_by_root_and_name, root);
  if (bindings_by_name == NULL)
    {
      bindings_by_name = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
      g_hash_table_insert (priv->bindings_by_root_and_name, g_object_ref (root), bindings_by_name);
    }

  g_hash_table_insert (bindings_by_name, g_strdup (track_name), g_object_ref (binding));

  binding->cache_index = priv->bindings->len;
  g_ptr_array_add (priv->bindings, g_object_ref (binding));
}

static void
_gthree_animation_mixer_remove_inactive_binding (GthreeAnimationMixer  *mixer,
                                                 GthreePropertyMixer *binding)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  g_warning ("TODO");
}

static void
_gthree_animation_mixer_add_inactive_action (GthreeAnimationMixer  *mixer,
                                             GthreeAnimationAction *action,
                                             GthreeAnimationClip *clip,
                                             GthreeObject *root)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  ClipInfo *clip_info = g_hash_table_lookup (priv->actions_by_clip, clip);
  GthreeAnimationActionMixerData *action_data = _gthree_animation_action_get_mixer_data (action);

  if (clip_info == NULL)
    {
      clip_info = clip_info_new ();
      g_ptr_array_add (clip_info->known_actions, g_object_ref (action));
      action_data->by_clip_cache_index = 0;
      g_hash_table_insert (priv->actions_by_clip, g_object_ref (clip), clip_info);
    }
  else
    {
      action_data->by_clip_cache_index = clip_info->known_actions->len;
      g_ptr_array_add (clip_info->known_actions, g_object_ref (action));
    }

  action_data->cache_index = priv->actions->len;
  g_ptr_array_add (priv->actions, g_object_ref (action));
  g_hash_table_insert (clip_info->action_by_root, g_object_ref (root), g_object_ref (action));
}

static void
_gthree_animation_mixer_remove_inactive_bindings_for_action (GthreeAnimationMixer  *mixer,
                                                             GthreeAnimationAction *action)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  g_warning ("TODO");
}


static void
_gthree_animation_mixer_remove_inactive_action (GthreeAnimationMixer  *mixer,
                                                GthreeAnimationAction *action)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  g_warning ("TODO");
}

static void
_gthree_animation_mixer_bind_action (GthreeAnimationMixer  *mixer,
                                     GthreeAnimationAction *action,
                                     GthreeAnimationAction *prototype_action)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GthreeObject *root = gthree_animation_action_get_root (action);
  GthreeAnimationClip *clip = gthree_animation_action_get_clip (action);
  int n_tracks = gthree_animation_clip_get_n_tracks (clip);
  GPtrArray *bindings =  _gthree_animation_action_get_property_bindings (action);
  GHashTable *bindings_by_name;
  int i;

  bindings_by_name = g_hash_table_lookup (priv->bindings_by_root_and_name, root);
  if (bindings_by_name == NULL)
    {
      bindings_by_name = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
      g_hash_table_insert (priv->bindings_by_root_and_name, g_object_ref (root), bindings_by_name);
    }

  for (i = 0; i < n_tracks; i++)
    {
      GthreeKeyframeTrack *track = gthree_animation_clip_get_track (clip, i);
      const char *track_name = gthree_animation_clip_get_name (clip);
      GthreePropertyMixer *binding;

      binding = g_hash_table_lookup (bindings_by_name, track_name);

      if (binding)
        {
          GthreePropertyMixer *old_binding = g_ptr_array_index (bindings, i);
          g_ptr_array_index (bindings, i) = g_object_ref (binding);
          if (old_binding)
            g_object_unref (old_binding);
        }
      else
        {
          GthreeParsedPath *path = NULL;
          g_autoptr(GthreePropertyBinding) new_binding = NULL;
          g_autoptr(GthreePropertyMixer) new_mixer = NULL;
          GthreePropertyMixer *old_binding;

          binding = g_ptr_array_index (bindings, i);
          if (binding != NULL)
            {
              // existing binding, make sure the cache knows
              if (binding->cache_index == -1)
                {
                  binding->reference_count++;
                  _gthree_animation_mixer_add_inactive_binding (mixer, binding, root, track_name);
                }
              continue;
            }

          if (prototype_action)
            {
              GPtrArray *prototype_bindings =  _gthree_animation_action_get_property_bindings (prototype_action);
              GthreePropertyMixer *m = g_ptr_array_index (prototype_bindings, i);
              GthreePropertyBinding *b = gthree_property_mixer_get_binding (m);

              path = gthree_property_binding_get_parsed_path (b);
            }

          new_binding = gthree_property_binding_new (root, track_name, path);
          new_mixer = gthree_property_mixer_new (new_binding,
                                                 gthree_keyframe_track_get_value_type (track),
                                                 gthree_keyframe_track_get_value_size (track));
          binding = new_mixer;

          binding->reference_count++;
          _gthree_animation_mixer_add_inactive_binding (mixer, binding, root, track_name);

          old_binding = g_ptr_array_index (bindings, i);
          g_ptr_array_index (bindings, i) = g_object_ref (binding);
          if (old_binding)
            g_object_unref (old_binding);
        }
    }
}

void
_gthree_animation_mixer_lend_binding (GthreeAnimationMixer  *mixer,
                                      GthreePropertyMixer *binding)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  int prev_index, last_active_index;
  GthreePropertyMixer *first_inactive_binding;

  // Note: this just moves the bindings around, so no ref/unref needed

  prev_index = binding->cache_index;
  last_active_index = priv->n_active_bindings++;

  first_inactive_binding = g_ptr_array_index (priv->bindings, last_active_index);

  binding->cache_index = last_active_index;
  g_ptr_array_index (priv->bindings, last_active_index) = binding;

  first_inactive_binding->cache_index = prev_index;
  g_ptr_array_index (priv->bindings, prev_index) = first_inactive_binding;
}

void
_gthree_animation_mixer_take_back_binding (GthreeAnimationMixer  *mixer,
                                           GthreePropertyMixer *binding)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  int prev_index, first_inactive_index;
  GthreePropertyMixer *last_active_binding;

  // Note: this just moves the bindings around, so no ref/unref needed

  prev_index = binding->cache_index;
  first_inactive_index = -- priv->n_active_bindings;

  last_active_binding = g_ptr_array_index (priv->bindings, first_inactive_index);

  binding->cache_index = first_inactive_index;
  g_ptr_array_index (priv->bindings, first_inactive_index) = binding;

  last_active_binding->cache_index = prev_index;
  g_ptr_array_index (priv->bindings, prev_index) = last_active_binding;
}

void
_gthree_animation_mixer_lend_action (GthreeAnimationMixer  *mixer,
                                     GthreeAnimationAction *action)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GthreeAnimationActionMixerData *action_data = _gthree_animation_action_get_mixer_data (action);
  int prev_index, last_active_index;
  GthreeAnimationAction *first_inactive_action;
  GthreeAnimationActionMixerData *first_inactive_action_data;

  // [ active actions |  inactive actions  ]
  // [  active actions >| inactive actions ]
  //                 s        a
  //                  <-swap->
  //                 a        s
  //
  // Note: this just moves the actions around, so no ref/unref needed

  prev_index = action_data->cache_index;
  last_active_index = priv->n_active_actions++;

  first_inactive_action = g_ptr_array_index (priv->actions, last_active_index);
  first_inactive_action_data = _gthree_animation_action_get_mixer_data (first_inactive_action);

  action_data->cache_index = last_active_index;
  g_ptr_array_index (priv->actions, last_active_index) = action;

  first_inactive_action_data->cache_index = prev_index;
  g_ptr_array_index (priv->actions, prev_index) = first_inactive_action;
}

void
_gthree_animation_mixer_take_back_action (GthreeAnimationMixer  *mixer,
                                          GthreeAnimationAction *action)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GthreeAnimationActionMixerData *action_data = _gthree_animation_action_get_mixer_data (action);
  int prev_index, first_inactive_index;
  GthreeAnimationAction *last_active_action;
  GthreeAnimationActionMixerData *last_active_action_data;

  // [  active actions  | inactive actions ]
  // [ active actions |< inactive actions  ]
  //        a        s
  //         <-swap->
  //        s        a
  //
  // Note: this just moves the actions around, so no ref/unref needed

  prev_index = action_data->cache_index;
  first_inactive_index = -- priv->n_active_actions;

  last_active_action = g_ptr_array_index (priv->actions, first_inactive_index);
  last_active_action_data = _gthree_animation_action_get_mixer_data (last_active_action);

  action_data->cache_index = first_inactive_index;
  g_ptr_array_index (priv->actions, first_inactive_index) = action;

  last_active_action_data->cache_index = prev_index;
  g_ptr_array_index (priv->actions, prev_index) = last_active_action;
}


GthreeInterpolant *
_gthree_animation_mixer_lend_control_interpolant (GthreeAnimationMixer  *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  int last_active_index = priv->n_active_control_interpolants++;
  GthreeInterpolant *interpolant;

  if (last_active_index < priv->control_interpolants->len)
    {
      interpolant = g_ptr_array_index (priv->control_interpolants, last_active_index);
    }
  else
    {
      g_autoptr(GthreeAttributeArray) times = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_FLOAT, 2, 1);
      g_autoptr(GthreeAttributeArray) values = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_FLOAT, 2, 1);

      interpolant = gthree_linear_interpolant_new (times, values);

      _gthree_interpolant_set_cache_index (interpolant, last_active_index);
      g_ptr_array_add (priv->control_interpolants, interpolant);
    }

  return interpolant;
}

void
_gthree_animation_mixer_take_back_control_interpolant (GthreeAnimationMixer  *mixer,
                                                       GthreeInterpolant     *interpolant)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  int prev_index, first_inactive_index;
  GthreeInterpolant *last_active_interpolant;

  prev_index = _gthree_interpolant_get_cache_index (interpolant);
  first_inactive_index = -- priv->n_active_control_interpolants;
  last_active_interpolant = g_ptr_array_index (priv->control_interpolants, first_inactive_index);

  _gthree_interpolant_set_cache_index (interpolant, first_inactive_index);
  g_ptr_array_index (priv->control_interpolants, first_inactive_index) = interpolant;

  _gthree_interpolant_set_cache_index (last_active_interpolant, prev_index);
  g_ptr_array_index (priv->control_interpolants, prev_index) = last_active_interpolant;
}


void
_gthree_animation_mixer_activate_action (GthreeAnimationMixer  *mixer,
                                         GthreeAnimationAction *action)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  if (!_gthree_animation_mixer_is_active_action (mixer, action))
    {
      GthreeAnimationActionMixerData *action_data = _gthree_animation_action_get_mixer_data (action);
      GPtrArray *bindings;
      int i;

      if (action_data->cache_index == -1)
        {
          // this action has been forgotten by the cache, but the user
          // appears to be still using it -> rebind
          GthreeObject *root = gthree_animation_action_get_root (action);
          GthreeAnimationClip *clip = gthree_animation_action_get_clip (action);
          ClipInfo *clip_info = g_hash_table_lookup (priv->actions_by_clip, clip);
          GthreeAnimationAction *prototype_action = NULL;

          if (clip_info && clip_info->known_actions->len > 0)
            prototype_action = g_ptr_array_index (clip_info->known_actions, 0);

          _gthree_animation_mixer_bind_action (mixer, action, prototype_action);
          _gthree_animation_mixer_add_inactive_action (mixer, action, clip, root);
        }

      // increment reference counts / sort out state
      bindings = _gthree_animation_action_get_property_bindings (action);
      for (i = 0; i < bindings->len; i++)
        {
          GthreePropertyMixer *binding = g_ptr_array_index (bindings, i);
          if (binding->use_count++ == 0)
            {
              _gthree_animation_mixer_lend_binding (mixer, binding);
              gthree_property_mixer_save_original_state (binding);
            }
        }
      _gthree_animation_mixer_lend_action (mixer, action);
    }
}

void
_gthree_animation_mixer_deactivate_action (GthreeAnimationMixer  *mixer,
                                           GthreeAnimationAction *action)
{
  if (_gthree_animation_mixer_is_active_action (mixer, action))
    {
      GPtrArray *bindings;
      int i;

      // decrement reference counts / sort out state
      bindings = _gthree_animation_action_get_property_bindings (action);
      for (i = 0; i < bindings->len; i++)
        {
          GthreePropertyMixer *binding = g_ptr_array_index (bindings, i);
          if ( -- binding->use_count == 0 )
            {
              gthree_property_mixer_restore_original_state (binding);
              _gthree_animation_mixer_take_back_binding (mixer, binding);
            }
        }

      _gthree_animation_mixer_take_back_action (mixer, action);
    }
}

//TODO:
//_controlInterpolantsResultBuffer: new Float32Array( 1 ),

// return an action for a clip optionally using a custom root target
// object (this method allocates a lot of dynamic memory in case a
// previously unknown clip/root combination is specified)
GthreeAnimationAction *
gthree_animation_mixer_clip_action (GthreeAnimationMixer *mixer,
                                    GthreeAnimationClip *clip,
                                    GthreeObject *optional_root)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GthreeObject *root;
  ClipInfo *clip_info;
  GthreeAnimationAction *prototype_action = NULL;
  g_autoptr(GthreeAnimationAction) new_action = NULL;

  root = optional_root ? optional_root : priv->root;
  clip_info = g_hash_table_lookup (priv->actions_by_clip, clip);

  if (clip_info)
    {
      GthreeAnimationAction *existing_action =
        g_hash_table_lookup (clip_info->action_by_root, root);

      if (existing_action)
        return existing_action;

      // we know the clip, so we don't have to parse all
      // the bindings again but can just copy
      prototype_action = g_ptr_array_index (clip_info->known_actions, 0);
  }

  // allocate all resources required to run it
  new_action = gthree_animation_action_new (mixer, clip, optional_root);

  _gthree_animation_mixer_bind_action (mixer, new_action, prototype_action);

  // and make the action known to the memory manager
  _gthree_animation_mixer_add_inactive_action (mixer, new_action, clip, root);

  return g_steal_pointer (&new_action);
}

// get an existing action
GthreeAnimationAction *
gthree_animation_mixer_existing_action (GthreeAnimationMixer  *mixer,
                                        GthreeAnimationClip *clip,
                                        GthreeObject *optional_root)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  GthreeObject *root;
  ClipInfo *clip_info;
  g_autoptr(GthreeAnimationAction) new_action = NULL;

  root = optional_root ? optional_root : priv->root;
  clip_info = g_hash_table_lookup (priv->actions_by_clip, clip);

  if (clip_info)
    {
      GthreeAnimationAction *existing_action =
        g_hash_table_lookup (clip_info->action_by_root, root);

      if (existing_action)
        return existing_action;
    }

  return NULL;
}

// deactivates all previously scheduled actionsG
void
gthree_animation_mixer_stop_all_action (GthreeAnimationMixer  *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  int n_actions = priv->n_active_actions;
  int n_bindings = priv->n_active_bindings;
  int i;

  priv->n_active_actions = 0;
  priv->n_active_bindings = 0;

  for (i = 0; i < n_actions; i++)
    {
      GthreeAnimationAction *action = g_ptr_array_index (priv->actions, i);
      gthree_animation_action_reset (action);
    }

  for (i = 0; i < n_bindings; i++)
    {
      GthreePropertyMixer *binding = g_ptr_array_index (priv->bindings, i);
      binding->use_count = 0;
    }
}

// advance the time and update apply the animation
void
gthree_animation_mixer_update (GthreeAnimationMixer  *mixer,
                               float delta_time)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  int n_actions = priv->n_active_actions;
  int n_bindings = priv->n_active_bindings;
  float time, time_direction;
  int accu_index, i;

  delta_time *= priv->time_scale;

  priv->time += delta_time;
  time = priv->time;

  if (delta_time >= 0)
    time_direction = 1;
  else
    time_direction = -1;

  priv->accu_index ^= 1;
  accu_index = priv->accu_index;

  // run active actions
  for (i = 0; i < n_actions; i++)
    {
      GthreeAnimationAction *action = g_ptr_array_index (priv->actions, i);
      _gthree_animation_action_update (action, time, delta_time, time_direction, accu_index);
    }

  // update scene graph
  for (i = 0; i < n_bindings; i++)
    {
      GthreePropertyMixer *binding = g_ptr_array_index (priv->bindings, i);

      gthree_property_mixer_apply (binding, accu_index);
    }
}

// free all resources specific to a particular clip
void
gthree_animation_mixer_uncache_clip (GthreeAnimationMixer  *mixer,
                                     GthreeAnimationClip *clip)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  g_warning ("TODO");
}

// free all resources specific to a particular root target object
void
gthree_animation_mixer_uncache_root (GthreeAnimationMixer  *mixer,
                                     GthreeObject *object)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  g_warning ("TODO");
}

// remove a targeted clip from the cache
void
gthree_animation_mixer_uncache_action (GthreeAnimationMixer  *mixer,
                                       GthreeAnimationClip *clip,
                                       GthreeObject *optional_root)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
  g_warning ("TODO");
}

float
gthree_animation_mixer_get_time (GthreeAnimationMixer  *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  return priv->time;
}

// return this mixer's root target object
GthreeObject *
gthree_animation_mixer_get_root (GthreeAnimationMixer  *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  return priv->root;
}

void
_gthree_animation_mixer_displatch_event (GthreeAnimationMixer  *mixer,
                                         const char *type,
                                         ...)
{
  g_warning ("TODO");
}
