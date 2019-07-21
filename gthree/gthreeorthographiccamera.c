#include <math.h>
#include <epoxy/gl.h>

#include "gthreeorthographiccamera.h"
#include "gthreeprivate.h"

typedef struct {
  float left;
  float right;
  float top;
  float bottom;
} GthreeOrthographicCameraPrivate;

enum {
  PROP_0,

  PROP_LEFT,
  PROP_RIGHT,
  PROP_TOP,
  PROP_BOTTOM,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeOrthographicCamera, gthree_orthographic_camera, GTHREE_TYPE_CAMERA)

static void
gthree_orthographic_camera_update (GthreeCamera *camera)
{
  GthreeOrthographicCamera *orthographic = GTHREE_ORTHOGRAPHIC_CAMERA (camera);
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);
  graphene_matrix_t *m = gthree_camera_get_projection_matrix_for_write (GTHREE_CAMERA (orthographic));

  graphene_matrix_init_ortho (m,
                              priv->left,
                              priv->right,
                              priv->top,
                              priv->bottom,
                              gthree_camera_get_near (camera),
                              gthree_camera_get_far (camera));
}

static void
gthree_orthographic_camera_set_property (GObject *obj,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
  GthreeOrthographicCamera *camera = GTHREE_ORTHOGRAPHIC_CAMERA (obj);

  switch (prop_id)
    {
    case PROP_LEFT:
      gthree_orthographic_camera_set_left (camera, g_value_get_float (value));
      break;

    case PROP_RIGHT:
      gthree_orthographic_camera_set_right (camera, g_value_get_float (value));
      break;

    case PROP_TOP:
      gthree_orthographic_camera_set_top (camera, g_value_get_float (value));
      break;

    case PROP_BOTTOM:
      gthree_orthographic_camera_set_bottom (camera, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_orthographic_camera_get_property (GObject *obj,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
  GthreeOrthographicCamera *camera = GTHREE_ORTHOGRAPHIC_CAMERA (obj);
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (camera);

  switch (prop_id)
    {
    case PROP_LEFT:
      g_value_set_float (value, priv->left);
      break;

    case PROP_RIGHT:
      g_value_set_float (value, priv->right);
      break;

    case PROP_TOP:
      g_value_set_float (value, priv->top);
      break;

    case PROP_BOTTOM:
      g_value_set_float (value, priv->bottom);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_orthographic_camera_class_init (GthreeOrthographicCameraClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_orthographic_camera_set_property;
  gobject_class->get_property = gthree_orthographic_camera_get_property;

  GTHREE_CAMERA_CLASS (klass)->update = gthree_orthographic_camera_update;

  obj_props[PROP_LEFT] =
    g_param_spec_float ("left", "Left", "Left",
                        -G_MAXFLOAT, G_MAXFLOAT, -1,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_RIGHT] =
    g_param_spec_float ("right", "Right", "Right",
                        -G_MAXFLOAT, G_MAXFLOAT, 1,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_BOTTOM] =
    g_param_spec_float ("bottom", "Bottom", "Bottom",
                        -G_MAXFLOAT, G_MAXFLOAT, -1,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_TOP] =
    g_param_spec_float ("top", "Top", "Top",
                        -G_MAXFLOAT, G_MAXFLOAT, 1,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_orthographic_camera_init (GthreeOrthographicCamera *orthographic)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  priv->left = -1;
  priv->right = 1;
  priv->top = 1;
  priv->bottom = -1;
}

GthreeOrthographicCamera *
gthree_orthographic_camera_new (float left, float right,
                                float top, float bottom,
                                float near, float far)
{
  return g_object_new (gthree_orthographic_camera_get_type (),
                       "left", left,
                       "right", right,
                       "top", top,
                       "bottom", bottom,
                       "near", near,
                       "far", far,
                       NULL);
}

void
gthree_orthographic_camera_set_left (GthreeOrthographicCamera *orthographic,
                                     float left)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  priv->left = left;

  gthree_camera_update (GTHREE_CAMERA (orthographic));

  g_object_notify_by_pspec (G_OBJECT (orthographic), obj_props[PROP_LEFT]);
}

float
gthree_orthographic_camera_get_left (GthreeOrthographicCamera *orthographic)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  return priv->left;
}

void
gthree_orthographic_camera_set_right (GthreeOrthographicCamera *orthographic,
                                     float right)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  priv->right = right;

  gthree_camera_update (GTHREE_CAMERA (orthographic));

  g_object_notify_by_pspec (G_OBJECT (orthographic), obj_props[PROP_RIGHT]);
}

float
gthree_orthographic_camera_get_right (GthreeOrthographicCamera *orthographic)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  return priv->right;
}

void
gthree_orthographic_camera_set_top (GthreeOrthographicCamera *orthographic,
                                     float top)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  priv->top = top;

  gthree_camera_update (GTHREE_CAMERA (orthographic));

  g_object_notify_by_pspec (G_OBJECT (orthographic), obj_props[PROP_TOP]);
}

float
gthree_orthographic_camera_get_top (GthreeOrthographicCamera *orthographic)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  return priv->top;
}

void
gthree_orthographic_camera_set_bottom (GthreeOrthographicCamera *orthographic,
                                       float bottom)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  priv->bottom = bottom;

  gthree_camera_update (GTHREE_CAMERA (orthographic));

  g_object_notify_by_pspec (G_OBJECT (orthographic), obj_props[PROP_BOTTOM]);
}

float
gthree_orthographic_camera_get_bottom (GthreeOrthographicCamera *orthographic)
{
  GthreeOrthographicCameraPrivate *priv = gthree_orthographic_camera_get_instance_private (orthographic);

  return priv->bottom;
}
