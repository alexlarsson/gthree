#ifndef __GTHREE_DISTANCE_MATERIAL_H__
#define __GTHREE_DISTANCE_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshmaterial.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_MESH_DISTANCE_MATERIAL      (gthree_mesh_distance_material_get_type ())
#define GTHREE_MESH_DISTANCE_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_MESH_DISTANCE_MATERIAL, \
                                                                     GthreeMeshDistanceMaterial))
#define GTHREE_IS_MESH_DISTANCE_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_MESH_DISTANCE_MATERIAL))

struct _GthreeMeshDistanceMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshDistanceMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshDistanceMaterial, g_object_unref)

GTHREE_API
GthreeMeshDistanceMaterial *gthree_mesh_distance_material_new ();
GTHREE_API
GType gthree_mesh_distance_material_get_type (void) G_GNUC_CONST;

GTHREE_API
const graphene_vec3_t *gthree_mesh_distance_material_get_reference_point (GthreeMeshDistanceMaterial  *distance);
GTHREE_API
void                   gthree_mesh_distance_material_set_reference_point (GthreeMeshDistanceMaterial  *distance,
                                                                          const graphene_vec3_t       *ref_point);
GTHREE_API
float                  gthree_mesh_distance_material_get_near_distance   (GthreeMeshDistanceMaterial  *distance);
GTHREE_API
void                   gthree_mesh_distance_material_set_near_distance   (GthreeMeshDistanceMaterial  *distance,
                                                                          float                        near_distance);
GTHREE_API
float                  gthree_mesh_distance_material_get_far_distance    (GthreeMeshDistanceMaterial  *distance);
GTHREE_API
void                   gthree_mesh_distance_material_set_far_distance    (GthreeMeshDistanceMaterial  *distance,
                                                                          float                        far_distance);

G_END_DECLS

#endif /* __GTHREE_DISTANCEMATERIAL_H__ */
