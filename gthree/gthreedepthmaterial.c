#include <math.h>
#include <epoxy/gl.h>

#include "gthreedepthmaterial.h"
#include "gthreecamera.h"

typedef struct {
  int dummy;
} GthreeDepthMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeDepthMaterial, gthree_depth_material, GTHREE_TYPE_MATERIAL);

GthreeDepthMaterial *
gthree_depth_material_new ()
{
  GthreeDepthMaterial *material;

  material = g_object_new (gthree_depth_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_depth_material_init (GthreeDepthMaterial *depth)
{
}

static void
gthree_depth_material_finalize (GObject *obj)
{

  G_OBJECT_CLASS (gthree_depth_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_depth_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("depth");
}

static void
gthree_depth_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GTHREE_MATERIAL_CLASS (gthree_depth_material_parent_class)->set_params (material, params);
}

static void
gthree_depth_material_real_set_uniforms (GthreeMaterial *material,
                                         GthreeUniforms *uniforms,
                                         GthreeCamera *camera)
{
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_depth_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "mNear");
  if (uni != NULL)
    gthree_uniform_set_float (uni, gthree_camera_get_near (camera));

  uni = gthree_uniforms_lookup_from_string (uniforms, "mFar");
  if (uni != NULL)
    gthree_uniform_set_float (uni, gthree_camera_get_far (camera));
}

static void
gthree_depth_material_class_init (GthreeDepthMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_depth_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_depth_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_depth_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_depth_material_real_set_uniforms;
}
