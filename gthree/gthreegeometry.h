#ifndef __GTHREE_GEOMETRY_H__
#define __GTHREE_GEOMETRY_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <graphene.h>

#include <gdk/gdk.h>
#include <gthree/gthreeobject.h>
#include <json-glib/json-glib.h>

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

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeGeometry, g_object_unref)

typedef struct {
  GObjectClass parent_class;

} GthreeGeometryClass;

typedef struct {
  int start;
  int count;
  int material_index;
} GthreeGeometryGroup;

GthreeGeometry *gthree_geometry_new ();

GthreeGeometry *gthree_geometry_parse_json (JsonObject *object);

GType gthree_geometry_get_type (void) G_GNUC_CONST;

GthreeAttribute *        gthree_geometry_add_attribute          (GthreeGeometry          *geometry,
                                                                 GthreeAttribute         *attribute);
void                     gthree_geometry_remove_attribute       (GthreeGeometry          *geometry,
                                                                 GthreeAttributeName      name);
GthreeAttribute *        gthree_geometry_get_attribute          (GthreeGeometry          *geometry,
                                                                 GthreeAttributeName      name);
gboolean                 gthree_geometry_has_attribute          (GthreeGeometry          *geometry,
                                                                 GthreeAttributeName      name);
GthreeAttribute *        gthree_geometry_get_position           (GthreeGeometry          *geometry);
int                      gthree_geometry_get_position_count     (GthreeGeometry          *geometry);
int                      gthree_geometry_get_vertex_count       (GthreeGeometry          *geometry);
GthreeAttribute *        gthree_geometry_get_normal             (GthreeGeometry          *geometry);
GthreeAttribute *        gthree_geometry_get_color              (GthreeGeometry          *geometry);
GthreeAttribute *        gthree_geometry_get_uv                 (GthreeGeometry          *geometry);
void                     gthree_geometry_set_index              (GthreeGeometry          *geometry,
                                                                 GthreeAttribute         *index);
GthreeAttribute *        gthree_geometry_get_index              (GthreeGeometry          *geometry);
GthreeAttribute *        gthree_geometry_get_wireframe_index    (GthreeGeometry          *geometry);
void                     gthree_geometry_add_group              (GthreeGeometry          *geometry,
                                                                 int                      start,
                                                                 int                      count,
                                                                 int                      material_index);
void                     gthree_geometry_clear_groups           (GthreeGeometry          *geometry);
int                      gthree_geometry_get_n_groups           (GthreeGeometry          *geometry);
GthreeGeometryGroup *    gthree_geometry_get_group              (GthreeGeometry          *geometry,
                                                                 int                      index);
GthreeGeometryGroup *    gthree_geometry_peek_groups            (GthreeGeometry          *geometry);
int                      gthree_geometry_get_draw_range_start   (GthreeGeometry          *geometry);
int                      gthree_geometry_get_draw_range_count   (GthreeGeometry          *geometry);
void                     gthree_geometry_set_draw_range         (GthreeGeometry          *geometry,
                                                                 int                      start,
                                                                 int                      count);
const graphene_sphere_t *gthree_geometry_get_bounding_sphere    (GthreeGeometry          *geometry);
void                     gthree_geometry_set_bounding_sphere    (GthreeGeometry          *geometry,
                                                                 const graphene_sphere_t *sphere);
void                     gthree_geometry_compute_vertex_normals (GthreeGeometry          *geometry);
void                     gthree_geometry_normalize_normals      (GthreeGeometry          *geometry);

G_END_DECLS

#endif /* __GTHREE_GEOMETRY_H__ */
