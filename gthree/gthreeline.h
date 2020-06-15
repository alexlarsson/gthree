#ifndef __GTHREE_LINE_H__
#define __GTHREE_LINE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreegeometry.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_LINE      (gthree_line_get_type ())
#define GTHREE_LINE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                           GTHREE_TYPE_LINE, \
                                                           GthreeLine))
#define GTHREE_IS_LINE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                           GTHREE_TYPE_LINE))

typedef struct {
  GthreeObject parent;
} GthreeLine;

typedef struct {
  GthreeObjectClass parent_class;

} GthreeLineClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeLine, g_object_unref)

GTHREE_API
GType gthree_line_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeLine *gthree_line_new (GthreeGeometry *geometry,
                             GthreeMaterial *material);

G_END_DECLS

#endif /* __GTHREE_LINE_H__ */
