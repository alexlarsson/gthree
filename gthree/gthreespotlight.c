#include <math.h>
#include <epoxy/gl.h>

#include "gthreespotlight.h"
#include "gthreespotlightshadow.h"
#include "gthreecamera.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeObject *target;
  float distance;
  float angle;
  float penumbra;
  float decay;
  GthreeUniforms *uniforms;
} GthreeSpotLightPrivate;

enum {
  PROP_0,

  PROP_TARGET,
  PROP_DECAY,
  PROP_DISTANCE,
  PROP_ANGLE,
  PROP_PENUMBRA,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSpotLight, gthree_spot_light, GTHREE_TYPE_LIGHT)

GthreeSpotLight *
gthree_spot_light_new (const graphene_vec3_t *color,
                       float intensity,
                       float distance,
                       float angle,
                       float penumbra)
{
  return g_object_new (gthree_spot_light_get_type (),
                       "color", color,
                       "intensity", intensity,
                       "distance", distance,
                       "angle", angle,
                       "penumbra", penumbra,
                       NULL);
}

static float zerov3[3] = { 0, 0, 0 };
static float white[3] = { 1, 1, 1 };
static int i0 = 0;
static float f0 = 0.0;
static float f1 = 1.0;
static float zerov2[2] = { 0, 0 };

static GthreeUniformsDefinition light_uniforms[] = {
  {"position", GTHREE_UNIFORM_TYPE_VECTOR3, &zerov3},
  {"direction", GTHREE_UNIFORM_TYPE_VECTOR3, &zerov3},
  {"color", GTHREE_UNIFORM_TYPE_VECTOR3, &white },
  {"distance", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"decay", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"coneCos", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"penumbraCos", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },

  {"shadow", GTHREE_UNIFORM_TYPE_INT, &i0 },
  {"shadowBias", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"shadowRadius", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"shadowMapSize", GTHREE_UNIFORM_TYPE_VECTOR2, &zerov2 },
};

static void
gthree_spot_light_init (GthreeSpotLight *spot)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (spot);
  g_autoptr(GthreeSpotLightShadow) shadow = NULL;

  priv->target = gthree_object_new ();
  priv->distance = 0;
  priv->decay = 1;
  priv->angle = G_PI / 3;
  priv->penumbra = 0;

  priv->uniforms = gthree_uniforms_new_from_definitions (light_uniforms, G_N_ELEMENTS (light_uniforms));

  shadow = gthree_spot_light_shadow_new ();
  gthree_light_set_shadow (GTHREE_LIGHT (spot), GTHREE_LIGHT_SHADOW (shadow));
}

static void
gthree_spot_light_finalize (GObject *obj)
{
  GthreeSpotLight *spot = GTHREE_SPOT_LIGHT (obj);
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (spot);

  g_clear_object (&priv->target);
  g_clear_object (&priv->uniforms);

  G_OBJECT_CLASS (gthree_spot_light_parent_class)->finalize (obj);
}

static void
gthree_spot_light_real_setup (GthreeLight *light,
                               GthreeCamera *camera,
                               GthreeLightSetup *setup)
{
  GthreeSpotLight *spot = GTHREE_SPOT_LIGHT (light);
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (spot);
  graphene_vec3_t color;
  float intensity = gthree_light_get_intensity (light);
  graphene_vec4_t light_pos;
  graphene_vec4_t light_pos_view;
  graphene_vec3_t light_pos_view3;
  graphene_vec4_t target_pos;
  graphene_vec4_t direction;
  graphene_vec3_t direction3;
  const graphene_matrix_t *view_matrix = gthree_camera_get_world_inverse_matrix (camera);
  GthreeTexture *shadow_map_texture = NULL;
  graphene_matrix_t shadow_matrix;

  graphene_vec3_scale (gthree_light_get_color (light), intensity, &color);
  gthree_uniforms_set_vec3 (priv->uniforms, "color", &color);

  graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)), 3, &light_pos);
  graphene_matrix_transform_vec4 (view_matrix, &light_pos, &light_pos_view);
  graphene_vec4_get_xyz (&light_pos_view, &light_pos_view3);
  gthree_uniforms_set_vec3 (priv->uniforms, "position", &light_pos_view3);

  graphene_matrix_get_row (gthree_object_get_world_matrix (priv->target), 3, &target_pos);

  graphene_vec4_subtract (&light_pos, &target_pos, &direction);
  graphene_vec4_get_xyz (&direction, &direction3);
  graphene_matrix_transform_vec3 (view_matrix, &direction3, &direction3);
  graphene_vec3_normalize (&direction3, &direction3);
  gthree_uniforms_set_vec3 (priv->uniforms, "direction", &direction3);

  gthree_uniforms_set_float (priv->uniforms, "distance", priv->distance);
  gthree_uniforms_set_float (priv->uniforms, "decay", priv->decay);
  gthree_uniforms_set_float (priv->uniforms, "coneCos", cosf (priv->angle));
  gthree_uniforms_set_float (priv->uniforms, "penumbraCos", cosf (priv->angle * (1 - priv->penumbra)));

  gthree_uniforms_set_int (priv->uniforms, "shadow", gthree_object_get_cast_shadow (GTHREE_OBJECT (light)) != FALSE);

  if (gthree_object_get_cast_shadow (GTHREE_OBJECT (light)))
    {
      GthreeLightShadow *shadow = gthree_light_get_shadow (light);
      graphene_vec2_t size;

      gthree_uniforms_set_float (priv->uniforms, "shadowBias", gthree_light_shadow_get_bias (shadow));
      gthree_uniforms_set_float (priv->uniforms, "shadowRadius", gthree_light_shadow_get_radius (shadow));

      graphene_vec2_init (&size,
                          gthree_light_shadow_get_map_width (shadow),
                          gthree_light_shadow_get_map_height (shadow));
      gthree_uniforms_set_vec2 (priv->uniforms, "shadowMapSize", &size);

      GthreeRenderTarget *shadow_map = gthree_light_shadow_get_map (shadow);
      if (shadow_map)
        shadow_map_texture = gthree_render_target_get_texture (shadow_map);

      shadow_matrix = *gthree_light_shadow_get_matrix (shadow);
    }
  else
    graphene_matrix_init_identity (&shadow_matrix);

  g_ptr_array_add (setup->spot, priv->uniforms);
  g_ptr_array_add (setup->spot_shadow_map, shadow_map_texture);
  g_array_append_val (setup->spot_shadow_map_matrix, shadow_matrix);

  GTHREE_LIGHT_CLASS (gthree_spot_light_parent_class)->setup (light, camera, setup);
}

