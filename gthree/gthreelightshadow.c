#include <math.h>
#include <epoxy/gl.h>

#include "gthreelightshadow.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeCamera *camera;
  float bias;
  float radius;

  int map_width;
  int map_height;

  GthreeRenderTarget *map;

  graphene_matrix_t matrix;
} GthreeLightShadowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLightShadow, gthree_light_shadow, G_TYPE_OBJECT);

static void
gthree_light_shadow_init (GthreeLightShadow *light_shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (light_shadow);

  priv->bias = 0;
  priv->radius = 1;

  priv->map_width = 512;
  priv->map_height = 512;

  graphene_matrix_init_identity (&priv->matrix);
}

static void
gthree_light_shadow_finalize (GObject *obj)
{
  GthreeLightShadow *light_shadow = GTHREE_LIGHT_SHADOW (obj);
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (light_shadow);

  g_clear_object (&priv->camera);
  g_clear_object (&priv->map);

  G_OBJECT_CLASS (gthree_light_shadow_parent_class)->finalize (obj);
}

static void
gthree_light_shadow_class_init (GthreeLightShadowClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_light_shadow_finalize;
}

GthreeLightShadow *
gthree_light_shadow_new (GthreeCamera *camera)
{
  GthreeLightShadow *light_shadow;
  GthreeLightShadowPrivate *priv;

  light_shadow = g_object_new (gthree_light_shadow_get_type (),
                               NULL);

  priv = gthree_light_shadow_get_instance_private (light_shadow);
  priv->camera = g_object_ref (camera);

  return light_shadow;
}

GthreeCamera *
gthree_light_shadow_get_camera (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return priv->camera;
}

void
gthree_light_shadow_set_camera (GthreeLightShadow *shadow,
                                GthreeCamera *camera)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  g_set_object (&priv->camera, camera);
}

int
gthree_light_shadow_get_map_width (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return priv->map_width;
}

int
gthree_light_shadow_get_map_height (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return priv->map_height;
}

GthreeRenderTarget *
gthree_light_shadow_get_map (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return priv->map;
}

void
gthree_light_shadow_set_map (GthreeLightShadow *shadow,
                             GthreeRenderTarget *map)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  g_set_object (&priv->map, map);
}

graphene_matrix_t *
gthree_light_shadow_get_matrix (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return &priv->matrix;
}

float
gthree_light_shadow_get_bias (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return priv->bias;
}

float
gthree_light_shadow_get_radius (GthreeLightShadow *shadow)
{
  GthreeLightShadowPrivate *priv = gthree_light_shadow_get_instance_private (shadow);

  return priv->radius;
}
