#ifndef __GTHREE_CAMERA_H__
#define __GTHREE_CAMERA_H__

#include <gtk/gtk.h>

#include "gthreeobject.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_CAMERA      (gthree_camera_get_type ())
#define GTHREE_CAMERA(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_CAMERA, \
                                                             GthreeCamera))
#define GTHREE_IS_CAMERA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_CAMERA))

typedef struct {
  GthreeObject parent;
} GthreeCamera;

typedef struct {
  GthreeObjectClass parent_class;

} GthreeCameraClass;

GType gthree_camera_get_type (void) G_GNUC_CONST;

GthreeCamera *gthree_camera_new (float fov, float aspect, float near, float far);

void gthree_camera_update_matrix (GthreeCamera *camera);
void gthree_camera_get_proj_screen_matrix (GthreeCamera *camera,
                                           graphene_matrix_t *res);


G_END_DECLS

#endif /* __GTHREE_CAMERA_H__ */
