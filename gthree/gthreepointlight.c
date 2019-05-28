#include <math.h>
#include <epoxy/gl.h>

#include "gthreepointlight.h"
#include "gthreeprivate.h"

typedef struct {
  float distance;
  float decay;
 GthreeUniforms *uniforms;
} GthreePointLightPrivate;

enum {
  PROP_0,

  PROP_DECAY,
  PROP_DISTANCE,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreePointLight, gthree_point_light, GTHREE_TYPE_LIGHT)

GthreePointLight *
gthree_point_light_new (const GdkRGBA *color,
                        float intensity,
                        float distance)
{
  return g_object_new (gthree_point_light_get_type (),
                       "color", color,
                       "intensity", intensity,
                       "distance", distance,
                       NULL);
}

static float zerov3[3] = { 0, 0, 0 };
static GdkRGBA white = { 1, 1, 1, 1.0 };
static int i0 = 0;
static float f0 = 0.0;
static float f1 = 1.0;
static float f1000 = 1000.0;
static float zerov2[2] = { 0, 0 };

static GthreeUniformsDefinition light_uniforms[] = {
  {"position", GTHREE_UNIFORM_TYPE_VECTOR3, &zerov3},
  {"color", GTHREE_UNIFORM_TYPE_COLOR, &white },
  {"distance", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"decay", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"shadow", GTHREE_UNIFORM_TYPE_INT, &i0 },
  {"shadowBias", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"shadowRadius", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"shadowMapSize", GTHREE_UNIFORM_TYPE_VECTOR2, &zerov2 },
  {"shadowCameraNear", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"shadowCameraFar", GTHREE_UNIFORM_TYPE_FLOAT, &f1000 },
};

static void
gthree_point_light_init (GthreePointLight *point)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);

  priv->distance = 0;
  priv->decay = 2;
  priv->uniforms = gthree_uniforms_new_from_definitions (light_uniforms, G_N_ELEMENTS (light_uniforms));
}

static void
gthree_point_light_finalize (GObject *obj)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (obj);
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);

  g_clear_object (&priv->uniforms);

  G_OBJECT_CLASS (gthree_point_light_parent_class)->finalize (obj);
}

static void
gthree_point_light_real_setup (GthreeLight *light,
                               GthreeCamera *camera,
                               GthreeLightSetup *setup)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (light);
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);
  GdkRGBA color = *gthree_light_get_color (light);
  float intensity = gthree_light_get_intensity (light);
  graphene_vec4_t light_pos, target_pos;
  graphene_vec3_t light_pos3;
  const graphene_matrix_t *view_matrix = gthree_camera_get_world_inverse_matrix (camera);

  color.red *= intensity;
  color.green *= intensity;
  color.blue *= intensity;
  color.alpha = 1.0;

  gthree_uniforms_set_color (priv->uniforms, "color", &color);

  graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)), 3, &light_pos);
  graphene_matrix_transform_vec4 (view_matrix, &light_pos, &light_pos);
  graphene_vec4_get_xyz (&light_pos, &light_pos3);

  gthree_uniforms_set_vec3 (priv->uniforms, "position", &light_pos3);
  gthree_uniforms_set_float (priv->uniforms, "distance", priv->distance);
  gthree_uniforms_set_float (priv->uniforms, "decay", priv->decay);


  gthree_uniforms_set_int (priv->uniforms, "shadow", 0);
#ifdef TODO
  uniforms.shadow = light.castShadow;
  if ( light.castShadow ) {
    var shadow = light.shadow;

    uniforms.shadowBias = shadow.bias;
    uniforms.shadowRadius = shadow.radius;
    uniforms.shadowMapSize = shadow.mapSize;
    uniforms.shadowCameraNear = shadow.camera.near;
    uniforms.shadowCameraFar = shadow.camera.far;

  }

  state.pointShadowMap[ pointLength ] = shadowMap;
  state.pointShadowMatrix[ pointLength ] = light.shadow.matrix;
#endif

  g_ptr_array_add (setup->point, priv->uniforms);

  GTHREE_LIGHT_CLASS (gthree_point_light_parent_class)->setup (light, camera, setup);
}

static void
gthree_point_light_set_property (GObject *obj,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (obj);

  switch (prop_id)
    {
    case PROP_DISTANCE:
      gthree_point_light_set_distance (point, g_value_get_float (value));
      break;

    case PROP_DECAY:
      gthree_point_light_set_decay (point, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_point_light_get_property (GObject *obj,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (obj);
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);

  switch (prop_id)
    {

    case PROP_DISTANCE:
      g_value_set_float (value, priv->distance);
      break;

    case PROP_DECAY:
      g_value_set_float (value, priv->decay);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_point_light_class_init (GthreePointLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeLightClass *light_class = GTHREE_LIGHT_CLASS (klass);

  gobject_class->set_property = gthree_point_light_set_property;
  gobject_class->get_property = gthree_point_light_get_property;
  gobject_class->finalize = gthree_point_light_finalize;

  light_class->setup = gthree_point_light_real_setup;

  obj_props[PROP_DISTANCE] =
    g_param_spec_float ("distance", "Distance", "Distance",
                        -G_MAXFLOAT, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_DECAY] =
    g_param_spec_float ("decay", "Decay", "Decay",
                        -G_MAXFLOAT, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

void
gthree_point_light_set_distance (GthreePointLight *light,
                                 float distance)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  priv->distance = distance;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_DISTANCE]);
}

float
gthree_point_light_get_distance (GthreePointLight *light)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  return priv->distance;
}

void
gthree_point_light_set_decay (GthreePointLight *light,
                                 float decay)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  priv->decay = decay;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_DECAY]);
}

float
gthree_point_light_get_decay (GthreePointLight *light)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  return priv->decay;
}
