#ifndef __GTHREE_INSTANCED_MESH_H__
#define __GTHREE_INSTANCED_MESH_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemesh.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_INSTANCED_MESH      (gthree_instanced_mesh_get_type ())
#define GTHREE_INSTANCED_MESH(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_INSTANCED_MESH, \
                                                                     GthreeInstancedMesh))
#define GTHREE_IS_INSTANCED_MESH(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_INSTANCED_MESH))

typedef struct {
  GthreeMesh parent;
} GthreeInstancedMesh;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeInstancedMesh, g_object_unref)

typedef struct {
  GthreeMeshClass parent_class;
} GthreeInstancedMeshClass;

GTHREE_API
GType gthree_instanced_mesh_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeInstancedMesh *gthree_instanced_mesh_new            (GthreeGeometry          *geometry,
                                                           GthreeMaterial          *material,
                                                           int                      count);
GTHREE_API
int                  gthree_instanced_mesh_get_count       (GthreeInstancedMesh     *mesh);
GTHREE_API
void                 gthree_instanced_mesh_set_count       (GthreeInstancedMesh     *mesh,
                                                            int                      count);
GTHREE_API
void                 gthree_instanced_mesh_set_matrix_at   (GthreeInstancedMesh     *mesh,
                                                            int                      index,
                                                            const graphene_matrix_t *matrix);
GTHREE_API
void                 gthree_instanced_mesh_get_matrix_at   (GthreeInstancedMesh     *mesh,
                                                            int                      index,
                                                            graphene_matrix_t       *matrix);
GTHREE_API
void                 gthree_instanced_mesh_set_color_at    (GthreeInstancedMesh     *mesh,
                                                            int                      index,
                                                            const graphene_vec3_t   *color);
GTHREE_API
void                 gthree_instanced_mesh_get_color_at    (GthreeInstancedMesh     *mesh,
                                                            int                      index,
                                                            graphene_vec3_t         *color);
GTHREE_API
GthreeAttribute *    gthree_instanced_mesh_get_instance_matrix (GthreeInstancedMesh *mesh);
GTHREE_API
GthreeAttribute *    gthree_instanced_mesh_get_instance_color  (GthreeInstancedMesh *mesh);
GTHREE_API
void                 gthree_instanced_mesh_set_morph_at        (GthreeInstancedMesh *mesh,
                                                                int                  index,
                                                                GthreeMesh          *source);

G_END_DECLS

#endif /* __GTHREE_INSTANCED_MESH_H__ */
