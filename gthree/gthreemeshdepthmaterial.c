#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshdepthmaterial.h"
#include "gthreecamera.h"

typedef struct {
  int dummy;
} GthreeMeshDepthMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshDepthMaterial, gthree_mesh_depth_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshDepthMaterial *
gthree_mesh_depth_material_new ()
{
  GthreeMeshDepthMaterial *material;

  material = g_object_new (gthree_mesh_depth_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_depth_material_init (GthreeMeshDepthMaterial *depth)
{
}

static void
gthree_mesh_depth_material_finalize (GObject *obj)
{

  G_OBJECT_CLASS (gthree_mesh_depth_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_depth_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("depth");
}

static void
gthree_mesh_depth_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{

  params->depth_packing = TRUE;

  GTHREE_MATERIAL_CLASS (gthree_mesh_depth_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_depth_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms,
                                         GthreeCamera *camera)
{
  GTHREE_MATERIAL_CLASS (gthree_mesh_depth_material_parent_class)->set_uniforms (material, uniforms, camera);

}

static void
gthree_mesh_depth_material_class_init (GthreeMeshDepthMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_depth_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_mesh_depth_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_mesh_depth_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_mesh_depth_material_real_set_uniforms;
}
