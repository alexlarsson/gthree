#include <math.h>

#include "gthreepropertymixerprivate.h"


typedef struct {
} GthreePropertyMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreePropertyMixer, gthree_property_mixer, G_TYPE_OBJECT)

static void
gthree_property_mixer_init (GthreePropertyMixer *mixer)
{
  //GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);
}

static void
gthree_property_mixer_finalize (GObject *obj)
{
  //GthreePropertyMixer *mixer = GTHREE_PROPERTY_MIXER (obj);
  //GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);

  G_OBJECT_CLASS (gthree_property_mixer_parent_class)->finalize (obj);
}

static void
gthree_property_mixer_class_init (GthreePropertyMixerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_property_mixer_finalize;
}

void
gthree_property_mixer_accumulate (GthreePropertyMixer *mixer, int accu_index, float weight)
{
  g_warning ("TODO");
}
