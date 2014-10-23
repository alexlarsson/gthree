#include <math.h>
#include <epoxy/gl.h>

#include "gthreelight.h"

typedef struct {
  GdkRGBA color;
  gboolean visible;
  gboolean only_shadow;
  gboolean casts_shadow;
} GthreeLightPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeLight, gthree_light, GTHREE_TYPE_OBJECT);

GthreeLight *
gthree_light_new ()
{
  GthreeLight *light;

  // TODO: properties
  light = g_object_new (gthree_light_get_type (),
                         NULL);

  return light;
}

static void
gthree_light_init (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;
  priv->visible = TRUE;
  priv->only_shadow = FALSE;
  priv->casts_shadow = FALSE;
}

static void
gthree_light_finalize (GObject *obj)
{
  //GthreeLight *light = GTHREE_LIGHT (obj);
  //GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  G_OBJECT_CLASS (gthree_light_parent_class)->finalize (obj);
}

static void
gthree_light_real_set_params (GthreeLight *light,
			      GthreeProgramParameters *params)
{
}

static void
gthree_light_real_setup (GthreeLight *light,
			 GthreeLightSetup *setup)
{
}

static void
gthree_light_class_init (GthreeLightClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_light_finalize;
  GTHREE_LIGHT_CLASS(klass)->set_params = gthree_light_real_set_params;
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_light_real_setup;
}

gboolean
gthree_light_get_is_only_shadow (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->only_shadow;
}

gboolean
gthree_light_get_is_visible (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->only_shadow;
}

void
gthree_light_set_is_visible (GthreeLight *light,
			     gboolean visible)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  priv->visible = visible;
}

gboolean
gthree_light_get_casts_shadow (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->casts_shadow;
}

void
gthree_light_set_params (GthreeLight       *light,
			 GthreeProgramParameters *params)
{
  GthreeLightClass *class = GTHREE_LIGHT_GET_CLASS(light);

  class->set_params (light, params);
}

const GdkRGBA *
gthree_light_get_color (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return &priv->color;
}

void
gthree_light_set_color (GthreeLight *light,
			const GdkRGBA *color)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  priv->color = *color;
}

void
gthree_light_setup (GthreeLight *light,
		    GthreeLightSetup *setup)
{
  GthreeLightClass *class = GTHREE_LIGHT_GET_CLASS(light);

  class->setup (light, setup);
}
  
