#ifndef __GTHREE_LINE_SEGMENTS_H__
#define __GTHREE_LINE_SEGMENTS_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreegeometry.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_LINE_SEGMENTS      (gthree_line_segments_get_type ())
#define GTHREE_LINE_SEGMENTS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_LINE_SEGMENTS, \
                                                             GthreeLineSegments))
#define GTHREE_IS_LINE_SEGMENTS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_LINE_SEGMENTS))

typedef struct {
  GthreeObject parent;
} GthreeLineSegments;

typedef struct {
  GthreeObjectClass parent_class;

} GthreeLineSegmentsClass;

GTHREE_API
GType gthree_line_segments_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeLineSegments *gthree_line_segments_new (GthreeGeometry *geometry,
                                              GthreeMaterial *material);

G_END_DECLS

#endif /* __GTHREE_LINE_SEGMENTS_H__ */
