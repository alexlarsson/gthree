#ifndef __GTHREE_CAMERA_H__
#define __GTHREE_CAMERA_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_CAMERA      (gthree_camera_get_type ())
#define GTHREE_CAMERA(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_CAMERA, \
                                                             GthreeCamera))
#define GTHREE_IS_CAMERA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_CAMERA))

struct _GthreeCamera {
  GthreeObject parent;
};

typedef struct {
  GthreeObjectClass parent_class;

} GthreeCameraClass;

GType gthree_camera_get_type (void) G_GNUC_CONST;

void                     gthree_camera_update_matrix            (GthreeCamera      *camera);
void                     gthree_camera_get_proj_screen_matrix   (GthreeCamera      *camera,
                                                                 graphene_matrix_t *res);
const graphene_matrix_t *gthree_camera_get_world_inverse_matrix (GthreeCamera      *camera);
const graphene_matrix_t *gthree_camera_get_projection_matrix    (GthreeCamera      *camera);

G_END_DECLS

#endif /* __GTHREE_CAMERA_H__ */
