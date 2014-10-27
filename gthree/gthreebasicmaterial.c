#include <math.h>
#include <epoxy/gl.h>

#include "gthreebasicmaterial.h"

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


G_DEFINE_TYPE_WITH_PRIVATE (GthreeBasicMaterial, gthree_basic_material, GTHREE_TYPE_MATERIAL);

GthreeBasicMaterial *
gthree_basic_material_new ()
{
  GthreeBasicMaterial *material;

  material = g_object_new (gthree_basic_material_get_type (),
                           NULL);

  return material;
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
gthree_basic_material_class_init (GthreeBasicMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_basic_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_basic_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_basic_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_uv = gthree_basic_material_needs_uv;
  GTHREE_MATERIAL_CLASS(klass)->needs_normals = gthree_basic_material_needs_normals;
  GTHREE_MATERIAL_CLASS(klass)->needs_camera_pos = gthree_basic_material_needs_camera_pos;
  GTHREE_MATERIAL_CLASS(klass)->needs_colors = gthree_basic_material_needs_colors;
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

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);
}

void
gthree_basic_material_set_vertex_colors (GthreeBasicMaterial *basic,
                                         GthreeColorType color_type)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  priv->vertex_colors = color_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);
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

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);
}

void
gthree_basic_material_set_map (GthreeBasicMaterial *basic,
                               GthreeTexture *texture)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  if (texture)
    g_object_ref (texture);
  if (priv->map)
    g_object_unref (priv->map);

  priv->map = texture;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);
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

  if (texture)
    g_object_ref (texture);
  if (priv->env_map)
    g_object_unref (priv->env_map);

  priv->env_map = texture;

  gthree_material_set_needs_update (GTHREE_MATERIAL (basic), TRUE);
}

GthreeTexture *
gthree_basic_material_get_env_map (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->env_map;
}

GthreeColorType
gthree_basic_material_get_vertex_colors (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->vertex_colors;
}
