#include <math.h>
#include <epoxy/gl.h>

#include "gthreebasicmaterial.h"

typedef struct {
  int dummy;
  GdkRGBA color;

  float reflectivity;
  float refraction_ratio;

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
}

static void
gthree_basic_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms)
{
  GthreeBasicMaterial *basic = GTHREE_BASIC_MATERIAL (material);
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_basic_material_parent_class)->set_uniforms (material, uniforms);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);
}

static void
gthree_basic_material_class_init (GthreeBasicMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_basic_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_basic_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_basic_material_real_set_uniforms;
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

  basic->parent.needs_update = TRUE;
}

void
gthree_basic_material_set_vertex_colors (GthreeBasicMaterial *basic,
                                         GthreeColorType color_type)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  priv->vertex_colors = color_type;

  basic->parent.needs_update = TRUE;
}

GthreeColorType
gthree_basic_material_get_vertex_colors (GthreeBasicMaterial *basic)
{
  GthreeBasicMaterialPrivate *priv = gthree_basic_material_get_instance_private (basic);

  return priv->vertex_colors;
}
