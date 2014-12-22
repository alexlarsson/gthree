#include <math.h>

#include "gthreebasicmaterial.h"
#include "gthreetypebuiltins.h"

typedef struct {
  GdkRGBA color;

  float reflectivity;
  float refraction_ratio;

  GthreeTexture *map;
  GthreeTexture *env_map;

  GthreeShadingType shading_type;
  GthreeColorType vertex_colors;
  GthreeOperation combine;

  gboolean skinning;
  gboolean morphTargets;
  gboolean fog;
} GthreeBasicMaterialPrivate;

enum {
  PROP_0,

  PROP_COLOR,
  PROP_ENV_MAP,
  PROP_MAP,
  PROP_SHADING_TYPE,
  PROP_VERTEX_COLORS,
  PROP_COMBINE,
  PROP_REFRACTION_RATIO,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeBasicMaterial, gthree_basic_material, GTHREE_TYPE_MATERIAL)

static void
gthree_basic_material_finalize (GObject *obj)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (obj);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  g_clear_object (&priv->map);
  g_clear_object (&priv->env_map);

  G_OBJECT_CLASS (gthree_basic_material_parent_class)->finalize (obj);
}

static void
gthree_basic_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  GTHREE_MATERIAL_CLASS (gthree_basic_material_parent_class)->set_params (material, params);

  params->vertex_colors = priv->vertex_colors;

  params->map = priv->map != NULL;
  params->env_map = priv->env_map != NULL;
}

static void
gthree_basic_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms,
                                         GthreeCamera   *camera)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);
  GthreeTexture *scale_map;
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_basic_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "map");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->map);

  /* uv repeat and offset setting priorities
   * 1. color map
   * 2. specular map
   * 3. normal map
   * 4. bump map
   * 5. alpha map
   */
  scale_map = NULL;
  if (priv->map != NULL)
    scale_map = priv->map;
  // ... other maps

  if (scale_map != NULL)
    {
      const graphene_vec2_t *repeat = gthree_texture_get_repeat (scale_map);
      const graphene_vec2_t *offset = gthree_texture_get_offset (scale_map);
      graphene_vec4_t offset_repeat;

      graphene_vec4_init (&offset_repeat,
                          graphene_vec2_get_x (offset),
                          graphene_vec2_get_y (offset),
                          graphene_vec2_get_x (repeat),
                          graphene_vec2_get_y (repeat));

      uni = gthree_uniforms_lookup_from_string (uniforms, "offsetRepeat");
      if (uni != NULL)
        gthree_uniform_set_vec4 (uni, &offset_repeat);
    }

  uni = gthree_uniforms_lookup_from_string (uniforms, "envMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->env_map);

  uni = gthree_uniforms_lookup_from_string (uniforms, "useRefract");
  if (uni != NULL)
    gthree_uniform_set_int (uni,
                            priv->env_map && gthree_texture_get_mapping (priv->env_map) == GTHREE_MAPPING_CUBE_REFRACTION);

  uni = gthree_uniforms_lookup_from_string (uniforms, "flipEnvMap");
  if (uni != NULL)
    gthree_uniform_set_float (uni, (TRUE /* TODO: material.envMap instanceof THREE.WebGLRenderTargetCube */) ? 1 : -1);

  uni = gthree_uniforms_lookup_from_string (uniforms, "combine");
  if (uni != NULL)
    gthree_uniform_set_int (uni, priv->combine);

  uni = gthree_uniforms_lookup_from_string (uniforms, "refractionRatio");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->refraction_ratio);


  // TODO: More from refreshUniformsCommon
}

static gboolean
gthree_basic_material_needs_uv (GthreeMaterial *material)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (priv->map != NULL
      /* TODO:  || material.lightMap ||
         material.bumpMap ||
         material.normalMap ||
         material.specularMap ||
         material.alphaMap */)
    return TRUE;

  return FALSE;
}

static GthreeShadingType
gthree_basic_material_needs_normals (GthreeMaterial *material)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->env_map != NULL ? GTHREE_SHADING_FLAT : GTHREE_SHADING_NONE;
}

static gboolean
gthree_basic_material_needs_camera_pos (GthreeMaterial *material)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->env_map != NULL;
}

static GthreeColorType
gthree_basic_material_needs_colors  (GthreeMaterial *material)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->vertex_colors;
}

