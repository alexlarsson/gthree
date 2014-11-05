#ifndef __GTHREE_GEOMETRY_GROUP_H__
#define __GTHREE_GEOMETRY_GROUP_H__

#include <gthree/gthreebufferprivate.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_GEOMETRY_GROUP      (gthree_geometry_group_get_type ())
#define GTHREE_GEOMETRY_GROUP(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_GEOMETRY_GROUP, \
                                                                     GthreeGeometryGroup))
#define GTHREE_IS_GEOMETRY_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_GEOMETRY_GROUP))

typedef struct {
  GthreeBuffer parent;

  GthreeGeometry *geometry;
  GArray *face_indexes; /* int */
  int n_vertices;

  float *vertex_array;
  float *normal_array;
  float *tangent_array;
  float *color_array;
  float *uv_array;
  float *uv2_array;

  guint16 *face_array;
  guint16 *line_array;

  guint vertices_need_update : 1;
  guint morph_targets_need_update : 1;
  guint elements_need_update : 1;
  guint uvs_need_update : 1;
  guint normals_need_update : 1;
  guint tangents_need_update : 1;
  guint colors_need_update : 1;

} GthreeGeometryGroup;

typedef struct {
  GthreeBufferClass parent_class;

} GthreeGeometryGroupClass;

GthreeGeometryGroup *gthree_geometry_group_new ();
GType gthree_geometry_group_get_type (void) G_GNUC_CONST;

void gthree_geometry_group_add_face (GthreeGeometryGroup *group,
                                     int face_index);
void gthree_geometry_group_dispose (GthreeGeometryGroup *group);

void gthree_geometry_group_realize (GthreeGeometryGroup *group,
                                    GthreeMaterial *material);
void gthree_geometry_group_update (GthreeGeometryGroup *group,
                                   GthreeMaterial *material,
                                   gboolean dispose);



G_END_DECLS

#endif /* __GTHREE_GEOMETRY_GROUP_H__ */
