#include <math.h>
#include <epoxy/gl.h>

#include "gthreeperspectivecamera.h"
#include "gthreeprivate.h"

typedef struct {
  float fov;
  float aspect;
  float near;
  float far;
} GthreePerspectiveCameraPrivate;

static void gthree_perspective_camera_update (GthreePerspectiveCamera *perspective);


G_DEFINE_TYPE_WITH_PRIVATE (GthreePerspectiveCamera, gthree_perspective_camera, GTHREE_TYPE_CAMERA);

GthreePerspectiveCamera *
gthree_perspective_camera_new (float fov, float aspect, float near, float far)
{
  GthreePerspectiveCamera *perspective;
  GthreePerspectiveCameraPrivate *priv;

  perspective = g_object_new (gthree_perspective_camera_get_type (),
                           NULL);

  priv = gthree_perspective_camera_get_instance_private (perspective);

  priv->fov = fov;
  priv->aspect = aspect;
  priv->near = near;
  priv->far = far;

  gthree_perspective_camera_update (perspective);

  return perspective;
}

static void
gthree_perspective_camera_update (GthreePerspectiveCamera *perspective)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);
  graphene_matrix_t *m = gthree_camera_get_projection_matrix_for_write (GTHREE_CAMERA (perspective));

  graphene_matrix_init_perspective (m,
                                    priv->fov,
                                    priv->aspect,
                                    priv->near,
                                    priv->far);
}

void
gthree_perspective_camera_set_aspect (GthreePerspectiveCamera *perspective,
                                      float aspect)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);

  priv->aspect = aspect;

  gthree_perspective_camera_update (perspective);
}

static void
gthree_perspective_camera_init (GthreePerspectiveCamera *perspective)
{
}

static void
gthree_perspective_camera_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_perspective_camera_parent_class)->finalize (obj);
}

static void
gthree_perspective_camera_class_init (GthreePerspectiveCameraClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_perspective_camera_finalize;
}
