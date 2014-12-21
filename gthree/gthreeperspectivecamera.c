#include <math.h>
#include <epoxy/gl.h>

#include "gthreeperspectivecamera.h"
#include "gthreeprivate.h"

typedef struct {
  float fov;
  float aspect;
} GthreePerspectiveCameraPrivate;

enum {
  PROP_0,

  PROP_FOV,
  PROP_ASPECT,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreePerspectiveCamera, gthree_perspective_camera, GTHREE_TYPE_CAMERA)

static void
gthree_perspective_camera_update (GthreeCamera *camera)
{
  GthreePerspectiveCamera *perspective = GTHREE_PERSPECTIVE_CAMERA (camera);
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);
  graphene_matrix_t *m = gthree_camera_get_projection_matrix_for_write (GTHREE_CAMERA (perspective));

  graphene_matrix_init_perspective (m,
                                    priv->fov,
                                    priv->aspect,
                                    gthree_camera_get_near (camera),
                                    gthree_camera_get_far (camera));
}

static void
gthree_perspective_camera_set_property (GObject *obj,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  GthreePerspectiveCamera *camera = GTHREE_PERSPECTIVE_CAMERA (obj);

  switch (prop_id)
    {
    case PROP_FOV:
      gthree_perspective_camera_set_fov (camera, g_value_get_float (value));
      break;

    case PROP_ASPECT:
      gthree_perspective_camera_set_aspect (camera, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_perspective_camera_get_property (GObject *obj,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  GthreePerspectiveCamera *camera = GTHREE_PERSPECTIVE_CAMERA (obj);
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (camera);

  switch (prop_id)
    {
    case PROP_FOV:
      g_value_set_float (value, priv->fov);
      break;

    case PROP_ASPECT:
      g_value_set_float (value, priv->aspect);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_perspective_camera_class_init (GthreePerspectiveCameraClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_perspective_camera_set_property;
  gobject_class->get_property = gthree_perspective_camera_get_property;

  GTHREE_CAMERA_CLASS (klass)->update = gthree_perspective_camera_update;

  obj_props[PROP_FOV] =
    g_param_spec_float ("fov", "Field of View", "Field of View",
                        -G_MAXFLOAT, G_MAXFLOAT, 50,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ASPECT] =
    g_param_spec_float ("aspect", "Aspect", "Aspect",
                        -G_MAXFLOAT, G_MAXFLOAT, 1,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_perspective_camera_init (GthreePerspectiveCamera *perspective)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);

  priv->fov = 50;
  priv->aspect = 1;
}

GthreePerspectiveCamera *
gthree_perspective_camera_new (float fov, float aspect, float near, float far)
{
  return g_object_new (gthree_perspective_camera_get_type (),
                       "fov", fov,
                       "aspect", aspect,
                       "near", near,
                       "far", far,
                       NULL);
}

void
gthree_perspective_camera_set_fov (GthreePerspectiveCamera *perspective,
                                   float fov)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);

  priv->fov = fov;

  gthree_camera_update (GTHREE_CAMERA (perspective));

  g_object_notify_by_pspec (G_OBJECT (perspective), obj_props[PROP_FOV]);
}

float
gthree_perspective_camera_get_fov (GthreePerspectiveCamera *perspective)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);

  return priv->fov;
}

void
gthree_perspective_camera_set_aspect (GthreePerspectiveCamera *perspective,
                                      float aspect)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);

  priv->aspect = aspect;

  gthree_camera_update (GTHREE_CAMERA (perspective));

  g_object_notify_by_pspec (G_OBJECT (perspective), obj_props[PROP_ASPECT]);
}

float
gthree_perspective_camera_get_aspect (GthreePerspectiveCamera *perspective)
{
  GthreePerspectiveCameraPrivate *priv = gthree_perspective_camera_get_instance_private (perspective);

  return priv->aspect;
}
