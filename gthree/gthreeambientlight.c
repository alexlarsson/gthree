#include <math.h>
#include <epoxy/gl.h>

#include "gthreeambientlight.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeAmbientLight, gthree_ambient_light, GTHREE_TYPE_LIGHT)

GthreeAmbientLight *
gthree_ambient_light_new (const GdkRGBA *color)
{
  return g_object_new (GTHREE_TYPE_LIGHT,
                       "color", color,
                       NULL);
}

static void
gthree_ambient_light_init (GthreeAmbientLight *ambient)
{
}

static void
gthree_ambient_light_real_set_params (GthreeLight *light,
				      GthreeProgramParameters *params)
{
  GTHREE_LIGHT_CLASS (gthree_ambient_light_parent_class)->set_params (light, params);
}

static void
gthree_ambient_light_real_setup (GthreeLight *light,
				 GthreeLightSetup *setup)
{
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
  GTHREE_LIGHT_CLASS(klass)->set_params = gthree_ambient_light_real_set_params;
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_ambient_light_real_setup;
}
