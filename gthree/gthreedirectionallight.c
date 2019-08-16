#include <math.h>
#include <epoxy/gl.h>

#include "gthreedirectionallight.h"
#include "gthreecamera.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeObject *target;
  GthreeUniforms *uniforms;
} GthreeDirectionalLightPrivate;

enum {
  PROP_0,

  PROP_TARGET,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeDirectionalLight, gthree_directional_light, GTHREE_TYPE_LIGHT)

GthreeDirectionalLight *
gthree_directional_light_new (const graphene_vec3_t *color,
                              float intensity)
{
  return g_object_new (gthree_directional_light_get_type (),
                       "color", color,
                       "intensity", intensity,
                       NULL);
}

static float zerov3[3] = { 0, 0, 0 };
static float white[3] = { 1, 1, 1 };
static int i0 = 0;
static float f0 = 0.0;
static float f1 = 1.0;
static float zerov2[2] = { 0, 0 };

static GthreeUniformsDefinition light_uniforms[] = {
  {"direction", GTHREE_UNIFORM_TYPE_VECTOR3, &zerov3},
  {"color", GTHREE_UNIFORM_TYPE_VECTOR3, &white },
  {"shadow", GTHREE_UNIFORM_TYPE_INT, &i0 },
  {"shadowBias", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"shadowRadius", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"shadowMapSize", GTHREE_UNIFORM_TYPE_VECTOR2, &zerov2 },
};

static void
gthree_directional_light_init (GthreeDirectionalLight *directional)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);
  graphene_point3d_t pos = {0, 1, 0};

  priv->target = gthree_object_new ();
  priv->uniforms = gthree_uniforms_new_from_definitions (light_uniforms, G_N_ELEMENTS (light_uniforms));

  gthree_object_set_position_point3d (GTHREE_OBJECT (directional), &pos);
}

void
gthree_directional_light_set_target (GthreeDirectionalLight *directional,
                                     GthreeObject *object)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  if (object == NULL)
    object = gthree_object_new ();

  if (g_set_object (&priv->target, object))
    g_object_notify_by_pspec (G_OBJECT (directional), obj_props[PROP_TARGET]);
}

GthreeObject *
gthree_directional_light_get_target (GthreeDirectionalLight *directional)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  return priv->target;
}

static void
gthree_directional_light_finalize (GObject *obj)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (obj);
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  g_clear_object (&priv->target);
  g_clear_object (&priv->uniforms);

  G_OBJECT_CLASS (gthree_directional_light_parent_class)->finalize (obj);
}

static void
gthree_directional_light_real_setup (GthreeLight *light,
                                     GthreeCamera  *camera,
                                     GthreeLightSetup *setup)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (light);
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);
  graphene_vec3_t color;
  float intensity = gthree_light_get_intensity (light);
  graphene_vec4_t light_pos, target_pos;
  graphene_vec3_t direction;
  const graphene_matrix_t *view_matrix = gthree_camera_get_world_inverse_matrix (camera);

  graphene_vec3_scale (gthree_light_get_color (light), intensity, &color);
  gthree_uniforms_set_vec3 (priv->uniforms, "color", &color);

  graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)), 3, &light_pos);
  graphene_matrix_get_row (gthree_object_get_world_matrix (priv->target), 3, &target_pos);

  graphene_vec4_subtract (&light_pos, &target_pos, &light_pos);
  graphene_vec4_get_xyz (&light_pos, &direction);
  graphene_vec3_normalize (&direction, &direction);
  graphene_matrix_transform_vec3 (view_matrix, &direction, &direction); //transformDirection()
  gthree_uniforms_set_vec3 (priv->uniforms, "direction", &direction);

  gthree_uniforms_set_int (priv->uniforms, "shadow", 0);

#ifdef TODO
  if ( light.castShadow ) {
    var shadow = light.shadow;

    uniforms.shadowBias = shadow.bias;
    uniforms.shadowRadius = shadow.radius;
    uniforms.shadowMapSize = shadow.mapSize;
  }
  state.directionalShadowMap[ directionalLength ] = shadowMap;
  state.directionalShadowMatrix[ directionalLength ] = light.shadow.matrix;
#endif

  g_ptr_array_add (setup->directional, priv->uniforms);

  GTHREE_LIGHT_CLASS (gthree_directional_light_parent_class)->setup (light, camera, setup);
}

static void
gthree_directional_light_set_property (GObject *obj,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (obj);

  switch (prop_id)
    {
    case PROP_TARGET:
      gthree_directional_light_set_target (directional, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_directional_light_get_property (GObject *obj,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (obj);
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  switch (prop_id)
    {
    case PROP_TARGET:
      g_value_set_object (value, priv->target);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_directional_light_class_init (GthreeDirectionalLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeLightClass *light_class = GTHREE_LIGHT_CLASS (klass);

  gobject_class->set_property = gthree_directional_light_set_property;
  gobject_class->get_property = gthree_directional_light_get_property;
  gobject_class->finalize = gthree_directional_light_finalize;

  light_class->setup = gthree_directional_light_real_setup;

  obj_props[PROP_TARGET] =
    g_param_spec_object ("target", "Target", "Target",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
