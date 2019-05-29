#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshphongmaterial.h"
#include "gthreecubetexture.h"
#include "gthreetypebuiltins.h"

typedef struct {
  GdkRGBA color;
  GdkRGBA emissive;
  GdkRGBA specular;
  float shininess;
  float reflectivity;
  float refraction_ratio;
  GthreeOperation combine;
  GthreeShadingType shading_type;

  GthreeTexture *map;
  GthreeTexture *env_map;

} GthreeMeshPhongMaterialPrivate;


enum {
  PROP_0,

  PROP_COLOR,
  PROP_EMISSIVE_COLOR,
  PROP_SPECULAR_COLOR,
  PROP_REFLECTIVITY,
  PROP_REFRACTION_RATIO,
  PROP_MAP,
  PROP_ENV_MAP,
  PROP_COMBINE,
  PROP_SHININESS,
  PROP_SHADING_TYPE,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshPhongMaterial, gthree_mesh_phong_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshPhongMaterial *
gthree_mesh_phong_material_new ()
{
  GthreeMeshPhongMaterial *material;

  material = g_object_new (gthree_mesh_phong_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_phong_material_init (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;

  priv->specular.red = 0.07;
  priv->specular.green = 0.07;
  priv->specular.blue = 0.07;
  priv->specular.alpha = 1.0;

  priv->combine = GTHREE_OPERATION_MULTIPLY;

  priv->shading_type = GTHREE_SHADING_SMOOTH;

  priv->reflectivity = 1;
  priv->refraction_ratio = 0.98;
  priv->shininess = 30;
}

static void
gthree_mesh_phong_material_finalize (GObject *obj)
{
  GthreeMeshPhongMaterial *phong = GTHREE_PHONG_MATERIAL (obj);
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

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

  G_OBJECT_CLASS (gthree_mesh_phong_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_phong_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("phong");
}

static void
gthree_mesh_phong_material_real_set_params (GthreeMaterial *material,
                                            GthreeProgramParameters *params)
{
  GthreeMeshPhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  params->map = priv->map != NULL;
  params->env_map = priv->env_map != NULL;
  if (params->env_map)
    params->env_map_mode = gthree_texture_get_mapping (priv->env_map);

  params->flat_shading = priv->shading_type == GTHREE_SHADING_FLAT;

  GTHREE_MATERIAL_CLASS (gthree_mesh_phong_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_phong_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms,
                                         GthreeCamera   *camera)
{
  GthreeMeshPhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_phong_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->emissive);

  uni = gthree_uniforms_lookup_from_string (uniforms, "specular");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->specular);

  uni = gthree_uniforms_lookup_from_string (uniforms, "shininess");
  if (uni != NULL)
    gthree_uniform_set_float (uni, MAX (priv->shininess, 1e-4 )); // to prevent pow( 0.0, 0.0 )

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
gthree_mesh_phong_material_needs_camera_pos (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_phong_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_phong_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}

static void
gthree_mesh_phong_material_set_property (GObject *obj,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMeshPhongMaterial *phong = GTHREE_PHONG_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_mesh_phong_material_set_color (phong, g_value_get_boxed (value));
      break;

    case PROP_EMISSIVE_COLOR:
      gthree_mesh_phong_material_set_emissive_color (phong, g_value_get_boxed (value));
      break;

    case PROP_SPECULAR_COLOR:
      gthree_mesh_phong_material_set_specular_color (phong, g_value_get_boxed (value));
      break;

    case PROP_REFRACTION_RATIO:
      gthree_mesh_phong_material_set_refraction_ratio (phong, g_value_get_float (value));
      break;

    case PROP_REFLECTIVITY:
      gthree_mesh_phong_material_set_refraction_ratio (phong, g_value_get_float (value));
      break;

    case PROP_SHININESS:
      gthree_mesh_phong_material_set_shininess (phong, g_value_get_float (value));
      break;

    case PROP_MAP:
      gthree_mesh_phong_material_set_map (phong, g_value_get_object (value));
      break;

    case PROP_ENV_MAP:
      gthree_mesh_phong_material_set_env_map (phong, g_value_get_object (value));
      break;

    case PROP_COMBINE:
      gthree_mesh_phong_material_set_combine (phong, g_value_get_enum (value));
      break;

    case PROP_SHADING_TYPE:
      gthree_mesh_phong_material_set_shading_type (phong, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_phong_material_get_property (GObject *obj,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMeshPhongMaterial *phong = GTHREE_PHONG_MATERIAL (obj);
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_EMISSIVE_COLOR:
      g_value_set_boxed (value, &priv->emissive);
      break;

    case PROP_SPECULAR_COLOR:
      g_value_set_boxed (value, &priv->specular);
      break;

    case PROP_REFRACTION_RATIO:
      g_value_set_float (value, priv->refraction_ratio);
      break;

    case PROP_REFLECTIVITY:
      g_value_set_float (value, priv->reflectivity);
      break;

    case PROP_SHININESS:
      g_value_set_float (value, priv->shininess);
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

    case PROP_SHADING_TYPE:
      g_value_set_enum (value, priv->shading_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}


static void
gthree_mesh_phong_material_class_init (GthreeMeshPhongMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->finalize = gthree_mesh_phong_material_finalize;
  gobject_class->set_property = gthree_mesh_phong_material_set_property;
  gobject_class->get_property = gthree_mesh_phong_material_get_property;

  material_class->get_shader = gthree_mesh_phong_material_real_get_shader;
  material_class->set_params = gthree_mesh_phong_material_real_set_params;
  material_class->set_uniforms = gthree_mesh_phong_material_real_set_uniforms;
  material_class->needs_camera_pos = gthree_mesh_phong_material_needs_camera_pos;
  material_class->needs_view_matrix = gthree_mesh_phong_material_needs_view_matrix;
  material_class->needs_lights = gthree_mesh_phong_material_needs_lights;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_COLOR] =
    g_param_spec_boxed ("emissive-color", "Emissive Color", "Emissive",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_COLOR] =
    g_param_spec_boxed ("specular-color", "Specular color", "Specular color",
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
  obj_props[PROP_SHININESS] =
    g_param_spec_float ("shininess", "Shininess", "Shininess",
                        0.f, 1000.f, 30.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MAP] =
    g_param_spec_object ("map", "Map", "Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ENV_MAP] =
    g_param_spec_object ("env-map", "Env Map", "Env Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHADING_TYPE] =
    g_param_spec_enum ("shading-type", "Shading Type", "Shading Type",
                       GTHREE_TYPE_SHADING_TYPE,
                       GTHREE_SHADING_SMOOTH,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

const GdkRGBA *
gthree_mesh_phong_material_get_emissive_color (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return &priv->emissive;
}

void
gthree_mesh_phong_material_set_emissive_color (GthreeMeshPhongMaterial *phong,
                                               const GdkRGBA *color)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  priv->emissive = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);
}

const GdkRGBA *
gthree_mesh_phong_material_get_specular_color (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return &priv->specular;
}

void
gthree_mesh_phong_material_set_specular_color (GthreeMeshPhongMaterial *phong,
                                               const GdkRGBA *color)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  priv->specular = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);
}

float
gthree_mesh_phong_material_get_shininess (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->shininess;
}

void
gthree_mesh_phong_material_set_shininess (GthreeMeshPhongMaterial *phong,
                                          float                shininess)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  priv->shininess = shininess;
  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);
}

GthreeShadingType
gthree_mesh_phong_material_get_shading_type (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->shading_type;
}

void
gthree_mesh_phong_material_set_shading_type (GthreeMeshPhongMaterial *phong,
                                             GthreeShadingType    shading_type)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  if (priv->shading_type == shading_type)
    return;

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

  g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_SHADING_TYPE]);
}

float
gthree_mesh_phong_material_get_refraction_ratio (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->refraction_ratio;
}

void
gthree_mesh_phong_material_set_refraction_ratio (GthreeMeshPhongMaterial *phong,
                                                   float                ratio)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  priv->refraction_ratio = ratio;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

  g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_REFRACTION_RATIO]);
}

