#include <math.h>
#include <epoxy/gl.h>

#include "gthreeorthographiccamera.h"
#include "gthreedirectionallight.h"
#include "gthreedirectionallightshadow.h"
#include "gthreeprivate.h"

typedef struct {
  int dummy;
} GthreeDirectionalLightShadowPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeDirectionalLightShadow, gthree_directional_light_shadow, GTHREE_TYPE_LIGHT_SHADOW)

GthreeDirectionalLightShadow *
gthree_directional_light_shadow_new (void)
{
  return g_object_new (gthree_directional_light_shadow_get_type (), NULL);
}

static void
gthree_directional_light_shadow_init (GthreeDirectionalLightShadow *directional)
{
  g_autoptr(GthreeOrthographicCamera) camera = NULL;

  camera = gthree_orthographic_camera_new (-5, 5, 5, -5, 0.5, 500);
  gthree_light_shadow_set_camera (GTHREE_LIGHT_SHADOW (directional), GTHREE_CAMERA (camera));
}

static void
gthree_directional_light_shadow_finalize (GObject *obj)
{

  G_OBJECT_CLASS (gthree_directional_light_shadow_parent_class)->finalize (obj);
}

static void
gthree_directional_light_shadow_class_init (GthreeDirectionalLightShadowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_directional_light_shadow_finalize;
}
