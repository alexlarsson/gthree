#include <math.h>
#include <epoxy/gl.h>

#include "gthreecamera.h"

typedef struct {
  graphene_matrix_t projection_matrix;
  graphene_matrix_t world_matrix_inverse;

  float fov;
  float aspect;
  float near;
  float far;

} GthreeCameraPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeCamera, gthree_camera, GTHREE_TYPE_OBJECT);

GthreeCamera *
gthree_camera_new (float fov, float aspect, float near, float far)
{
  GthreeCamera *camera;
  GthreeCameraPrivate *priv;

  // TODO: properties
  camera = g_object_new (gthree_camera_get_type (),
                         NULL);

  priv = gthree_camera_get_instance_private (camera);

  priv->fov = fov;
  priv->aspect = aspect;
  priv->near = near;
  priv->far = far;

  graphene_matrix_init_perspective (&priv->projection_matrix,
                                    priv->fov = fov,
                                    priv->aspect = aspect,
                                    priv->near = near,
                                    priv->far = far);

  return camera;
}

static void
gthree_camera_init (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  graphene_matrix_init_identity (&priv->projection_matrix);
}

static void
gthree_camera_finalize (GObject *obj)
{
  //GthreeCamera *camera = GTHREE_CAMERA (obj);
  //GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  G_OBJECT_CLASS (gthree_camera_parent_class)->finalize (obj);
}

static void
gthree_camera_class_init (GthreeCameraClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_camera_finalize;
}

void
gthree_camera_update_matrix (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  graphene_matrix_inverse (gthree_object_get_world_matrix (GTHREE_OBJECT (camera)),
                           &priv->world_matrix_inverse);
}

void
gthree_camera_get_proj_screen_matrix (GthreeCamera *camera,
                                      graphene_matrix_t *res)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  graphene_matrix_multiply (&priv->projection_matrix, &priv->world_matrix_inverse, res);
}
