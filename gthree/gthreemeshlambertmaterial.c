#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshlambertmaterial.h"

typedef struct {
  GdkRGBA emissive;

} GthreeMeshLambertMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshLambertMaterial, gthree_mesh_lambert_material, GTHREE_TYPE_BASIC_MATERIAL);

GthreeMeshLambertMaterial *
gthree_mesh_lambert_material_new ()
{
  GthreeMeshLambertMaterial *material;

  material = g_object_new (gthree_mesh_lambert_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_lambert_material_init (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;
}

static void
gthree_mesh_lambert_material_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_mesh_lambert_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_lambert_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("lambert");
}

static void
gthree_mesh_lambert_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GTHREE_MATERIAL_CLASS (gthree_mesh_lambert_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_lambert_material_real_set_uniforms (GthreeMaterial *material,
                                           GthreeUniforms *uniforms,
                                           GthreeCamera   *camera)
{
  GthreeMeshLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (material);
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_lambert_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->emissive);
}

static gboolean
gthree_mesh_lambert_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_lambert_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}

static void
gthree_mesh_lambert_material_class_init (GthreeMeshLambertMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_lambert_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_mesh_lambert_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_mesh_lambert_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_mesh_lambert_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_view_matrix = gthree_mesh_lambert_material_needs_view_matrix;
  GTHREE_MATERIAL_CLASS(klass)->needs_lights = gthree_mesh_lambert_material_needs_lights;
}

const GdkRGBA *
gthree_mesh_lambert_material_get_emissive_color (GthreeMeshLambertMaterial *lambert)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  return &priv->emissive;
}

void
gthree_mesh_lambert_material_set_emissive_color (GthreeMeshLambertMaterial *lambert,
                                            const GdkRGBA *color)
{
  GthreeMeshLambertMaterialPrivate *priv = gthree_mesh_lambert_material_get_instance_private (lambert);

  priv->emissive = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);
}
