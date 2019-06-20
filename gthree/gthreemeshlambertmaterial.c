#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshlambertmaterial.h"
#include "gthreecubetexture.h"
#include "gthreetypebuiltins.h"

typedef struct {
  GdkRGBA color;
  GdkRGBA emissive;
  float reflectivity;
  float refraction_ratio;
  GthreeOperation combine;

  GthreeTexture *map;
  GthreeTexture *env_map;

} GthreeMeshLambertMaterialPrivate;

enum {
  PROP_0,

  PROP_COLOR,
  PROP_EMISSIVE_COLOR,
  PROP_REFLECTIVITY,
  PROP_REFRACTION_RATIO,
  PROP_MAP,
  PROP_ENV_MAP,
  PROP_COMBINE,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshLambertMaterial, gthree_mesh_lambert_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshLambertMaterial *
gthree_mesh_lambert_material_new ()
{
  GthreeMeshLambertMaterial *material;

  material = g_object_new (gthree_mesh_lambert_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_lambert_material_init (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;

  priv->combine = GTHREE_OPERATION_MULTIPLY;

  priv->reflectivity = 1;
  priv->refraction_ratio = 0.98;
}

static void
gthree_mesh_lambert_material_finalize (GObject *obj)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (obj);
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  if (priv->map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->map));
      g_clear_object (&priv->map);
    }

  if (priv->env_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->env_map));
      g_clear_object (&priv->env_map);
    }

  G_OBJECT_CLASS (gthree_mesh_lambert_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_lambert_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("lambert");
}

static void
gthree_mesh_lambert_material_real_set_params (GthreeMaterial *material,
                                              GthreeProgramParameters *params)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (material);
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  params->map = priv->map != NULL;
  if (params->map)
    params->map_encoding = gthree_texture_get_encoding (priv->map);

  params->env_map = priv->env_map != NULL;
  if (params->env_map)
    {
      params->env_map_encoding = gthree_texture_get_encoding (priv->env_map);
      params->env_map_mode = gthree_texture_get_mapping (priv->env_map);
    }

  GTHREE_MATERIAL_CLASS (gthree_mesh_lambert_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_lambert_material_real_set_uniforms (GthreeMaterial *material,
                                                GthreeUniforms *uniforms,
                                                GthreeCamera   *camera)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (material);
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_lambert_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->emissive);

  uni = gthree_uniforms_lookup_from_string (uniforms, "map");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->map);

  if (priv->env_map)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "envMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->env_map);

      uni = gthree_uniforms_lookup_from_string (uniforms, "flipEnvMap");
      if (uni != NULL)
        gthree_uniform_set_float (uni, GTHREE_IS_CUBE_TEXTURE (priv->env_map) ? -1 : 1);

      uni = gthree_uniforms_lookup_from_string (uniforms, "reflectivity");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->reflectivity);

      uni = gthree_uniforms_lookup_from_string (uniforms, "refractionRatio");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->refraction_ratio);
    }
}

static gboolean
gthree_mesh_lambert_material_needs_camera_pos (GthreeMaterial *material)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (material);
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return priv->env_map != NULL;
}

static gboolean
gthree_mesh_lambert_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_lambert_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}


