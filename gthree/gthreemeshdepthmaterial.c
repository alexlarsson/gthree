#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshdepthmaterial.h"
#include "gthreecamera.h"
#include "gthreeprivate.h"

typedef struct {
  int format;
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
  GthreeMeshDepthMaterialPrivate *priv = gthree_mesh_depth_material_get_instance_private (depth);
  priv->format = GTHREE_DEPTH_PACKING_FORMAT_BASIC;
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
  GthreeMeshDepthMaterial *depth = GTHREE_MESH_DEPTH_MATERIAL (material);
  GthreeMeshDepthMaterialPrivate *priv = gthree_mesh_depth_material_get_instance_private (depth);

  params->depth_packing = 1 + priv->format;

  GTHREE_MATERIAL_CLASS (gthree_mesh_depth_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_depth_material_real_set_uniforms (GthreeMaterial *material,
                                              GthreeUniforms *uniforms,
                                              GthreeCamera *camera,
                                              GthreeRenderer *renderer)
{
  GTHREE_MATERIAL_CLASS (gthree_mesh_depth_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

}

GthreeDepthPackingFormat
gthree_mesh_depth_material_get_depth_packing_format (GthreeMeshDepthMaterial  *depth)
{
  GthreeMeshDepthMaterialPrivate *priv = gthree_mesh_depth_material_get_instance_private (depth);

  return priv->format;
}

void
gthree_mesh_depth_material_set_depth_packing_format (GthreeMeshDepthMaterial  *depth,
                                                     GthreeDepthPackingFormat  format)
{
  GthreeMeshDepthMaterialPrivate *priv = gthree_mesh_depth_material_get_instance_private (depth);

  priv->format = format;
}


static void
gthree_mesh_depth_material_class_init (GthreeMeshDepthMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_depth_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_mesh_depth_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_mesh_depth_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_mesh_depth_material_real_set_uniforms;
}
