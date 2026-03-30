#include <math.h>
#include <epoxy/gl.h>

#include "gthreeambientlight.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeAmbientLight, gthree_ambient_light, GTHREE_TYPE_LIGHT)

GthreeAmbientLight *
gthree_ambient_light_new (const graphene_vec3_t *color)
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
  graphene_vec3_t color;
  float intensity = gthree_light_get_intensity (light);

  graphene_vec3_scale (gthree_light_get_color (light),
                       intensity, &color);

  graphene_vec3_add (&setup->ambient, &color, &setup->ambient);

  GTHREE_LIGHT_CLASS (gthree_ambient_light_parent_class)->setup (light, camera, setup);
}

static void
gthree_ambient_light_class_init (GthreeAmbientLightClass *klass)
{
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_ambient_light_real_setup;
}
