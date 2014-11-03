#include <math.h>
#include <epoxy/gl.h>

#include "gthreearea.h"
#include "gthreerenderer.h"
#include "gthreemarshalers.h"

typedef struct {
  GthreeRenderer *renderer;
  GthreeScene *scene;
  GthreeCamera *camera;
} GthreeAreaPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeArea, gthree_area, GTK_TYPE_GL_AREA);

static gboolean gthree_area_render        (GtkGLArea     *area,
                                           GdkGLContext  *context);
static void     gthree_area_realize       (GtkWidget     *widget);
static void     gthree_area_unrealize     (GtkWidget     *widget);

GtkWidget *
gthree_area_new (GthreeScene *scene,
                 GthreeCamera *camera)
{
  GthreeArea *area;
  GthreeAreaPrivate *priv;

  area = g_object_new (gthree_area_get_type (),
                       "has-depth-buffer", TRUE,
                       "profile", GDK_GL_PROFILE_3_2_CORE,
                       NULL);

  priv = gthree_area_get_instance_private (area);

  if (scene)
    priv->scene = g_object_ref (scene);

  if (camera)
    priv->camera = g_object_ref (camera);

  return GTK_WIDGET (area);
}

static void
gthree_area_init (GthreeArea *area)
{
}

static void
gthree_area_finalize (GObject *obj)
{
  GthreeArea *area = GTHREE_AREA (obj);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  g_clear_object (&priv->scene);
  g_clear_object (&priv->camera);

  G_OBJECT_CLASS (gthree_area_parent_class)->finalize (obj);
}

/* new window size or realize */
static void
gthree_area_real_resize (GtkGLArea *gl_area, int width, int height)
{
  GthreeArea *area = GTHREE_AREA (gl_area);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  GTK_GL_AREA_CLASS (gthree_area_parent_class)->resize (gl_area, width, height);

  gthree_renderer_set_size (priv->renderer, width, height);
}

static GdkGLContext *
gthree_area_real_create_context (GtkGLArea *gl_area)
{
  GthreeArea *area = GTHREE_AREA (gl_area);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);
  GdkGLContext *context = NULL;

  if (priv->scene)
    {
      context = gthree_scene_get_context (priv->scene);
      if (context != NULL)
        g_object_ref (context);
    }

  if (context == NULL)
    context = GTK_GL_AREA_CLASS (gthree_area_parent_class)->create_context (gl_area);

  if (priv->scene && context)
    gthree_scene_set_context (priv->scene, context);

  return context;
}

static void
gthree_area_class_init (GthreeAreaClass *klass)
{
  GTK_GL_AREA_CLASS (klass)->resize = gthree_area_real_resize;
  GTK_GL_AREA_CLASS (klass)->create_context = gthree_area_real_create_context;
  GTK_GL_AREA_CLASS (klass)->render = gthree_area_render;
  GTK_WIDGET_CLASS (klass)->realize = gthree_area_realize;
  GTK_WIDGET_CLASS (klass)->unrealize = gthree_area_unrealize;
  G_OBJECT_CLASS (klass)->finalize = gthree_area_finalize;
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
                            priv->camera,
                            FALSE);

  return TRUE;
}

static void
gthree_area_realize (GtkWidget *widget)
{
  GtkGLArea *glarea = GTK_GL_AREA (widget);
  GthreeArea *area = GTHREE_AREA(widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  GTK_WIDGET_CLASS (gthree_area_parent_class)->realize (widget);

  gtk_gl_area_make_current (glarea);
  priv->renderer = gthree_renderer_new ();
}

static void
gthree_area_unrealize (GtkWidget *widget)
{
  GthreeArea *area = GTHREE_AREA(widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  if (priv->scene)
    gthree_scene_set_context (priv->scene, NULL);

  g_clear_object (&priv->renderer);

  GTK_WIDGET_CLASS (gthree_area_parent_class)->unrealize (widget);
}

GthreeRenderer *
gthree_area_get_renderer (GthreeArea *area)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  return priv->renderer;
}