static void
gthree_spot_light_set_property (GObject *obj,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  GthreeSpotLight *spot = GTHREE_SPOT_LIGHT (obj);

  switch (prop_id)
    {
    case PROP_TARGET:
      gthree_spot_light_set_target (spot, g_value_get_object (value));
      break;

    case PROP_DISTANCE:
      gthree_spot_light_set_distance (spot, g_value_get_float (value));
      break;

    case PROP_DECAY:
      gthree_spot_light_set_decay (spot, g_value_get_float (value));
      break;

    case PROP_ANGLE:
      gthree_spot_light_set_angle (spot, g_value_get_float (value));
      break;

    case PROP_PENUMBRA:
      gthree_spot_light_set_penumbra (spot, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_spot_light_get_property (GObject *obj,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  GthreeSpotLight *spot = GTHREE_SPOT_LIGHT (obj);
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (spot);

  switch (prop_id)
    {
    case PROP_TARGET:
      g_value_set_object (value, priv->target);
      break;

    case PROP_DISTANCE:
      g_value_set_float (value, priv->distance);
      break;

    case PROP_DECAY:
      g_value_set_float (value, priv->decay);
      break;

    case PROP_ANGLE:
      g_value_set_float (value, priv->angle);
      break;

    case PROP_PENUMBRA:
      g_value_set_float (value, priv->penumbra);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_spot_light_class_init (GthreeSpotLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeLightClass *light_class = GTHREE_LIGHT_CLASS (klass);

  gobject_class->set_property = gthree_spot_light_set_property;
  gobject_class->get_property = gthree_spot_light_get_property;
  gobject_class->finalize = gthree_spot_light_finalize;

  light_class->setup = gthree_spot_light_real_setup;

  obj_props[PROP_TARGET] =
    g_param_spec_object ("target", "Target", "Target",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_DISTANCE] =
    g_param_spec_float ("distance", "Distance", "Distance",
                        0.0f, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_DECAY] =
    g_param_spec_float ("decay", "Decay", "Decay",
                        0.5f, 3.0f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_ANGLE] =
    g_param_spec_float ("angle", "Angle", "Angle",
                        0.0f, G_PI / 2, G_PI / 3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_PENUMBRA] =
    g_param_spec_float ("penumbra", "Penumbra", "Penumbra",
                        0.0f, 1.0f, 0.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

void
gthree_spot_light_set_distance (GthreeSpotLight *light,
                                 float distance)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  priv->distance = distance;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_DISTANCE]);
}

float
gthree_spot_light_get_distance (GthreeSpotLight *light)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  return priv->distance;
}

void
gthree_spot_light_set_decay (GthreeSpotLight *light,
                                 float decay)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  priv->decay = decay;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_DECAY]);
}

float
gthree_spot_light_get_decay (GthreeSpotLight *light)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  return priv->decay;
}

void
gthree_spot_light_set_target (GthreeSpotLight *spot,
                              GthreeObject *object)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (spot);

  if (object == NULL)
    object = gthree_object_new ();

  if (g_set_object (&priv->target, object))
    g_object_notify_by_pspec (G_OBJECT (spot), obj_props[PROP_TARGET]);
}

GthreeObject *
gthree_spot_light_get_target (GthreeSpotLight *spot)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (spot);

  return priv->target;
}

void
gthree_spot_light_set_angle (GthreeSpotLight *light,
                                 float angle)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  priv->angle = angle;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_ANGLE]);
}

float
gthree_spot_light_get_angle (GthreeSpotLight *light)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  return priv->angle;
}

void
gthree_spot_light_set_penumbra (GthreeSpotLight *light,
                                 float penumbra)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  priv->penumbra = penumbra;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_PENUMBRA]);
}

float
gthree_spot_light_get_penumbra (GthreeSpotLight *light)
{
  GthreeSpotLightPrivate *priv = gthree_spot_light_get_instance_private (light);

  return priv->penumbra;
}
