#ifndef __GTHREE_GEOMETRY_H__
#define __GTHREE_GEOMETRY_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

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

void                   gthree_geometry_add_vertex     (GthreeGeometry  *geometry,
                                                       graphene_vec3_t *v);
guint                  gthree_geometry_get_n_vertices (GthreeGeometry  *geometry);
const graphene_vec3_t *gthree_geometry_get_vertices   (GthreeGeometry  *geometry);
void                   gthree_geometry_add_face       (GthreeGeometry  *geometry,
                                                       GthreeFace      *face);
GthreeFace *           gthree_geometry_get_face       (GthreeGeometry  *geometry,
                                                       int              i);
guint                  gthree_geometry_get_n_faces    (GthreeGeometry  *geometry);
const graphene_vec2_t *gthree_geometry_get_uvs        (GthreeGeometry  *geometry);
guint                  gthree_geometry_get_n_uv       (GthreeGeometry  *geometry);
void                   gthree_geometry_add_uv         (GthreeGeometry  *geometry,
                                                       graphene_vec2_t *v);
const graphene_vec2_t *gthree_geometry_get_uv2s       (GthreeGeometry  *geometry);
guint                  gthree_geometry_get_n_uv2      (GthreeGeometry  *geometry);
void                   gthree_geometry_add_uv2        (GthreeGeometry  *geometry,
                                                       graphene_vec2_t *v);
void                   gthree_geometry_set_uv_n       (GthreeGeometry  *geometry,
                                                       int              layer,
                                                       int              index,
                                                       graphene_vec2_t *v);

const GthreeSphere *gthree_geometry_get_bounding_sphere  (GthreeGeometry *geometry);
void                gthree_geometry_compute_face_normals (GthreeGeometry *geometry);

gboolean gthree_geometry_make_groups (GthreeGeometry *geometry,
                                      gboolean use_face_material,
                                      int max_vertices_in_group);


GthreeGeometry *
gthree_geometry_new_box (float width, float height, float depth,
                         int width_segments, int height_segments, int depth_segments);

G_END_DECLS

#endif /* __GTHREE_GEOMETRY_H__ */
