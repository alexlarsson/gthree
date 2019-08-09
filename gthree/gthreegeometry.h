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

  // Used internally by renderer
  GArray *influences;
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

GTHREE_API
GthreeGeometry *gthree_geometry_new ();

GTHREE_API
GthreeGeometry *gthree_geometry_parse_json (JsonObject *object);

GTHREE_API
GType gthree_geometry_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeAttribute *        gthree_geometry_add_attribute              (GthreeGeometry          *geometry,
                                                                     const char              *name,
                                                                     GthreeAttribute         *attribute);
GTHREE_API
void                     gthree_geometry_remove_attribute           (GthreeGeometry          *geometry,
                                                                     const char              *name);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_attribute              (GthreeGeometry          *geometry,
                                                                     const char              *name);
GTHREE_API
gboolean                 gthree_geometry_has_attribute              (GthreeGeometry          *geometry,
                                                                     const char              *name);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_position               (GthreeGeometry          *geometry);
GTHREE_API
int                      gthree_geometry_get_position_count         (GthreeGeometry          *geometry);
GTHREE_API
int                      gthree_geometry_get_vertex_count           (GthreeGeometry          *geometry);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_normal                 (GthreeGeometry          *geometry);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_color                  (GthreeGeometry          *geometry);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_uv                     (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_set_index                  (GthreeGeometry          *geometry,
                                                                     GthreeAttribute         *index);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_index                  (GthreeGeometry          *geometry);
GTHREE_API
GthreeAttribute *        gthree_geometry_get_wireframe_index        (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_add_morph_attribute        (GthreeGeometry          *geometry,
                                                                    const char               *name,
                                                                    GthreeAttribute          *attribute);
GTHREE_API
void                     gthree_geometry_remove_morph_attributes    (GthreeGeometry          *geometry,
                                                                     const char              *name);
GTHREE_API
GPtrArray *              gthree_geometry_get_morph_attributes       (GthreeGeometry          *geometry,
                                                                     const char              *name);
GTHREE_API
GList *                  gthree_geometry_get_morph_attributes_names (GthreeGeometry          *geometry);
GTHREE_API
gboolean                 gthree_geometry_has_morph_attributes       (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_add_group                  (GthreeGeometry          *geometry,
                                                                     int                      start,
                                                                     int                      count,
                                                                     int                      material_index);
GTHREE_API
void                     gthree_geometry_clear_groups               (GthreeGeometry          *geometry);
GTHREE_API
int                      gthree_geometry_get_n_groups               (GthreeGeometry          *geometry);
GTHREE_API
GthreeGeometryGroup *    gthree_geometry_get_group                  (GthreeGeometry          *geometry,
                                                                     int                      index);
GTHREE_API
GthreeGeometryGroup *    gthree_geometry_peek_groups                (GthreeGeometry          *geometry);
GTHREE_API
int                      gthree_geometry_get_draw_range_start       (GthreeGeometry          *geometry);
GTHREE_API
int                      gthree_geometry_get_draw_range_count       (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_set_draw_range             (GthreeGeometry          *geometry,
                                                                     int                      start,
                                                                     int                      count);
GTHREE_API
void                     gthree_geometry_invalidate_bounds          (GthreeGeometry          *geometry);
GTHREE_API
const graphene_sphere_t *gthree_geometry_get_bounding_sphere        (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_set_bounding_sphere        (GthreeGeometry          *geometry,
                                                                     const graphene_sphere_t *sphere);
GTHREE_API
const graphene_box_t    *gthree_geometry_get_bounding_box           (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_set_bounding_box           (GthreeGeometry          *geometry,
                                                                     const graphene_box_t    *box);
void                     gthree_geometry_compute_vertex_normals     (GthreeGeometry          *geometry);
GTHREE_API
void                     gthree_geometry_normalize_normals          (GthreeGeometry          *geometry);


G_END_DECLS

#endif /* __GTHREE_GEOMETRY_H__ */
