#include <graphene-gobject.h>

#include "gthreesky.h"
#include "gthreeshadermaterial.h"
#include "gthreeprimitives.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeShader *shader;
  float turbidity;
  float rayleigh;
  float mie_coefficient;
  float mie_directional_g;
  graphene_vec3_t sun_position;
  graphene_vec3_t up;
} GthreeSkyPrivate;

enum {
  PROP_0,

  PROP_TURBIDITY,
  PROP_RAYLEIGH,
  PROP_MIE_COEFFICIENT,
  PROP_MIE_DIRECTIONAL_G,
  PROP_SUN_POSITION,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSky, gthree_sky, GTHREE_TYPE_MESH)

static void
gthree_sky_init (GthreeSky *sky)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->turbidity = 2.0;
  priv->rayleigh = 1.0;
  priv->mie_coefficient = 0.005;
  priv->mie_directional_g = 0.8;
  graphene_vec3_init (&priv->sun_position, 0, 0, 0);
  graphene_vec3_init (&priv->up, 0, 1, 0);
}

static GObject *
gthree_sky_constructor (GType type,
                        guint n_construct_params,
                        GObjectConstructParam *construct_params)
{
  GObject *obj;
  GthreeSky *sky;
  GthreeSkyPrivate *priv;
  g_autoptr(GthreeGeometry) geometry = NULL;
  g_autoptr(GthreeShaderMaterial) material = NULL;
  g_autoptr(GPtrArray) materials = NULL;

  obj = G_OBJECT_CLASS (gthree_sky_parent_class)->constructor (type, n_construct_params, construct_params);
  sky = GTHREE_SKY (obj);
  priv = gthree_sky_get_instance_private (sky);

  geometry = gthree_geometry_new_box (1, 1, 1, 1, 1, 1);
  priv->shader = gthree_clone_shader_from_library ("sky");

  material = gthree_shader_material_new (priv->shader);
  gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_BACK);
  gthree_material_set_depth_write (GTHREE_MATERIAL (material), FALSE);

  materials = g_ptr_array_new_with_free_func (g_object_unref);
  g_ptr_array_add (materials, g_object_ref (material));

  g_object_set (obj,
                "geometry", geometry,
                "materials", materials,
                NULL);

  gthree_object_set_is_frustum_culled (GTHREE_OBJECT (obj), FALSE);

  return obj;
}

static void
gthree_sky_finalize (GObject *obj)
{
  GthreeSky *sky = GTHREE_SKY (obj);
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  g_clear_object (&priv->shader);

  G_OBJECT_CLASS (gthree_sky_parent_class)->finalize (obj);
}

