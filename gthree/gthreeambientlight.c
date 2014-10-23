#include <math.h>
#include <epoxy/gl.h>

#include "gthreeambientlight.h"
#include "gthreeprivate.h"

typedef struct {
  int dummy;
} GthreeAmbientLightPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeAmbientLight, gthree_ambient_light, GTHREE_TYPE_LIGHT);

GthreeAmbientLight *
gthree_ambient_light_new (const GdkRGBA *color)
{
  GthreeAmbientLight *light;

  light = g_object_new (gthree_ambient_light_get_type (),
                           NULL);

  gthree_light_set_color (GTHREE_LIGHT (light), color);

  return light;
}

static void
gthree_ambient_light_init (GthreeAmbientLight *ambient)
{
  GthreeAmbientLightPrivate *priv = gthree_ambient_light_get_instance_private (ambient);

}

static void
gthree_ambient_light_finalize (GObject *obj)
{
  GthreeAmbientLight *ambient = GTHREE_AMBIENT_LIGHT (obj);
  GthreeAmbientLightPrivate *priv = gthree_ambient_light_get_instance_private (ambient);

  G_OBJECT_CLASS (gthree_ambient_light_parent_class)->finalize (obj);
}

static void
gthree_ambient_light_real_set_params (GthreeLight *light,
				      GthreeProgramParameters *params)
{
  GthreeAmbientLight *ambient = GTHREE_AMBIENT_LIGHT (light);
  GthreeAmbientLightPrivate *priv = gthree_ambient_light_get_instance_private (ambient);

  GTHREE_LIGHT_CLASS (gthree_ambient_light_parent_class)->set_params (light, params);
}

static void
gthree_ambient_light_real_setup (GthreeLight *light,
				 GthreeLightSetup *setup)
{
  GthreeAmbientLight *ambient = GTHREE_AMBIENT_LIGHT (light);
  GthreeAmbientLightPrivate *priv = gthree_ambient_light_get_instance_private (ambient);
  const GdkRGBA *color = gthree_light_get_color (light);
  
  if (gthree_light_get_is_visible (light))
    {
      if (FALSE /* this.gammaInput */)
	{
	  setup->ambient.red += color->red * color->red;
	  setup->ambient.green += color->green * color->green;
	  setup->ambient.blue += color->blue * color->blue;
	}
      else
	{
	  setup->ambient.red += color->red;
	  setup->ambient.green += color->green;
	  setup->ambient.blue += color->blue;
	}
    }
  
  GTHREE_LIGHT_CLASS (gthree_ambient_light_parent_class)->setup (light, setup);
}

static void
gthree_ambient_light_class_init (GthreeAmbientLightClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_ambient_light_finalize;
  GTHREE_LIGHT_CLASS(klass)->set_params = gthree_ambient_light_real_set_params;
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_ambient_light_real_setup;
}
