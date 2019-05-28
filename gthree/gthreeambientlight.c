#include <math.h>
#include <epoxy/gl.h>

#include "gthreeambientlight.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeAmbientLight, gthree_ambient_light, GTHREE_TYPE_LIGHT)

GthreeAmbientLight *
gthree_ambient_light_new (const GdkRGBA *color)
{
  return g_object_new (GTHREE_TYPE_AMBIENT_LIGHT,
                       "color", color,
                       NULL);
}

static void
gthree_ambient_light_init (GthreeAmbientLight *ambient)
{
}

static void
gthree_ambient_light_real_setup (GthreeLight *light,
                                 GthreeCamera  *camera,
                                 GthreeLightSetup *setup)
{
  const GdkRGBA *color = gthree_light_get_color (light);
  float intensity = gthree_light_get_intensity (light);

  setup->ambient.red += color->red * intensity;
  setup->ambient.green += color->green * intensity;
  setup->ambient.blue += color->blue * intensity;

  GTHREE_LIGHT_CLASS (gthree_ambient_light_parent_class)->setup (light, camera, setup);
}

static void
gthree_ambient_light_class_init (GthreeAmbientLightClass *klass)
{
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_ambient_light_real_setup;
}
