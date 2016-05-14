#ifndef __GTHREE_GEOMETRY_H__
#define __GTHREE_GEOMETRY_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <graphene.h>

#include <gdk/gdk.h>
#include <gthree/gthreeobject.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_GEOMETRY      (gthree_geometry_get_type ())
#define GTHREE_GEOMETRY(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                               GTHREE_TYPE_GEOMETRY, \
                                                               GthreeGeometry))
#define GTHREE_IS_GEOMETRY(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                               GTHREE_TYPE_GEOMETRY))

struct _GthreeGeometry {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;

} GthreeGeometryClass;

GthreeGeometry *gthree_geometry_new ();
GType gthree_geometry_get_type (void) G_GNUC_CONST;

void                   gthree_geometry_add_vertex     (GthreeGeometry  *geometry,
						       graphene_vec3_t *v);
guint                  gthree_geometry_get_n_vertices (GthreeGeometry  *geometry);
const graphene_vec3_t *gthree_geometry_get_vertices   (GthreeGeometry  *geometry);
int                    gthree_geometry_add_face       (GthreeGeometry  *geometry,
						       int              a,
						       int              b,
						       int              c);
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
guint                  gthree_geometry_get_n_colors   (GthreeGeometry  *geometry);

const graphene_sphere_t *gthree_geometry_get_bounding_sphere  (GthreeGeometry          *geometry);
void                     gthree_geometry_set_bounding_sphere  (GthreeGeometry          *geometry,
                                                               const graphene_sphere_t *sphere);
void                     gthree_geometry_compute_face_normals (GthreeGeometry          *geometry);
void                     gthree_geometry_compute_vertex_normals (GthreeGeometry *geometry,
                                                                 gboolean area_weighted);


int                    gthree_geometry_face_get_a              (GthreeGeometry         *geometry,
								int                     index);
int                    gthree_geometry_face_get_b              (GthreeGeometry         *geometry,
								int                     index);
int                    gthree_geometry_face_get_c              (GthreeGeometry         *geometry,
								int                     index);
void                   gthree_geometry_face_set_normal         (GthreeGeometry         *geometry,
								int                     index,
								const graphene_vec3_t  *normal);
const graphene_vec3_t  *gthree_geometry_face_get_normal        (GthreeGeometry         *geometry,
								int                     index);
void                   gthree_geometry_face_set_vertex_normals (GthreeGeometry         *geometry,
								int                     index,
								const graphene_vec3_t  *normal_a,
								const graphene_vec3_t  *normal_b,
								const graphene_vec3_t  *normal_c);
gboolean               gthree_geometry_face_get_vertex_normals (GthreeGeometry         *geometry,
								int                     index,
								const graphene_vec3_t **normal_a,
								const graphene_vec3_t **normal_b,
								const graphene_vec3_t **normal_c);
void                   gthree_geometry_face_set_color          (GthreeGeometry         *geometry,
								int                     index,
								const GdkRGBA          *color);
const GdkRGBA         *gthree_geometry_face_get_color          (GthreeGeometry         *geometry,
								int                     index);
void                   gthree_geometry_face_set_vertex_colors  (GthreeGeometry         *geometry,
								int                     index,
								const GdkRGBA          *a,
								const GdkRGBA          *b,
								const GdkRGBA          *c);
gboolean               gthree_geometry_face_get_vertex_colors  (GthreeGeometry         *geometry,
								int                     index,
								const GdkRGBA         **a,
								const GdkRGBA         **b,
								const GdkRGBA         **c);
void                   gthree_geometry_face_set_material_index (GthreeGeometry         *geometry,
								int                     index,
								int                     material_index);
int                    gthree_geometry_face_get_material_index (GthreeGeometry         *geometry,
								int                     index);

void     gthree_geometry_set_is_lines (GthreeGeometry *geometry,
                                       gboolean        lines);

gboolean gthree_geometry_get_is_lines (GthreeGeometry *geometry);

G_END_DECLS

#endif /* __GTHREE_GEOMETRY_H__ */
