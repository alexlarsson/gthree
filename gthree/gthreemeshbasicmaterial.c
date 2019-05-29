#include <math.h>

#include "gthreemeshbasicmaterial.h"
#include "gthreetypebuiltins.h"
#include "gthreecubetexture.h"

typedef struct {
  GdkRGBA color;

  float reflectivity;
  float refraction_ratio;

  GthreeTexture *map;
  GthreeTexture *env_map;

  GthreeOperation combine;

  gboolean fog;
} GthreeMeshBasicMaterialPrivate;

enum {
  PROP_0,

  PROP_COLOR,
  PROP_ENV_MAP,
  PROP_MAP,
  PROP_COMBINE,
  PROP_REFLECTIVITY,
  PROP_REFRACTION_RATIO,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshBasicMaterial, gthree_mesh_basic_material, GTHREE_TYPE_MESH_MATERIAL)

static void
gthree_mesh_basic_material_finalize (GObject *obj)
{
  GthreeMeshBasicMaterial *basic = GTHREE_BASIC_MATERIAL (obj);
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

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

  G_OBJECT_CLASS (gthree_mesh_basic_material_parent_class)->finalize (obj);
}

static void
gthree_mesh_basic_material_real_set_params (GthreeMaterial *material,
                                            GthreeProgramParameters *params)
{
  GthreeMeshBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  GTHREE_MATERIAL_CLASS (gthree_mesh_basic_material_parent_class)->set_params (material, params);

  params->map = priv->map != NULL;
  params->env_map = priv->env_map != NULL;
  if (params->env_map)
    params->env_map_mode = gthree_texture_get_mapping (priv->env_map);
}

static void
gthree_mesh_basic_material_real_set_uniforms (GthreeMaterial *material,
                                              GthreeUniforms *uniforms,
                                              GthreeCamera   *camera)
{
  GthreeMeshBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);
  GthreeTexture *scale_map;
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_basic_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  //TODO: from refreshUniformsCommon 
  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");

  uni = gthree_uniforms_lookup_from_string (uniforms, "map");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->map);

  //TODO: from refreshUniformsCommon 
  uni = gthree_uniforms_lookup_from_string (uniforms, "alphaMap");

  //TODO: from refreshUniformsCommon 
  uni = gthree_uniforms_lookup_from_string (uniforms, "specularMap");

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

  //TODO: from refreshUniformsCommon 
  uni = gthree_uniforms_lookup_from_string (uniforms, "lightMap");
  uni = gthree_uniforms_lookup_from_string (uniforms, "lightMapIntensity");

  //TODO: from refreshUniformsCommon 
  uni = gthree_uniforms_lookup_from_string (uniforms, "aoMap");
  uni = gthree_uniforms_lookup_from_string (uniforms, "aoMapIntensity");

  // uv repeat and offset setting priorities
  // 1. color map
  // 2. specular map
  // 3. normal map
  // 4. bump map
  // 5. alpha map
  // 6. emissive map

  scale_map = NULL;
  if (priv->map != NULL)
    scale_map = priv->map;
  // ... other maps

  if (scale_map != NULL)
    {
      /* TODO: from refreshUniformsCommon
      if ( uvScaleMap.matrixAutoUpdate === true ) {
        uvScaleMap.updateMatrix();
      }
      */

      //TODO: from refreshUniformsCommon
      uni = gthree_uniforms_lookup_from_string (uniforms, "uvTransform");
    }
}

static gboolean
gthree_mesh_basic_material_needs_camera_pos (GthreeMaterial *material)
{
  GthreeMeshBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return priv->env_map != NULL;
}

static void
gthree_mesh_basic_material_set_property (GObject *obj,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMeshBasicMaterial *basic = GTHREE_BASIC_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_mesh_basic_material_set_color (basic, g_value_get_boxed (value));
      break;

    case PROP_REFRACTION_RATIO:
      gthree_mesh_basic_material_set_refraction_ratio (basic, g_value_get_float (value));
      break;

    case PROP_REFLECTIVITY:
      gthree_mesh_basic_material_set_refraction_ratio (basic, g_value_get_float (value));
      break;

    case PROP_MAP:
      gthree_mesh_basic_material_set_map (basic, g_value_get_object (value));
      break;

    case PROP_ENV_MAP:
      gthree_mesh_basic_material_set_env_map (basic, g_value_get_object (value));
      break;

    case PROP_COMBINE:
      gthree_mesh_basic_material_set_combine (basic, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_basic_material_get_property (GObject *obj,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMeshBasicMaterial *basic = GTHREE_BASIC_MATERIAL (obj);
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
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
gthree_mesh_basic_material_class_init (GthreeMeshBasicMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_mesh_basic_material_set_property;
  gobject_class->get_property = gthree_mesh_basic_material_get_property;
  gobject_class->finalize = gthree_mesh_basic_material_finalize;

  material_class->set_params = gthree_mesh_basic_material_real_set_params;
  material_class->set_uniforms = gthree_mesh_basic_material_real_set_uniforms;
  material_class->needs_camera_pos = gthree_mesh_basic_material_needs_camera_pos;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
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

static void
gthree_mesh_basic_material_init (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->combine = GTHREE_OPERATION_MULTIPLY;

  priv->reflectivity = 1;
  priv->refraction_ratio = 0.98;
}

GthreeMeshBasicMaterial *
gthree_mesh_basic_material_new (void)
{
  return g_object_new (gthree_mesh_basic_material_get_type (), NULL);
}

const GdkRGBA *
gthree_mesh_basic_material_get_color (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return &priv->color;
}

void
gthree_mesh_basic_material_set_color (GthreeMeshBasicMaterial *basic,
                                      const GdkRGBA *color)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_COLOR]);
}

float
gthree_mesh_basic_material_get_refraction_ratio (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return priv->refraction_ratio;
}

void
gthree_mesh_basic_material_set_refraction_ratio (GthreeMeshBasicMaterial *basic,
                                                 float                ratio)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  priv->refraction_ratio = ratio;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_REFRACTION_RATIO]);
}

float
gthree_mesh_basic_material_get_reflectivity (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return priv->reflectivity;
}

void
gthree_mesh_basic_material_set_reflectivity (GthreeMeshBasicMaterial *basic,
                                             float  reflectivity)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  priv->reflectivity = reflectivity;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_REFRACTION_RATIO]);
}

void
gthree_mesh_basic_material_set_map (GthreeMeshBasicMaterial *basic,
                                    GthreeTexture *texture)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->map));

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

      g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_MAP]);
    }
}

GthreeTexture *
gthree_mesh_basic_material_get_map (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return priv->map;
}

void
gthree_mesh_basic_material_set_env_map (GthreeMeshBasicMaterial *basic,
                                        GthreeTexture *texture)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->env_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->env_map));

  if (g_set_object (&priv->env_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

      g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_ENV_MAP]);
    }
}

GthreeTexture *
gthree_mesh_basic_material_get_env_map (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return priv->env_map;
}

void
gthree_mesh_basic_material_set_combine (GthreeMeshBasicMaterial *basic,
                                        GthreeOperation combine)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  if (priv->combine == combine)
    return;

  priv->combine = combine;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_COMBINE]);
}

GthreeOperation
gthree_mesh_basic_material_get_combine (GthreeMeshBasicMaterial *basic)
{
  GthreeMeshBasicMaterialPrivate *priv = gthree_mesh_basic_material_get_instance_private (basic);

  return priv->combine;
}
