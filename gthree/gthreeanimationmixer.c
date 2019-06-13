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
