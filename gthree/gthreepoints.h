#ifndef __GTHREE_POINTS_H__
#define __GTHREE_POINTS_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreegeometry.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_POINTS      (gthree_points_get_type ())
#define GTHREE_POINTS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_POINTS, \
                                                             GthreePoints))
#define GTHREE_IS_POINTS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_POINTS))

typedef struct {
  GthreeObject parent;
} GthreePoints;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePoints, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

} GthreePointsClass;

GTHREE_API
GType gthree_points_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreePoints *gthree_points_new (GthreeGeometry *geometry,
                                 GthreeMaterial *material);

GTHREE_API
GthreeMaterial *gthree_points_get_material (GthreePoints          *points);
GTHREE_API
void            gthree_points_set_material (GthreePoints          *points,
                                            GthreeMaterial        *material);
GTHREE_API
GthreeGeometry *gthree_points_get_geometry (GthreePoints     *points);


G_END_DECLS

#endif /* __GTHREE_POINTS_H__ */