static void
gthree_sky_set_property (GObject *obj,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
  GthreeSky *sky = GTHREE_SKY (obj);

  switch (prop_id)
    {
    case PROP_TURBIDITY:
      gthree_sky_set_turbidity (sky, g_value_get_float (value));
      break;
    case PROP_RAYLEIGH:
      gthree_sky_set_rayleigh (sky, g_value_get_float (value));
      break;
    case PROP_MIE_COEFFICIENT:
      gthree_sky_set_mie_coefficient (sky, g_value_get_float (value));
      break;
    case PROP_MIE_DIRECTIONAL_G:
      gthree_sky_set_mie_directional_g (sky, g_value_get_float (value));
      break;
    case PROP_SUN_POSITION:
      gthree_sky_set_sun_position (sky, g_value_get_boxed (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_sky_get_property (GObject *obj,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
  GthreeSky *sky = GTHREE_SKY (obj);
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  switch (prop_id)
    {
    case PROP_TURBIDITY:
      g_value_set_float (value, priv->turbidity);
      break;
    case PROP_RAYLEIGH:
      g_value_set_float (value, priv->rayleigh);
      break;
    case PROP_MIE_COEFFICIENT:
      g_value_set_float (value, priv->mie_coefficient);
      break;
    case PROP_MIE_DIRECTIONAL_G:
      g_value_set_float (value, priv->mie_directional_g);
      break;
    case PROP_SUN_POSITION:
      g_value_set_boxed (value, &priv->sun_position);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_sky_set_direct_uniforms (GthreeObject   *object,
                                GthreeProgram  *program,
                                GthreeRenderer *renderer)
{
  GthreeSky *sky = GTHREE_SKY (object);
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);
  GthreeUniforms *uniforms = gthree_shader_get_uniforms (priv->shader);
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, "turbidity");
  if (uni)
    {
      gthree_uniform_set_float (uni, priv->turbidity);
      gthree_uniform_load (uni, renderer);
    }

  uni = gthree_uniforms_lookup_from_string (uniforms, "rayleigh");
  if (uni)
    {
      gthree_uniform_set_float (uni, priv->rayleigh);
      gthree_uniform_load (uni, renderer);
    }

  uni = gthree_uniforms_lookup_from_string (uniforms, "mieCoefficient");
  if (uni)
    {
      gthree_uniform_set_float (uni, priv->mie_coefficient);
      gthree_uniform_load (uni, renderer);
    }

  uni = gthree_uniforms_lookup_from_string (uniforms, "mieDirectionalG");
  if (uni)
    {
      gthree_uniform_set_float (uni, priv->mie_directional_g);
      gthree_uniform_load (uni, renderer);
    }

  uni = gthree_uniforms_lookup_from_string (uniforms, "sunPosition");
  if (uni)
    {
      gthree_uniform_set_vec3 (uni, &priv->sun_position);
      gthree_uniform_load (uni, renderer);
    }

  uni = gthree_uniforms_lookup_from_string (uniforms, "up");
  if (uni)
    {
      gthree_uniform_set_vec3 (uni, &priv->up);
      gthree_uniform_load (uni, renderer);
    }

  GTHREE_OBJECT_CLASS (gthree_sky_parent_class)->set_direct_uniforms (object, program, renderer);
}

static void
gthree_sky_class_init (GthreeSkyClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->constructor = gthree_sky_constructor;
  gobject_class->finalize = gthree_sky_finalize;
  gobject_class->set_property = gthree_sky_set_property;
  gobject_class->get_property = gthree_sky_get_property;
  object_class->set_direct_uniforms = gthree_sky_set_direct_uniforms;

  obj_props[PROP_TURBIDITY] =
    g_param_spec_float ("turbidity", "Turbidity", "Turbidity",
                        0.f, 100.f, 2.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_RAYLEIGH] =
    g_param_spec_float ("rayleigh", "Rayleigh", "Rayleigh",
                        0.f, 100.f, 1.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MIE_COEFFICIENT] =
    g_param_spec_float ("mie-coefficient", "Mie Coefficient", "Mie Coefficient",
                        0.f, 1.f, 0.005f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MIE_DIRECTIONAL_G] =
    g_param_spec_float ("mie-directional-g", "Mie Directional G", "Mie Directional G",
                        0.f, 1.f, 0.8f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SUN_POSITION] =
    g_param_spec_boxed ("sun-position", "Sun Position", "Sun Position",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

GthreeSky *
gthree_sky_new (void)
{
  return g_object_new (gthree_sky_get_type (), NULL);
}

void
gthree_sky_set_turbidity (GthreeSky *sky,
                          float      turbidity)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->turbidity = turbidity;
  g_object_notify_by_pspec (G_OBJECT (sky), obj_props[PROP_TURBIDITY]);
}

float
gthree_sky_get_turbidity (GthreeSky *sky)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);
  return priv->turbidity;
}

void
gthree_sky_set_rayleigh (GthreeSky *sky,
                         float      rayleigh)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->rayleigh = rayleigh;
  g_object_notify_by_pspec (G_OBJECT (sky), obj_props[PROP_RAYLEIGH]);
}

float
gthree_sky_get_rayleigh (GthreeSky *sky)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);
  return priv->rayleigh;
}

void
gthree_sky_set_mie_coefficient (GthreeSky *sky,
                                float      mie_coefficient)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->mie_coefficient = mie_coefficient;
  g_object_notify_by_pspec (G_OBJECT (sky), obj_props[PROP_MIE_COEFFICIENT]);
}

float
gthree_sky_get_mie_coefficient (GthreeSky *sky)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);
  return priv->mie_coefficient;
}

void
gthree_sky_set_mie_directional_g (GthreeSky *sky,
                                  float      mie_directional_g)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->mie_directional_g = mie_directional_g;
  g_object_notify_by_pspec (G_OBJECT (sky), obj_props[PROP_MIE_DIRECTIONAL_G]);
}

float
gthree_sky_get_mie_directional_g (GthreeSky *sky)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);
  return priv->mie_directional_g;
}

void
gthree_sky_set_sun_position (GthreeSky             *sky,
                             const graphene_vec3_t  *sun_position)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->sun_position = *sun_position;
  g_object_notify_by_pspec (G_OBJECT (sky), obj_props[PROP_SUN_POSITION]);
}

const graphene_vec3_t *
gthree_sky_get_sun_position (GthreeSky *sky)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);
  return &priv->sun_position;
}

void
gthree_sky_set_up (GthreeSky             *sky,
                   const graphene_vec3_t  *up)
{
  GthreeSkyPrivate *priv = gthree_sky_get_instance_private (sky);

  priv->up = *up;
}
