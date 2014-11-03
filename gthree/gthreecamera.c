#include <math.h>
#include <epoxy/gl.h>

#include "gthreecamera.h"

typedef struct {
  graphene_matrix_t projection_matrix;
  graphene_matrix_t world_matrix_inverse;

  float near;
  float far;

} GthreeCameraPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeCamera, gthree_camera, GTHREE_TYPE_OBJECT);

static void
gthree_camera_init (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  priv->near = 30;
  priv->far = 2000;

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

  graphene_matrix_multiply (&priv->world_matrix_inverse, &priv->projection_matrix, res);
}

const graphene_matrix_t *
gthree_camera_get_projection_matrix (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  return &priv->projection_matrix;
}

graphene_matrix_t *
gthree_camera_get_projection_matrix_for_write (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  return &priv->projection_matrix;
}

const graphene_matrix_t *
gthree_camera_get_world_inverse_matrix (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  return &priv->world_matrix_inverse;
}

float
gthree_camera_get_near (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  return priv->near;
}

void
gthree_camera_set_near (GthreeCamera *camera,
                        float near)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  priv->near = near;

  gthree_camera_update (camera);
}

float
gthree_camera_get_far (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  return priv->far;
}

void
gthree_camera_set_far (GthreeCamera *camera,
                       float far)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  priv->far = far;

  gthree_camera_update (camera);
}

void
gthree_camera_update (GthreeCamera *camera)
{
  GthreeCameraClass *class = GTHREE_CAMERA_GET_CLASS(camera);

  class->update (camera);
}

void
gthree_camera_unproject_point3d (GthreeCamera *camera,
                                 const graphene_point3d_t *pos,
                                 graphene_point3d_t *res)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);
  graphene_matrix_t projection_matrix_inverse, view_projection_matrix;
  graphene_vec4_t v;
  float w_inv;

  graphene_matrix_inverse (&priv->projection_matrix, &projection_matrix_inverse);
  graphene_matrix_multiply (&projection_matrix_inverse,
                            gthree_object_get_world_matrix (GTHREE_OBJECT (camera)),
                            &view_projection_matrix);
  graphene_vec4_init (&v, pos->x, pos->y, pos->z, 1);
  graphene_matrix_transform_vec4 (&view_projection_matrix, &v, &v);

  w_inv = 1.0f / graphene_vec4_get_w (&v);
  graphene_point3d_init (res,
                         graphene_vec4_get_x (&v) * w_inv,
                         graphene_vec4_get_y (&v) * w_inv,
                         graphene_vec4_get_z (&v) * w_inv);
}
