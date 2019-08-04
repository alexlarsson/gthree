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

GTHREE_API
GType gthree_area_get_type (void) G_GNUC_CONST;

GTHREE_API
GtkWidget *gthree_area_new (GthreeScene *scene,
                            GthreeCamera *camera);

GTHREE_API
GthreeScene *   gthree_area_get_scene    (GthreeArea   *area);
GTHREE_API
void            gthree_area_set_scene    (GthreeArea   *area,
                                          GthreeScene  *scene);
GTHREE_API
GthreeCamera *  gthree_area_get_camera   (GthreeArea   *area);
GTHREE_API
void            gthree_area_set_camera   (GthreeArea   *area,
                                          GthreeCamera *camera);
GTHREE_API
GthreeRenderer *gthree_area_get_renderer (GthreeArea   *area);

G_END_DECLS

#endif /* __GTHREE_AREA_H__ */