static void
gthree_mesh_lambert_material_set_property (GObject *obj,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_mesh_lambert_material_set_color (lambert, g_value_get_boxed (value));
      break;

    case PROP_EMISSIVE_COLOR:
      gthree_mesh_lambert_material_set_emissive_color (lambert, g_value_get_boxed (value));
      break;

    case PROP_REFRACTION_RATIO:
      gthree_mesh_lambert_material_set_refraction_ratio (lambert, g_value_get_float (value));
      break;

    case PROP_REFLECTIVITY:
      gthree_mesh_lambert_material_set_refraction_ratio (lambert, g_value_get_float (value));
      break;

    case PROP_MAP:
      gthree_mesh_lambert_material_set_map (lambert, g_value_get_object (value));
      break;

    case PROP_ENV_MAP:
      gthree_mesh_lambert_material_set_env_map (lambert, g_value_get_object (value));
      break;

    case PROP_COMBINE:
      gthree_mesh_lambert_material_set_combine (lambert, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_lambert_material_get_property (GObject *obj,
                                           guint prop_id,
                                           GValue *value,
                                           GParamSpec *pspec)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (obj);
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_EMISSIVE_COLOR:
      g_value_set_boxed (value, &priv->emissive);
      break;

    case PROP_REFRACTION_RATIO:
      g_value_set_float (value, priv->refraction_ratio);
      break;

    case PROP_REFLECTIVITY:
      g_value_set_float (value, priv->reflectivity);
      break;

    case PROP_MAP:
      g_value_set_object (value, priv->map);
      break;

    case PROP_ENV_MAP:
      g_value_set_object (value, priv->env_map);
      break;

    case PROP_COMBINE:
      g_value_set_enum (value, priv->combine);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_lambert_material_class_init (GthreeMeshLambertMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_mesh_lambert_material_set_property;
  gobject_class->get_property = gthree_mesh_lambert_material_get_property;
  gobject_class->finalize = gthree_mesh_lambert_material_finalize;

  material_class->get_shader = gthree_mesh_lambert_material_real_get_shader;
  material_class->set_params = gthree_mesh_lambert_material_real_set_params;
  material_class->set_uniforms = gthree_mesh_lambert_material_real_set_uniforms;
  material_class->needs_view_matrix = gthree_mesh_lambert_material_needs_view_matrix;
  material_class->needs_lights = gthree_mesh_lambert_material_needs_lights;
  material_class->needs_camera_pos = gthree_mesh_lambert_material_needs_camera_pos;


  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_COLOR] =
    g_param_spec_boxed ("emissive-color", "Emissive Color", "Emissive",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_COMBINE] =
    g_param_spec_enum ("combine", "Combine", "Combine",
                       GTHREE_TYPE_OPERATION,
                       GTHREE_OPERATION_MULTIPLY,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_REFRACTION_RATIO] =
    g_param_spec_float ("refraction-ratio", "Refraction Ratio", "Refraction Ratio",
                        0.f, 1.f, 0.98f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_REFLECTIVITY] =
    g_param_spec_float ("reflectivity", "Reflectivity", "Reflectivity",
                        0.f, 1.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MAP] =
    g_param_spec_object ("map", "Map", "Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ENV_MAP] =
    g_param_spec_object ("env-map", "Env Map", "Env Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

const GdkRGBA *
gthree_mesh_lambert_material_get_emissive_color (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return &priv->emissive;
}

void
gthree_mesh_lambert_material_set_emissive_color (GthreeMeshLambertMaterial *lambert,
                                                 const GdkRGBA *color)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  priv->emissive = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);
}

const GdkRGBA *
gthree_mesh_lambert_material_get_color (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return &priv->color;
}

void
gthree_mesh_lambert_material_set_color (GthreeMeshLambertMaterial *lambert,
                                 const GdkRGBA *color)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);

  g_object_notify_by_pspec (G_OBJECT (lambert), obj_props[PROP_COLOR]);
}

float
gthree_mesh_lambert_material_get_refraction_ratio (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return priv->refraction_ratio;
}

void
gthree_mesh_lambert_material_set_refraction_ratio (GthreeMeshLambertMaterial *lambert,
                                                   float                ratio)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  priv->refraction_ratio = ratio;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);

  g_object_notify_by_pspec (G_OBJECT (lambert), obj_props[PROP_REFRACTION_RATIO]);
}

float
gthree_mesh_lambert_material_get_reflectivity (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return priv->reflectivity;
}

void
gthree_mesh_lambert_material_set_reflectivity (GthreeMeshLambertMaterial *lambert,
                                               float  reflectivity)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  priv->reflectivity = reflectivity;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);

  g_object_notify_by_pspec (G_OBJECT (lambert), obj_props[PROP_REFRACTION_RATIO]);
}


void
gthree_mesh_lambert_material_set_map (GthreeMeshLambertMaterial *lambert,
                                      GthreeTexture *texture)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->map));

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);

      g_object_notify_by_pspec (G_OBJECT (lambert), obj_props[PROP_MAP]);
    }
}

GthreeTexture *
gthree_mesh_lambert_material_get_map (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return priv->map;
}

void
gthree_mesh_lambert_material_set_env_map (GthreeMeshLambertMaterial *lambert,
                                          GthreeTexture *texture)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->env_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->env_map));

  if (g_set_object (&priv->env_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);

      g_object_notify_by_pspec (G_OBJECT (lambert), obj_props[PROP_ENV_MAP]);
    }
}

GthreeTexture *
gthree_mesh_lambert_material_get_env_map (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return priv->env_map;
}

void
gthree_mesh_lambert_material_set_combine (GthreeMeshLambertMaterial *lambert,
                                          GthreeOperation combine)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  if (priv->combine == combine)
    return;

  priv->combine = combine;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);

  g_object_notify_by_pspec (G_OBJECT (lambert), obj_props[PROP_COMBINE]);
}

GthreeOperation
gthree_mesh_lambert_material_get_combine (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return priv->combine;
}
