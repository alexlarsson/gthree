#ifndef __GTHREE_CAMERA_H__
#define __GTHREE_CAMERA_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_CAMERA      (gthree_camera_get_type ())
#define GTHREE_CAMERA(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_CAMERA, GthreeCamera))
#define GTHREE_CAMERA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_CAMERA, GthreeCameraClass))
#define GTHREE_IS_CAMERA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_CAMERA))
#define GTHREE_IS_CAMERA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_CAMERA))
#define GTHREE_CAMERA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_CAMERA, GthreeCameraClass))

struct _GthreeCamera {
  GthreeObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeCamera, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

  void (*update) (GthreeCamera *camera);
} GthreeCameraClass;

GTHREE_API
GType gthree_camera_get_type (void) G_GNUC_CONST;

GTHREE_API
void                     gthree_camera_update_matrix            (GthreeCamera      *camera);
GTHREE_API
void                     gthree_camera_get_proj_screen_matrix   (GthreeCamera      *camera,
                                                                 graphene_matrix_t *res);
GTHREE_API
const graphene_matrix_t *gthree_camera_get_world_inverse_matrix (GthreeCamera      *camera);
GTHREE_API
const graphene_matrix_t *gthree_camera_get_projection_matrix    (GthreeCamera      *camera);
GTHREE_API
float                    gthree_camera_get_near                 (GthreeCamera      *camera);
GTHREE_API
void                     gthree_camera_set_near                 (GthreeCamera      *camera,
                                                                 float              near);
GTHREE_API
float                    gthree_camera_get_far                  (GthreeCamera      *camera);
GTHREE_API
void                     gthree_camera_set_far                  (GthreeCamera      *camera,
                                                                 float              far);
GTHREE_API
void                     gthree_camera_update                   (GthreeCamera      *camera);

GTHREE_API
void                     gthree_camera_unproject_point3d        (GthreeCamera      *camera,
                                                                 const graphene_point3d_t *pos,
                                                                 graphene_point3d_t *res);

G_END_DECLS

#endif /* __GTHREE_CAMERA_H__ */
