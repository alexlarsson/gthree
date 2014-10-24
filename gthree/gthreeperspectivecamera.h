#ifndef __GTHREE_PERSPECTIVE_CAMERA_H__
#define __GTHREE_PERSPECTIVE_CAMERA_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include "gthreecamera.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_PERSPECTIVE_CAMERA      (gthree_perspective_camera_get_type ())
#define GTHREE_PERSPECTIVE_CAMERA(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_PERSPECTIVE_CAMERA, \
                                                                     GthreePerspectiveCamera))
#define GTHREE_IS_PERSPECTIVE_CAMERA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_PERSPECTIVE_CAMERA))

struct _GthreePerspectiveCamera {
  GthreeCamera parent;
};

typedef struct {
  GthreeCameraClass parent_class;

} GthreePerspectiveCameraClass;

GthreePerspectiveCamera *gthree_perspective_camera_new (float fov, float aspect, float near, float far);

GType gthree_perspective_camera_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_PERSPECTIVECAMERA_H__ */
