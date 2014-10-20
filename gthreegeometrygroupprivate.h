#ifndef __GTHREE_GEOMETRY_GROUP_H__
#define __GTHREE_GEOMETRY_GROUP_H__

#include "gthreebufferprivate.h"
#include "gthreeface.h"

G_BEGIN_DECLS

#define GTHREE_TYPE_GEOMETRY_GROUP      (gthree_geometry_group_get_type ())
#define GTHREE_GEOMETRY_GROUP(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_GEOMETRY_GROUP, \
                                                                     GthreeGeometryGroup))
#define GTHREE_IS_GEOMETRY_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_GEOMETRY_GROUP))

typedef struct {
  GthreeBuffer parent;

  GPtrArray *faces; /* GthreeFace* */
  int n_vertices;

  guint realized : 1;

  float *vertex_array;
  float *normal_array;
  float *tangent_array;
  float *color_array;
  float *uv_array;
  float *uv2_array;

  guint16 *face_array;
  guint16 *line_array;

} GthreeGeometryGroup;

typedef struct {
  GthreeBufferClass parent_class;

} GthreeGeometryGroupClass;

GthreeGeometryGroup *gthree_geometry_group_new (GthreeObject *object,
                                                GthreeMaterial *material);
GType gthree_geometry_group_get_type (void) G_GNUC_CONST;

void gthree_geometry_group_add_face (GthreeGeometryGroup *group,
                                     GthreeFace *face);
void gthree_geometry_group_dispose (GthreeGeometryGroup *group);

G_END_DECLS

#endif /* __GTHREE_GEOMETRY_GROUP_H__ */
