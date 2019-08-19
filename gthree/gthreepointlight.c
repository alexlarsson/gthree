#include <math.h>
#include <epoxy/gl.h>

#include "gthreepointlight.h"
#include "gthreeperspectivecamera.h"
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
gthree_point_light_new (const graphene_vec3_t *color,
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
static float white[3] = { 1, 1, 1 };
static int i0 = 0;
static float f0 = 0.0;
static float f1 = 1.0;
static float f1000 = 1000.0;
static float zerov2[2] = { 0, 0 };

static GthreeUniformsDefinition light_uniforms[] = {
  {"position", GTHREE_UNIFORM_TYPE_VECTOR3, &zerov3},
  {"color", GTHREE_UNIFORM_TYPE_VECTOR3, &white },
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
  g_autoptr(GthreePerspectiveCamera) camera = NULL;
  g_autoptr(GthreeLightShadow) shadow = NULL;

  priv->distance = 0;
  priv->decay = 1;
  priv->uniforms = gthree_uniforms_new_from_definitions (light_uniforms, G_N_ELEMENTS (light_uniforms));

  camera = gthree_perspective_camera_new (50, 1, 0.5, 500);
  shadow = gthree_light_shadow_new (GTHREE_CAMERA (camera));
  gthree_light_set_shadow (GTHREE_LIGHT (point), GTHREE_LIGHT_SHADOW (shadow));
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
  graphene_vec3_t color;
  float intensity = gthree_light_get_intensity (light);
  graphene_vec4_t light_pos;
  graphene_vec3_t light_pos3;
  const graphene_matrix_t *view_matrix = gthree_camera_get_world_inverse_matrix (camera);
  GthreeTexture *shadow_map_texture = NULL;
  graphene_matrix_t shadow_matrix;

  graphene_vec3_scale (gthree_light_get_color (light), intensity, &color);
  gthree_uniforms_set_vec3 (priv->uniforms, "color", &color);

  graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)), 3, &light_pos);
  graphene_matrix_transform_vec4 (view_matrix, &light_pos, &light_pos);
  graphene_vec4_get_xyz (&light_pos, &light_pos3);

  gthree_uniforms_set_vec3 (priv->uniforms, "position", &light_pos3);
  gthree_uniforms_set_float (priv->uniforms, "distance", priv->distance);
  gthree_uniforms_set_float (priv->uniforms, "decay", priv->decay);

  gthree_uniforms_set_int (priv->uniforms, "shadow", gthree_object_get_cast_shadow (GTHREE_OBJECT (light)) != FALSE);

  if (gthree_object_get_cast_shadow (GTHREE_OBJECT (light)))
    {
      GthreeLightShadow *shadow = gthree_light_get_shadow (light);
      GthreeCamera *shadow_camera = gthree_light_shadow_get_camera (shadow);
      graphene_vec2_t size;

      gthree_uniforms_set_float (priv->uniforms, "shadowBias", gthree_light_shadow_get_bias (shadow));
      gthree_uniforms_set_float (priv->uniforms, "shadowRadius", gthree_light_shadow_get_radius (shadow));

      graphene_vec2_init (&size,
                          gthree_light_shadow_get_map_width (shadow),
                          gthree_light_shadow_get_map_height (shadow));
      gthree_uniforms_set_vec2 (priv->uniforms, "shadowMapSize", &size);

      gthree_uniforms_set_float (priv->uniforms, "shadowCameraNear", gthree_camera_get_near (shadow_camera));
      gthree_uniforms_set_float (priv->uniforms, "shadowCameraFar", gthree_camera_get_far (shadow_camera));

      GthreeRenderTarget *shadow_map = gthree_light_shadow_get_map (shadow);
      if (shadow_map)
        shadow_map_texture = gthree_render_target_get_texture (shadow_map);

      shadow_matrix = *gthree_light_shadow_get_matrix (shadow);
    }
  else
    graphene_matrix_init_identity (&shadow_matrix);

  g_ptr_array_add (setup->point, priv->uniforms);
  g_ptr_array_add (setup->point_shadow_map, shadow_map_texture);
  g_array_append_val (setup->point_shadow_map_matrix, shadow_matrix);

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
                        0.0f, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_DECAY] =
    g_param_spec_float ("decay", "Decay", "Decay",
                        0.5f, 3.0f, 1.0f,
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