float
gthree_mesh_phong_material_get_reflectivity (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->reflectivity;
}

void
gthree_mesh_phong_material_set_reflectivity (GthreeMeshPhongMaterial *phong,
                                               float  reflectivity)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  priv->reflectivity = reflectivity;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

  g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_REFRACTION_RATIO]);
}

const GdkRGBA *
gthree_mesh_phong_material_get_color (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return &priv->color;
}

void
gthree_mesh_phong_material_set_color (GthreeMeshPhongMaterial *phong,
                                      const GdkRGBA *color)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

  g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_COLOR]);
}

void
gthree_mesh_phong_material_set_map (GthreeMeshPhongMaterial *phong,
                                    GthreeTexture *texture)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->map));

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

      g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_MAP]);
    }
}

GthreeTexture *
gthree_mesh_phong_material_get_map (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->map;
}

void
gthree_mesh_phong_material_set_env_map (GthreeMeshPhongMaterial *phong,
                                          GthreeTexture *texture)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->env_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->env_map));

  if (g_set_object (&priv->env_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

      g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_ENV_MAP]);
    }
}

GthreeTexture *
gthree_mesh_phong_material_get_env_map (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->env_map;
}

void
gthree_mesh_phong_material_set_combine (GthreeMeshPhongMaterial *phong,
                                          GthreeOperation combine)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  if (priv->combine == combine)
    return;

  priv->combine = combine;

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);

  g_object_notify_by_pspec (G_OBJECT (phong), obj_props[PROP_COMBINE]);
}

GthreeOperation
gthree_mesh_phong_material_get_combine (GthreeMeshPhongMaterial *phong)
{
  GthreeMeshPhongMaterialPrivate *priv = gthree_mesh_phong_material_get_instance_private (phong);

  return priv->combine;
}
