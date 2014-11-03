#ifndef __GTHREE_AREA_H__
#define __GTHREE_AREA_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gtk/gtk.h>

#include <gthree/gthreescene.h>
#include <gthree/gthreecamera.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_AREA      (gthree_area_get_type ())
#define GTHREE_AREA(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                            GTHREE_TYPE_AREA, \
                                                            GthreeArea))
#define GTHREE_IS_AREA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                            GTHREE_TYPE_AREA))

typedef struct {
  GtkGLArea parent;
} GthreeArea;

typedef struct {
  GtkGLAreaClass parent_class;
} GthreeAreaClass;

GtkWidget *gthree_area_new (GthreeScene *scene,
                            GthreeCamera *camera);
GType gthree_area_get_type (void) G_GNUC_CONST;

GthreeRenderer *gthree_area_get_renderer (GthreeArea *area);

G_END_DECLS

#endif /* __GTHREE_AREA_H__ */
