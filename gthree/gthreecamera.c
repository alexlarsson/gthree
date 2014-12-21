#include <math.h>

#include "gthreecamera.h"

typedef struct {
  graphene_matrix_t projection_matrix;
  graphene_matrix_t world_matrix_inverse;

  float near;
  float far;

} GthreeCameraPrivate;

enum {
  PROP_0,

  PROP_NEAR,
  PROP_FAR,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeCamera, gthree_camera, GTHREE_TYPE_OBJECT)

static void
gthree_camera_set_property (GObject *obj,
                            guint prop_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
  GthreeCamera *camera = GTHREE_CAMERA (obj);

  switch (prop_id)
    {
    case PROP_NEAR:
      gthree_camera_set_near (camera, g_value_get_float (value));
      break;

    case PROP_FAR:
      gthree_camera_set_far (camera, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_camera_get_property (GObject *obj,
                            guint prop_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  GthreeCamera *camera = GTHREE_CAMERA (obj);
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  switch (prop_id)
    {
    case PROP_NEAR:
      g_value_set_float (value, priv->near);
      break;

    case PROP_FAR:
      g_value_set_float (value, priv->far);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_camera_class_init (GthreeCameraClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_camera_set_property;
  gobject_class->get_property = gthree_camera_get_property;

  obj_props[PROP_NEAR] =
    g_param_spec_float ("near", "Near", "Near",
                        -G_MAXFLOAT, G_MAXFLOAT, 30.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_FAR] =
    g_param_spec_float ("far", "Far", "Far",
                        -G_MAXFLOAT, G_MAXFLOAT, 2000.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_camera_init (GthreeCamera *camera)
{
  GthreeCameraPrivate *priv = gthree_camera_get_instance_private (camera);

  priv->near = 30;
  priv->far = 2000;

  graphene_matrix_init_identity (&priv->projection_matrix);
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

  graphene_matrix_unproject_point3d (&priv->projection_matrix,
                                     gthree_object_get_world_matrix (GTHREE_OBJECT (camera)),
                                     pos, res);
}
