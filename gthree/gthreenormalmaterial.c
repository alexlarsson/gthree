#include <math.h>
#include <epoxy/gl.h>

#include "gthreenormalmaterial.h"

typedef struct {
  GthreeShadingType shading_type;
} GthreeNormalMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeNormalMaterial, gthree_normal_material, GTHREE_TYPE_MATERIAL);

GthreeNormalMaterial *
gthree_normal_material_new ()
{
  GthreeNormalMaterial *material;

  material = g_object_new (gthree_normal_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_normal_material_init (GthreeNormalMaterial *normal)
{
  GthreeNormalMaterialPrivate *priv = gthree_normal_material_get_instance_private (normal);

  priv->shading_type = GTHREE_SHADING_FLAT;
}

static void
gthree_normal_material_finalize (GObject *obj)
{

  G_OBJECT_CLASS (gthree_normal_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_normal_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("normal");
}

static void
gthree_normal_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GTHREE_MATERIAL_CLASS (gthree_normal_material_parent_class)->set_params (material, params);
}

static void
gthree_normal_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms)
{

  GTHREE_MATERIAL_CLASS (gthree_normal_material_parent_class)->set_uniforms (material, uniforms);
}

static GthreeShadingType
gthree_normal_material_needs_normals (GthreeMaterial *material)
{
  GthreeNormalMaterial *normal = GTHREE_NORMAL_MATERIAL (material);
  GthreeNormalMaterialPrivate *priv = gthree_normal_material_get_instance_private (normal);

  return priv->shading_type;
}

static void
gthree_normal_material_class_init (GthreeNormalMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_normal_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_normal_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_normal_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_normal_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_normals = gthree_normal_material_needs_normals;
}

GthreeShadingType
gthree_normal_material_get_shading_type (GthreeNormalMaterial *normal)
{
  GthreeNormalMaterialPrivate *priv = gthree_normal_material_get_instance_private (normal);

  return priv->shading_type;
}

void
gthree_normal_material_set_shading_type (GthreeNormalMaterial *normal,
                                        GthreeShadingType    shading_type)
{
  GthreeNormalMaterialPrivate *priv = gthree_normal_material_get_instance_private (normal);

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (normal), TRUE);
}
