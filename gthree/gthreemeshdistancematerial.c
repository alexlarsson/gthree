#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshdistancematerial.h"
#include "gthreecamera.h"
#include "gthreeprivate.h"

typedef struct {
  graphene_vec3_t reference_point;
  float nearDistance;
  float farDistance;

  // TODO: Support displacement map and alpha map

} GthreeMeshDistanceMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshDistanceMaterial, gthree_mesh_distance_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshDistanceMaterial *
gthree_mesh_distance_material_new ()
{
  GthreeMeshDistanceMaterial *material;

  material = g_object_new (gthree_mesh_distance_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_distance_material_init (GthreeMeshDistanceMaterial *distance)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  graphene_vec3_init (&priv->reference_point, 0, 0, 0);
  priv->nearDistance = 1;
  priv->farDistance = 1000;
}

static void
gthree_mesh_distance_material_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_mesh_distance_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_distance_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("distanceRGBA");
}

static void
gthree_mesh_distance_material_real_set_params (GthreeMaterial *material,
                                               GthreeProgramParameters *params)
{
  GTHREE_MATERIAL_CLASS (gthree_mesh_distance_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_distance_material_real_set_uniforms (GthreeMaterial *material,
                                                 GthreeUniforms *uniforms,
                                                 GthreeCamera *camera,
                                                 GthreeRenderer *renderer)
{
  GthreeMeshDistanceMaterial *distance = GTHREE_MESH_DISTANCE_MATERIAL (material);
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  gthree_uniforms_set_float (uniforms, "nearDistance", priv->nearDistance);
  gthree_uniforms_set_float (uniforms, "farDistance", priv->farDistance);
  gthree_uniforms_set_vec3 (uniforms, "referencePosition", &priv->reference_point);

  GTHREE_MATERIAL_CLASS (gthree_mesh_distance_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

}

const graphene_vec3_t *
gthree_mesh_distance_material_get_reference_point (GthreeMeshDistanceMaterial  *distance)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  return &priv->reference_point;
}

void
gthree_mesh_distance_material_set_reference_point (GthreeMeshDistanceMaterial  *distance,
                                                   const graphene_vec3_t       *ref_point)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  priv->reference_point = *ref_point;
}

float
gthree_mesh_distance_material_get_near_distance   (GthreeMeshDistanceMaterial  *distance)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  return priv->nearDistance;
}

void
gthree_mesh_distance_material_set_near_distance   (GthreeMeshDistanceMaterial  *distance,
                                                   float                        near_distance)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  priv->nearDistance = near_distance;
}

float
gthree_mesh_distance_material_get_far_distance   (GthreeMeshDistanceMaterial  *distance)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  return priv->farDistance;
}

void
gthree_mesh_distance_material_set_far_distance   (GthreeMeshDistanceMaterial  *distance,
                                                   float                        far_distance)
{
  GthreeMeshDistanceMaterialPrivate *priv = gthree_mesh_distance_material_get_instance_private (distance);

  priv->farDistance = far_distance;
}

static void
gthree_mesh_distance_material_class_init (GthreeMeshDistanceMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_distance_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_mesh_distance_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_mesh_distance_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_mesh_distance_material_real_set_uniforms;
}
