#include <math.h>

#include "gthreeanimationmixer.h"


typedef struct {
} GthreeAnimationMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeAnimationMixer, gthree_animation_mixer, G_TYPE_OBJECT)

static void
gthree_animation_mixer_init (GthreeAnimationMixer *mixer)
{
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);
}

static void
gthree_animation_mixer_finalize (GObject *obj)
{
  //GthreeAnimationMixer *mixer = GTHREE_ANIMATION_MIXER (obj);
  //GthreeAnimationMixerPrivate *priv = gthree_animation_mixer_get_instance_private (mixer);

  G_OBJECT_CLASS (gthree_animation_mixer_parent_class)->finalize (obj);
}

static void
gthree_animation_mixer_class_init (GthreeAnimationMixerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_animation_mixer_finalize;
}

void
gthree_animation_mixer_activate_action (GthreeAnimationMixer  *mixer,
                                        GthreeAnimationAction *action)
{
  g_warning ("TODO");
}

void
gthree_animation_mixer_deactivate_action (GthreeAnimationMixer  *mixer,
                                          GthreeAnimationAction *action)
{
  g_warning ("TODO");
}

gboolean
gthree_animation_mixer_is_active_action (GthreeAnimationMixer  *mixer,
                                         GthreeAnimationAction *action)
{
  g_warning ("TODO");
  return FALSE;
}

void
gthree_action_mixer_take_back_control_interpolant (GthreeAnimationMixer  *mixer,
                                                   GthreeInterpolant     *interpolant)
{
  g_warning ("TODO");
}

GthreeInterpolant *
gthree_action_mixer_lend_control_interpolant (GthreeAnimationMixer  *mixer)
{
  g_warning ("TODO");
  return NULL;
}

float
gthree_action_mixer_get_time (GthreeAnimationMixer  *mixer)
{
  g_warning ("TODO");
  return 0;
}

GthreeObject *
gthree_action_mixer_get_root (GthreeAnimationMixer  *mixer)
{
  g_warning ("TODO");
  return NULL;
}

void
gthree_action_mixer_displatch_event (GthreeAnimationMixer  *mixer,
                                     const char *type /*, ... */)
{
  g_warning ("TODO");
}
