#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshnormalmaterial.h"

typedef struct {
  GthreeShadingType shading_type;
} GthreeMeshNormalMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshNormalMaterial, gthree_mesh_normal_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshNormalMaterial *
gthree_mesh_normal_material_new ()
{
  GthreeMeshNormalMaterial *material;

  material = g_object_new (gthree_mesh_normal_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_normal_material_init (GthreeMeshNormalMaterial *normal)
{
  GthreeMeshNormalMaterialPrivate *priv = gthree_mesh_normal_material_get_instance_private (normal);

  priv->shading_type = GTHREE_SHADING_FLAT;
}

static void
gthree_mesh_normal_material_finalize (GObject *obj)
{

  G_OBJECT_CLASS (gthree_mesh_normal_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_normal_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("normal");
}

static void
gthree_mesh_normal_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GTHREE_MATERIAL_CLASS (gthree_mesh_normal_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_normal_material_real_set_uniforms (GthreeMaterial *material,
                                               GthreeUniforms *uniforms,
                                               GthreeCamera   *camera,
                                               GthreeRenderer *renderer)
{

  GTHREE_MATERIAL_CLASS (gthree_mesh_normal_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);
}

static void
gthree_mesh_normal_material_class_init (GthreeMeshNormalMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_normal_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_mesh_normal_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_mesh_normal_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_mesh_normal_material_real_set_uniforms;
}

GthreeShadingType
gthree_mesh_normal_material_get_shading_type (GthreeMeshNormalMaterial *normal)
{
  GthreeMeshNormalMaterialPrivate *priv = gthree_mesh_normal_material_get_instance_private (normal);

  return priv->shading_type;
}

void
gthree_mesh_normal_material_set_shading_type (GthreeMeshNormalMaterial *normal,
                                        GthreeShadingType    shading_type)
{
  GthreeMeshNormalMaterialPrivate *priv = gthree_mesh_normal_material_get_instance_private (normal);

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (normal), TRUE);
}
