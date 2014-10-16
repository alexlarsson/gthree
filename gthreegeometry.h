#ifndef __GTHREE_GEOMETRY_H__
#define __GTHREE_GEOMETRY_H__

#include <gtk/gtk.h>
#include <graphene.h>

#include "gthreeobject.h"
#include "gthreeface.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_GEOMETRY      (gthree_geometry_get_type ())
#define GTHREE_GEOMETRY(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                               GTHREE_TYPE_GEOMETRY, \
                                                               GthreeGeometry))
#define GTHREE_IS_GEOMETRY(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                               GTHREE_TYPE_GEOMETRY))

typedef struct {
  GObject parent;
} GthreeGeometry;

typedef struct {
  GObjectClass parent_class;

} GthreeGeometryClass;

GthreeGeometry *gthree_geometry_new ();
GType gthree_geometry_get_type (void) G_GNUC_CONST;

void gthree_geometry_add_vertex (GthreeGeometry *geometry,
                                 graphene_vec3_t *v);
guint gthree_geometry_get_n_vertices (GthreeGeometry *geometry);
void gthree_geometry_add_face (GthreeGeometry *geometry,
                               GthreeFace *face);
GthreeFace * gthree_geometry_get_face (GthreeGeometry *geometry, int i);
guint gthree_geometry_get_n_faces (GthreeGeometry *geometry);

gboolean gthree_geometry_make_groups (GthreeGeometry *geometry,
                                      gboolean use_face_material,
                                      int max_vertices_in_group);


GthreeGeometry *
gthree_geometry_new_box (float width, float height, float depth,
                         int width_segments, int height_segments, int depth_segments);

G_END_DECLS

#endif /* __GTHREE_GEOMETRY_H__ */