static void
gthree_basic_material_set_property (GObject *obj,
                                    guint prop_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_basic_material_set_color (basic, g_value_get_boxed (value));
      break;

    case PROP_REFRACTION_RATIO:
      gthree_basic_material_set_refraction_ratio (basic, g_value_get_float (value));
      break;

    case PROP_SHADING_TYPE:
      gthree_basic_material_set_shading_type (basic, g_value_get_enum (value));
      break;

    case PROP_MAP:
      gthree_basic_material_set_map (basic, g_value_get_object (value));
      break;

    case PROP_ENV_MAP:
      gthree_basic_material_set_env_map (basic, g_value_get_object (value));
      break;

    case PROP_VERTEX_COLORS:
      gthree_basic_material_set_vertex_colors (basic, g_value_get_enum (value));
      break;

    case PROP_COMBINE:
      gthree_basic_material_set_combine (basic, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_basic_material_get_property (GObject *obj,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (obj);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_REFRACTION_RATIO:
      g_value_set_float (value, priv->refraction_ratio);
      break;

    case PROP_SHADING_TYPE:
      g_value_set_enum (value, priv->shading_type);
      break;

    case PROP_MAP:
      g_value_set_object (value, priv->map);
      break;

    case PROP_ENV_MAP:
      g_value_set_object (value, priv->env_map);
      break;

    case PROP_VERTEX_COLORS:
      g_value_set_enum (value, priv->vertex_colors);
      break;

    case PROP_COMBINE:
      g_value_set_enum (value, priv->combine);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_basic_material_class_init (GthreeBasicMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_basic_material_set_property;
  gobject_class->get_property = gthree_basic_material_get_property;
  gobject_class->finalize = gthree_basic_material_finalize;

  material_class->set_params = gthree_basic_material_real_set_params;
  material_class->set_uniforms = gthree_basic_material_real_set_uniforms;
  material_class->needs_uv = gthree_basic_material_needs_uv;
  material_class->needs_normals = gthree_basic_material_needs_normals;
  material_class->needs_camera_pos = gthree_basic_material_needs_camera_pos;
  material_class->needs_colors = gthree_basic_material_needs_colors;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_COMBINE] =
    g_param_spec_enum ("combine", "Combine", "Combine",
                       GTHREE_TYPE_OPERATION,
                       GTHREE_OPERATION_MULTIPLY,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_VERTEX_COLORS] =
    g_param_spec_enum ("vertex-colors", "Vertex Colors", "Vertex Colors",
                       GTHREE_TYPE_COLOR_TYPE,
                       GTHREE_COLOR_NONE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHADING_TYPE] =
    g_param_spec_enum ("shading-type", "Shading Type", "Shading Type",
                       GTHREE_TYPE_SHADING_TYPE,
                       GTHREE_SHADING_SMOOTH,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_REFRACTION_RATIO] =
    g_param_spec_float ("refraction-ratio", "Refraction Ratio", "Refraction Ratio",
                        0.f, 1.f, 0.98f,
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
gthree_basic_material_init (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->combine = GTHREE_OPERATION_MULTIPLY;
  priv->vertex_colors = GTHREE_COLOR_NONE;
  priv->shading_type = GTHREE_SHADING_SMOOTH;

  priv->reflectivity = 1;
  priv->refraction_ratio = 0.98;
}

GthreeBasicMaterial *
gthree_basic_material_new (void)
{
  return g_object_new (gthree_basic_material_get_type (), NULL);
}

const GdkRGBA *
gthree_basic_material_get_color (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return &priv->color;
}

void
gthree_basic_material_set_color (GthreeBasicMaterial *basic,
                                 const GdkRGBA *color)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_COLOR]);
}

float
gthree_basic_material_get_refraction_ratio (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->refraction_ratio;
}

void
gthree_basic_material_set_refraction_ratio (GthreeBasicMaterial *basic,
                                            float                ratio)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  priv->refraction_ratio = ratio;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_REFRACTION_RATIO]);
}

GthreeShadingType
gthree_basic_material_get_shading_type (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->shading_type;
}

void
gthree_basic_material_set_shading_type (GthreeBasicMaterial *basic,
                                        GthreeShadingType    shading_type)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (priv->shading_type == shading_type)
    return;

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_SHADING_TYPE]);
}

void
gthree_basic_material_set_map (GthreeBasicMaterial *basic,
                               GthreeTexture *texture)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

      g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_MAP]);
    }
}

GthreeTexture *
gthree_basic_material_get_map (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->map;
}

void
gthree_basic_material_set_env_map (GthreeBasicMaterial *basic,
                                   GthreeTexture *texture)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (g_set_object (&priv->env_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

      g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_ENV_MAP]);
    }
}

GthreeTexture *
gthree_basic_material_get_env_map (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->env_map;
}

void
gthree_basic_material_set_vertex_colors (GthreeBasicMaterial *basic,
                                         GthreeColorType color_type)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (priv->vertex_colors == color_type)
    return;

  priv->vertex_colors = color_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_VERTEX_COLORS]);
}

GthreeColorType
gthree_basic_material_get_vertex_colors (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->vertex_colors;
}

void
gthree_basic_material_set_combine (GthreeBasicMaterial *basic,
                                   GthreeOperation combine)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (priv->combine == combine)
    return;

  priv->combine = combine;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (basic), obj_props[PROP_COMBINE]);
}

GthreeOperation
gthree_basic_material_get_combine (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->combine;
}
