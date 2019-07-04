#ifndef __GTHREE_MESH_H__
#define __GTHREE_MESH_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreegeometry.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_MESH      (gthree_mesh_get_type ())
#define GTHREE_MESH(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_MESH, \
                                                             GthreeMesh))
#define GTHREE_IS_MESH(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_MESH))

typedef struct {
  GthreeObject parent;
} GthreeMesh;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMesh, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

} GthreeMeshClass;

GType gthree_mesh_get_type (void) G_GNUC_CONST;

GthreeMesh *gthree_mesh_new (GthreeGeometry *geometry,
                             GthreeMaterial *material);

GthreeMaterial *gthree_mesh_get_material         (GthreeMesh     *mesh);
void            gthree_mesh_set_material         (GthreeMesh     *mesh,
                                                  GthreeMaterial *material);
GthreeGeometry *gthree_mesh_get_geometry         (GthreeMesh     *mesh);
GthreeDrawMode  gthree_mesh_get_draw_mode        (GthreeMesh     *mesh);
void            gthree_mesh_set_draw_mode        (GthreeMesh     *mesh,
                                                  GthreeDrawMode  mode);
void            gthree_mesh_update_morph_targets (GthreeMesh     *mesh);
gboolean        gthree_mesh_has_morph_targets    (GthreeMesh     *mesh);
GArray *        gthree_mesh_get_morph_targets    (GthreeMesh     *mesh);
void            gthree_mesh_set_morph_targets    (GthreeMesh     *mesh,
                                                  GArray *        morph_targets);

G_END_DECLS

#endif /* __GTHREE_MESH_H__ */
