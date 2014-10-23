#include <math.h>
#include <epoxy/gl.h>

#include "gthreephongmaterial.h"

typedef struct {
  GdkRGBA color;
  GdkRGBA ambient;
  GdkRGBA emissive;
  GdkRGBA specular;
  float shininess;

  gboolean metal;
  
  float reflectivity;
  float refraction_ratio;

  GthreeTexture *map;

  GthreeShadingType shading_type;
  GthreeColorType vertex_colors;
  GthreeOperation combine;

  gboolean wrap_around;
  graphene_vec3_t wrap_rgb;
  
  gboolean skinning;
  gboolean morphTargets;
  gboolean fog;
} GthreePhongMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreePhongMaterial, gthree_phong_material, GTHREE_TYPE_MATERIAL);

GthreePhongMaterial *
gthree_phong_material_new ()
{
  GthreePhongMaterial *material;

  material = g_object_new (gthree_phong_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_phong_material_init (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->ambient.red = 1.0;
  priv->ambient.green = 1.0;
  priv->ambient.blue = 1.0;
  priv->ambient.alpha = 1.0;

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;

  priv->specular.red = 0.07;
  priv->specular.green = 0.07;
  priv->specular.blue = 0.07;
  priv->specular.alpha = 1.0;

  priv->shininess = 30;
  
  priv->combine = GTHREE_OPERATION_MULTIPLY;
  priv->vertex_colors = GTHREE_COLOR_NONE;
  priv->shading_type = GTHREE_SHADING_SMOOTH;

  priv->reflectivity = 1;
  priv->refraction_ratio = 0.98;

  priv->metal = FALSE;
}

static void
gthree_phong_material_finalize (GObject *obj)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (obj);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  g_clear_object (&priv->map);

  G_OBJECT_CLASS (gthree_phong_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_phong_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("phong");
}

static void
gthree_phong_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  GTHREE_MATERIAL_CLASS (gthree_phong_material_parent_class)->set_params (material, params);

  params->vertex_colors = priv->vertex_colors;
  params->wrap_around = priv->wrap_around;
  params->metal = priv->metal;

  params->map = priv->map != NULL;
}

static void
gthree_phong_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);
  GthreeTexture *scale_map;
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_phong_material_parent_class)->set_uniforms (material, uniforms);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "map");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->map);

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

  uni = gthree_uniforms_lookup_from_string (uniforms, "combine");
  if (uni != NULL)
    gthree_uniform_set_int (uni, priv->combine);


  // TODO: More from refreshUniformsCommon

  // phong specific:

  uni = gthree_uniforms_lookup_from_string (uniforms, "shininess");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->shininess);
  
#if TODO
  if ( _this.gammaInput )
    {
      uniforms.ambient.value.copyGammaToLinear( material.ambient );
      uniforms.emissive.value.copyGammaToLinear( material.emissive );
      uniforms.specular.value.copyGammaToLinear( material.specular );
    }
  else
#endif
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "ambient");
      if (uni != NULL)
	gthree_uniform_set_color (uni, &priv->ambient);

      uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
      if (uni != NULL)
	gthree_uniform_set_color (uni, &priv->emissive);

      uni = gthree_uniforms_lookup_from_string (uniforms, "specular");
      if (uni != NULL)
	gthree_uniform_set_color (uni, &priv->specular);
    }

#if TODO
  if (priv->wrap_around )
    {
      uniforms.wrapRGB.value.copy( material.wrapRGB );
    }
#endif
  

}

static gboolean
gthree_phong_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_phong_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_phong_material_needs_uv (GthreeMaterial *material)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

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
gthree_phong_material_needs_normals (GthreeMaterial *material)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  if (priv->shading_type == GTHREE_SHADING_SMOOTH)
    return GTHREE_SHADING_SMOOTH;

  return GTHREE_SHADING_FLAT;
}

static GthreeColorType
gthree_phong_material_needs_colors  (GthreeMaterial *material)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return priv->vertex_colors;
}

static void
gthree_phong_material_class_init (GthreePhongMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_phong_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_phong_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_phong_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_phong_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_view_matrix = gthree_phong_material_needs_view_matrix;
  GTHREE_MATERIAL_CLASS(klass)->needs_lights = gthree_phong_material_needs_lights;
  GTHREE_MATERIAL_CLASS(klass)->needs_uv = gthree_phong_material_needs_uv;
  GTHREE_MATERIAL_CLASS(klass)->needs_normals = gthree_phong_material_needs_normals;
  GTHREE_MATERIAL_CLASS(klass)->needs_colors = gthree_phong_material_needs_colors;
}

const GdkRGBA *
gthree_phong_material_get_color (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return &priv->color;
}

void
gthree_phong_material_set_color (GthreePhongMaterial *phong,
				   const GdkRGBA *color)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->color = *color;

  phong->parent.needs_update = TRUE;
}

const GdkRGBA *
gthree_phong_material_get_ambient_color (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return &priv->ambient;
}

void
gthree_phong_material_set_ambient_color (GthreePhongMaterial *phong,
				  	 const GdkRGBA *color)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->ambient = *color;

  phong->parent.needs_update = TRUE;
}

const GdkRGBA *
gthree_phong_material_get_emissive_color (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return &priv->emissive;
}

void
gthree_phong_material_set_emissive_color (GthreePhongMaterial *phong,
				  	 const GdkRGBA *color)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->emissive = *color;

  phong->parent.needs_update = TRUE;
}

const GdkRGBA *
gthree_phong_material_get_specular_color (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return &priv->specular;
}

void
gthree_phong_material_set_specular_color (GthreePhongMaterial *phong,
				  	 const GdkRGBA *color)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->specular = *color;

  phong->parent.needs_update = TRUE;
}

void
gthree_phong_material_set_vertex_colors (GthreePhongMaterial *phong,
					   GthreeColorType color_type)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->vertex_colors = color_type;

  phong->parent.needs_update = TRUE;
}


void
gthree_phong_material_set_shading_type (GthreePhongMaterial *phong,
					  GthreeShadingType shading_type)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->shading_type = shading_type;

  phong->parent.needs_update = TRUE;
}

void
gthree_phong_material_set_map (GthreePhongMaterial *phong,
                               GthreeTexture *texture)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  if (texture)
    g_object_ref (texture);
  if (priv->map)
    g_object_unref (priv->map);

  priv->map = texture;

  phong->parent.needs_update = TRUE;
}

GthreeTexture *
gthree_phong_material_get_map (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return priv->map;
}

GthreeColorType
gthree_phong_material_get_vertex_colors (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return priv->vertex_colors;
}
