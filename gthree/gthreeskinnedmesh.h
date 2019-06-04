#ifndef __GTHREE_SKINNED_SKINNED_MESH_H__
#define __GTHREE_SKINNED_SKINNED_MESH_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeskeleton.h>
#include <gthree/gthreemesh.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_SKINNED_MESH      (gthree_skinned_mesh_get_type ())
#define GTHREE_SKINNED_MESH(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                   GTHREE_TYPE_SKINNED_MESH, \
                                                                   GthreeSkinnedMesh))
#define GTHREE_IS_SKINNED_MESH(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                   GTHREE_TYPE_SKINNED_MESH))

typedef struct {
  GthreeMesh parent;
} GthreeSkinnedMesh;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSkinnedMesh, g_object_unref)

typedef struct {
  GthreeMeshClass parent_class;

} GthreeSkinnedMeshClass;

GType gthree_skinned_mesh_get_type (void) G_GNUC_CONST;

GthreeSkinnedMesh *gthree_skinned_mesh_new (GthreeGeometry *geometry,
                                            GthreeMaterial *material);

GthreeSkeleton *         gthree_skinned_mesh_get_skeleton            (GthreeSkinnedMesh       *mesh);
const graphene_matrix_t *gthree_skinned_mesh_get_bind_matrix         (GthreeSkinnedMesh       *mesh);
const graphene_matrix_t *gthree_skinned_mesh_get_inverse_bind_matrix (GthreeSkinnedMesh       *mesh);
void                     gthree_skinned_mesh_set_bind_mode           (GthreeSkinnedMesh       *mesh,
                                                                      GthreeBindMode           bind_mode);
void                     gthree_skinned_mesh_normalize_skin_weights  (GthreeSkinnedMesh       *mesh);
void                     gthree_skinned_mesh_bind                    (GthreeSkinnedMesh       *mesh,
                                                                      GthreeSkeleton          *skeleton,
                                                                      const graphene_matrix_t *bind_matrix);
void                     gthree_skinned_mesh_pose                    (GthreeSkinnedMesh       *mesh);



G_END_DECLS

#endif /* __GTHREE_SKINNED_SKINNED_MESH_H__ */
