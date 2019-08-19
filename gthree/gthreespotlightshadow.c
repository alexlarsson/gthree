#include <math.h>
#include <epoxy/gl.h>

#include "gthreeperspectivecamera.h"
#include "gthreespotlight.h"
#include "gthreespotlightshadow.h"
#include "gthreeprivate.h"

typedef struct {
  int dummy;
} GthreeSpotLightShadowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSpotLightShadow, gthree_spot_light_shadow, GTHREE_TYPE_LIGHT_SHADOW)

GthreeSpotLightShadow *
gthree_spot_light_shadow_new (void)
{
  return g_object_new (gthree_spot_light_shadow_get_type (), NULL);
}

static void
gthree_spot_light_shadow_init (GthreeSpotLightShadow *spot)
{
  g_autoptr(GthreePerspectiveCamera) camera = NULL;

  camera = gthree_perspective_camera_new (50, 1, 0.5, 500);
  gthree_light_shadow_set_camera (GTHREE_LIGHT_SHADOW (spot), GTHREE_CAMERA (camera));
}

static void
gthree_spot_light_shadow_finalize (GObject *obj)
{

  G_OBJECT_CLASS (gthree_spot_light_shadow_parent_class)->finalize (obj);
}

static void
gthree_spot_light_shadow_class_init (GthreeSpotLightShadowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_spot_light_shadow_finalize;
}

void
gthree_spot_light_shadow_update (GthreeSpotLightShadow *shadow,
                                 GthreeSpotLight *light)
{
  GthreeCamera *camera = gthree_light_shadow_get_camera (GTHREE_LIGHT_SHADOW (shadow));
  float angle = gthree_spot_light_get_angle (light);

  float fov = 180.0 / G_PI * 2 * angle;
  float aspect = gthree_light_shadow_get_map_width (GTHREE_LIGHT_SHADOW (shadow)) / (float) gthree_light_shadow_get_map_height (GTHREE_LIGHT_SHADOW (shadow));
  float far = gthree_spot_light_get_distance (light);
  if (far == 0)
    far = gthree_camera_get_far (camera);

  gthree_perspective_camera_set_fov (GTHREE_PERSPECTIVE_CAMERA (camera), fov);
  gthree_perspective_camera_set_aspect (GTHREE_PERSPECTIVE_CAMERA (camera), aspect);
  gthree_camera_set_far (GTHREE_CAMERA (camera), far);
}
