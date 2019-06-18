#include <math.h>

#include "gthreeanimationmixer.h"
#include "gthreeprivate.h"


typedef struct {
  GPtrArray *known_actions; // Array< AnimationAction > - used as prototypes
  GHashTable *action_by_root; // AnimationAction - lookup
} ByClipInfo;

static ByClipInfo *
by_clip_info_new (void)
{
  ByClipInfo *info = g_new0 (ByClipInfo, 1);
  info->known_actions = g_ptr_array_new_with_free_func (g_object_unref);
  info->action_by_root = NULL; // TODO
  return info;
}

static void
by_clip_info_free (ByClipInfo *info)
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
  GHashTable *actions_by_clip; // ByClipInfo

} GthreeAnimationMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeAnimationMixer, gthree_animation_mixer, G_TYPE_OBJECT)

static void
gthree_animation_mixer_init (GthreeAnimationMixer *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  priv->time_scale = 1.0;
  priv->time = 0;
  priv->accu_index = 0;
  priv->actions = g_ptr_array_new_with_free_func (g_object_unref);
  priv->actions_by_clip = NULL; // TODO
}

static void
gthree_animation_mixer_finalize (GObject *obj)
{
  GthreeAnimationMixer *mixer = GTHREE_ANIMATION_MIXER (obj);
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  g_clear_object (&priv->root);

  g_ptr_array_unref (priv->actions);
  if (priv->actions_by_clip)
    g_hash_table_unref (priv->actions_by_clip);

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

void
_gthree_animation_mixer_activate_action (GthreeAnimationMixer  *mixer,
                                         GthreeAnimationAction *action)
{
  g_warning ("TODO");
}

void
_gthree_animation_mixer_deactivate_action (GthreeAnimationMixer  *mixer,
                                           GthreeAnimationAction *action)
{
  g_warning ("TODO");
}

gboolean
_gthree_animation_mixer_is_active_action (GthreeAnimationMixer  *mixer,
                                          GthreeAnimationAction *action)
{
  g_warning ("TODO");
  return FALSE;
}

void
_gthree_animation_mixer_take_back_control_interpolant (GthreeAnimationMixer  *mixer,
                                                       GthreeInterpolant     *interpolant)
{
  g_warning ("TODO");
}

GthreeInterpolant *
_gthree_animation_mixer_lend_control_interpolant (GthreeAnimationMixer  *mixer)
{
  g_warning ("TODO");
  return NULL;
}

float
gthree_animation_mixer_get_time (GthreeAnimationMixer  *mixer)
{
  GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  return priv->time;
}

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
