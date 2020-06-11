#include <math.h>
#include <epoxy/gl.h>
#include <graphene-gobject.h>

#include "gthreehemispherelight.h"
#include "gthreecamera.h"
#include "gthreeprivate.h"


typedef struct {
  graphene_vec3_t ground_color;
  GthreeUniforms *uniforms;
} GthreeHemisphereLightPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeHemisphereLight, gthree_hemisphere_light, GTHREE_TYPE_LIGHT)

enum {
  PROP_0,

  PROP_GROUND_COLOR,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

GthreeHemisphereLight *
gthree_hemisphere_light_new (const graphene_vec3_t *sky_color,
                             const graphene_vec3_t *ground_color,
                             float                  intensity)
{
  return g_object_new (GTHREE_TYPE_HEMISPHERE_LIGHT,
                       "color", sky_color,
                       "intensity", intensity,
                       "ground-color", ground_color,
                       NULL);
}

static float zerov3[3] = { 0, 0, 0 };
static float white[3] = { 1, 1, 1 };

static GthreeUniformsDefinition light_uniforms[] = {
  {"direction", GTHREE_UNIFORM_TYPE_VECTOR3, &zerov3},
  {"skyColor", GTHREE_UNIFORM_TYPE_VECTOR3, &white },
  {"groundColor", GTHREE_UNIFORM_TYPE_VECTOR3, &white },
};

static void
gthree_hemisphere_light_init (GthreeHemisphereLight *hemisphere)
{
  GthreeHemisphereLightPrivate *priv = gthree_hemisphere_light_get_instance_private (hemisphere);

  priv->uniforms = gthree_uniforms_new_from_definitions (light_uniforms, G_N_ELEMENTS (light_uniforms));
}

static void
gthree_hemisphere_light_finalize (GObject *obj)
{
  GthreeHemisphereLight *hemisphere = GTHREE_HEMISPHERE_LIGHT (obj);
  GthreeHemisphereLightPrivate *priv = gthree_hemisphere_light_get_instance_private (hemisphere);

  g_clear_object (&priv->uniforms);

  G_OBJECT_CLASS (gthree_hemisphere_light_parent_class)->finalize (obj);
}

static void
gthree_hemisphere_light_real_setup (GthreeLight *light,
                                    GthreeCamera  *camera,
                                    GthreeLightSetup *setup)
{
  GthreeHemisphereLight *hemisphere = GTHREE_HEMISPHERE_LIGHT (light);
  GthreeHemisphereLightPrivate *priv = gthree_hemisphere_light_get_instance_private (hemisphere);
  graphene_vec3_t color;
  float intensity = gthree_light_get_intensity (light);
  const graphene_matrix_t *view_matrix = gthree_camera_get_world_inverse_matrix (camera);
  graphene_vec4_t light_pos;
  graphene_vec3_t direction;

  graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)), 3, &light_pos);
  graphene_vec4_get_xyz (&light_pos, &direction);
  graphene_matrix_transform_vec3 (view_matrix, &direction, &direction);
  graphene_vec3_normalize (&direction, &direction);

  gthree_uniforms_set_vec3 (priv->uniforms, "direction", &direction);

  graphene_vec3_scale (gthree_light_get_color (light),
                       intensity, &color);
  gthree_uniforms_set_vec3 (priv->uniforms, "skyColor", &color);
  graphene_vec3_scale (&priv->ground_color,
                       intensity, &color);
  gthree_uniforms_set_vec3 (priv->uniforms, "groundColor", &color);

  g_ptr_array_add (setup->hemi, priv->uniforms);

  GTHREE_LIGHT_CLASS (gthree_hemisphere_light_parent_class)->setup (light, camera, setup);
}


const graphene_vec3_t *
gthree_hemisphere_light_get_ground_color (GthreeHemisphereLight *light)
{
  GthreeHemisphereLightPrivate *priv = gthree_hemisphere_light_get_instance_private (light);

  return &priv->ground_color;
}

void
gthree_hemisphere_light_set_ground_color (GthreeHemisphereLight *light,
                                          const graphene_vec3_t *color)
{
  GthreeHemisphereLightPrivate *priv = gthree_hemisphere_light_get_instance_private (light);

  if (graphene_vec3_equal (color, &priv->ground_color))
    return;

  priv->ground_color = *color;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_GROUND_COLOR]);
}


static void
gthree_hemisphere_light_set_property (GObject *obj,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
  GthreeHemisphereLight *hemisphere = GTHREE_HEMISPHERE_LIGHT (obj);

  switch (prop_id)
    {
    case PROP_GROUND_COLOR:
      gthree_hemisphere_light_set_ground_color (hemisphere, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_hemisphere_light_get_property (GObject *obj,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
  GthreeHemisphereLight *hemisphere = GTHREE_HEMISPHERE_LIGHT (obj);
  GthreeHemisphereLightPrivate *priv = gthree_hemisphere_light_get_instance_private (hemisphere);

  switch (prop_id)
    {
    case PROP_GROUND_COLOR:
      g_value_set_boxed (value, &priv->ground_color);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}


static void
gthree_hemisphere_light_class_init (GthreeHemisphereLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_hemisphere_light_set_property;
  gobject_class->get_property = gthree_hemisphere_light_get_property;
  gobject_class->finalize = gthree_hemisphere_light_finalize;
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_hemisphere_light_real_setup;

  obj_props[PROP_GROUND_COLOR] =
    g_param_spec_boxed ("ground-color", "Ground Color", "Light ground color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
