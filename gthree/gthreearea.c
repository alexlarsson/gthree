#include <math.h>

#include "gthreearea.h"
#include "gthreerenderer.h"
#include "gthreemarshalers.h"

typedef struct {
  GthreeRenderer *renderer;
  GthreeScene *scene;
  GthreeCamera *camera;
} GthreeAreaPrivate;

enum {
  PROP_0,

  PROP_SCENE,
  PROP_CAMERA,
  PROP_RENDERER,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeArea, gthree_area, GTK_TYPE_GL_AREA)

static gboolean gthree_area_render        (GtkGLArea     *area,
                                           GdkGLContext  *context);
static void     gthree_area_realize       (GtkWidget     *widget);
static void     gthree_area_unrealize     (GtkWidget     *widget);

static void
gthree_area_dispose (GObject *obj)
{
  GthreeArea *area = GTHREE_AREA (obj);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  g_clear_object (&priv->scene);
  g_clear_object (&priv->camera);

  G_OBJECT_CLASS (gthree_area_parent_class)->dispose (obj);
}

static void
gthree_area_set_property (GObject *obj,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GthreeArea *area = GTHREE_AREA (obj);

  switch (prop_id)
    {
    case PROP_SCENE:
      gthree_area_set_scene (area, g_value_get_object (value));
      break;

    case PROP_CAMERA:
      gthree_area_set_camera (area, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_area_get_property (GObject *obj,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  GthreeArea *area = GTHREE_AREA (obj);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  switch (prop_id)
    {
    case PROP_SCENE:
      g_value_set_object (value, priv->scene);
      break;

    case PROP_CAMERA:
      g_value_set_object (value, priv->camera);
      break;

    case PROP_RENDERER:
      g_value_set_object (value, priv->renderer);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

/* new window size or realize */
static void
gthree_area_real_resize (GtkGLArea *gl_area, int width, int height)
{
  GthreeArea *area = GTHREE_AREA (gl_area);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);
  int scale;

  GTK_GL_AREA_CLASS (gthree_area_parent_class)->resize (gl_area, width, height);

  scale = gtk_widget_get_scale_factor (GTK_WIDGET (gl_area));
  gthree_renderer_set_size (priv->renderer, width / scale, height / scale);
  gthree_renderer_set_pixel_ratio (priv->renderer, scale);
}

static void
gthree_area_class_init (GthreeAreaClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkGLAreaClass *glarea_class = GTK_GL_AREA_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = gthree_area_set_property;
  gobject_class->get_property = gthree_area_get_property;
  gobject_class->dispose = gthree_area_dispose;

  widget_class->realize = gthree_area_realize;
  widget_class->unrealize = gthree_area_unrealize;

  glarea_class->resize = gthree_area_real_resize;
  glarea_class->render = gthree_area_render;

  obj_props[PROP_SCENE] =
    g_param_spec_object ("scene", "Scene", "Scene",
                         GTHREE_TYPE_SCENE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_CAMERA] =
    g_param_spec_object ("camera", "Camera", "Camera",
                         GTHREE_TYPE_CAMERA,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_RENDERER] =
    g_param_spec_object ("renderer", "Renderer", "Renderer",
                         GTHREE_TYPE_RENDERER,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_area_init (GthreeArea *area)
{
}

static gboolean
gthree_area_render (GtkGLArea    *gl_area,
                    GdkGLContext *context)
{
  GthreeArea *area = GTHREE_AREA (gl_area);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  if (priv->scene && priv->camera)
    gthree_renderer_render (priv->renderer,
                            priv->scene,
                            priv->camera);

  return TRUE;
}

static void
gthree_area_realize (GtkWidget *widget)
{
  GtkGLArea *glarea = GTK_GL_AREA (widget);
  GthreeArea *area = GTHREE_AREA(widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  GTK_WIDGET_CLASS (gthree_area_parent_class)->realize (widget);

  // Ensure we have the right target framebuffer
  gtk_gl_area_attach_buffers (glarea);

  priv->renderer = gthree_renderer_new ();
}

static void
gthree_area_unrealize (GtkWidget *widget)
{
  GtkGLArea *glarea = GTK_GL_AREA (widget);
  GthreeArea *area = GTHREE_AREA(widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  gtk_gl_area_make_current (glarea);

  gthree_resources_unrealize_all_for (gtk_gl_area_get_context (glarea));

  g_clear_object (&priv->renderer);

  GTK_WIDGET_CLASS (gthree_area_parent_class)->unrealize (widget);
}

GtkWidget *
gthree_area_new (GthreeScene *scene,
                 GthreeCamera *camera)
{
  return g_object_new (GTHREE_TYPE_AREA,
                       "has-depth-buffer", TRUE,
                       "scene", scene,
                       "camera", camera,
                       NULL);

}

GthreeRenderer *
gthree_area_get_renderer (GthreeArea *area)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  return priv->renderer;
}

void
gthree_area_set_scene (GthreeArea *area,
                       GthreeScene *scene)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  if (g_set_object (&priv->scene, scene))
    g_object_notify_by_pspec (G_OBJECT (area), obj_props[PROP_SCENE]);
}

GthreeScene *
gthree_area_get_scene (GthreeArea *area)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  return priv->scene;
}

void
gthree_area_set_camera (GthreeArea *area,
                        GthreeCamera *camera)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  if (g_set_object (&priv->camera, camera))
    g_object_notify_by_pspec (G_OBJECT (area), obj_props[PROP_CAMERA]);
}

GthreeCamera *
gthree_area_get_camera (GthreeArea *area)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  return priv->camera;
}
