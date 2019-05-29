#include <math.h>
#include <epoxy/gl.h>

#include "gthreephongmaterial.h"

typedef struct {
  GdkRGBA emissive;
  GdkRGBA specular;
  float shininess;
} GthreePhongMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreePhongMaterial, gthree_phong_material, GTHREE_TYPE_BASIC_MATERIAL);

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

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;

  priv->specular.red = 0.07;
  priv->specular.green = 0.07;
  priv->specular.blue = 0.07;
  priv->specular.alpha = 1.0;

  priv->shininess = 30;
}

static void
gthree_phong_material_finalize (GObject *obj)
{
  //GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (obj);
  //GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

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
  GTHREE_MATERIAL_CLASS (gthree_phong_material_parent_class)->set_params (material, params);

  // This is only supported in phong
  params->flat_shading = gthree_basic_material_get_shading_type (GTHREE_BASIC_MATERIAL (material)) == GTHREE_SHADING_FLAT;
}

static void
gthree_phong_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms,
                                         GthreeCamera   *camera)
{
  GthreePhongMaterial *phong = GTHREE_PHONG_MATERIAL (material);
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_phong_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "shininess");
  if (uni != NULL)
    gthree_uniform_set_float (uni, MAX (priv->shininess, 1e-4 )); // to prevent pow( 0.0, 0.0 )

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->emissive);

  uni = gthree_uniforms_lookup_from_string (uniforms, "specular");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->specular);
}

static gboolean
gthree_phong_material_needs_camera_pos (GthreeMaterial *material)
{
  return TRUE;
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

static void
gthree_phong_material_class_init (GthreePhongMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_phong_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_phong_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_phong_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_phong_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_camera_pos = gthree_phong_material_needs_camera_pos;
  GTHREE_MATERIAL_CLASS(klass)->needs_view_matrix = gthree_phong_material_needs_view_matrix;
  GTHREE_MATERIAL_CLASS(klass)->needs_lights = gthree_phong_material_needs_lights;
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

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);
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

  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);
}

float
gthree_phong_material_get_shininess (GthreePhongMaterial *phong)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  return priv->shininess;
}

void
gthree_phong_material_set_shininess (GthreePhongMaterial *phong,
                                     float                shininess)
{
  GthreePhongMaterialPrivate *priv = gthree_phong_material_get_instance_private (phong);

  priv->shininess = shininess;
  gthree_material_set_needs_update (GTHREE_MATERIAL (phong), TRUE);
}
