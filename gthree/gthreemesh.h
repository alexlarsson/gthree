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

GTHREE_API
GType gthree_mesh_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeMesh *gthree_mesh_new (GthreeGeometry *geometry,
                             GthreeMaterial *material);

GTHREE_API
GthreeMaterial *gthree_mesh_get_material         (GthreeMesh     *mesh,
                                                  int             index);
GTHREE_API
int             gthree_mesh_get_n_materials      (GthreeMesh     *mesh);
GTHREE_API
void            gthree_mesh_set_materials        (GthreeMesh     *mesh,
                                                  GPtrArray      *materials);
GTHREE_API
void            gthree_mesh_set_material         (GthreeMesh     *mesh,
                                                  int             index,
                                                  GthreeMaterial *material);
GTHREE_API
void            gthree_mesh_add_material         (GthreeMesh     *mesh,
                                                  GthreeMaterial *material);
GTHREE_API
GthreeGeometry *gthree_mesh_get_geometry         (GthreeMesh     *mesh);
GTHREE_API
GthreeDrawMode  gthree_mesh_get_draw_mode        (GthreeMesh     *mesh);
GTHREE_API
void            gthree_mesh_set_draw_mode        (GthreeMesh     *mesh,
                                                  GthreeDrawMode  mode);
GTHREE_API
void            gthree_mesh_update_morph_targets (GthreeMesh     *mesh);
GTHREE_API
gboolean        gthree_mesh_has_morph_targets    (GthreeMesh     *mesh);
GTHREE_API
GArray *        gthree_mesh_get_morph_targets    (GthreeMesh     *mesh);
GTHREE_API
void            gthree_mesh_set_morph_targets    (GthreeMesh     *mesh,
                                                  GArray         *morph_targets);


G_END_DECLS

#endif /* __GTHREE_MESH_H__ */
