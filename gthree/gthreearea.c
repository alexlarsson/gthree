#include <math.h>
#include <epoxy/gl.h>

#include "gthreearea.h"
#include "gthreerenderer.h"
#include "gthreemarshalers.h"

enum
{
  RESIZE,

  LAST_SIGNAL
};

static guint area_signals[LAST_SIGNAL] = { 0, };


typedef struct {
  GthreeRenderer *renderer;
  GthreeScene *scene;
  GthreeCamera *camera;
} GthreeAreaPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeArea, gthree_area, GTK_TYPE_GL_AREA);

static gboolean gthree_area_render        (GtkGLArea     *area,
                                           GdkGLContext  *context);
static void     gthree_area_size_allocate (GtkWidget     *widget,
                                           GtkAllocation *allocation);
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
                        NULL);

  priv = gthree_area_get_instance_private (area);

  priv->scene = g_object_ref (scene);
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
gthree_area_real_resize (GthreeArea *area, int width, int height)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  gthree_renderer_set_size (priv->renderer, width, height);
}

static void
gthree_area_class_init (GthreeAreaClass *klass)
{
  klass->resize = gthree_area_real_resize;
  GTK_GL_AREA_CLASS (klass)->render = gthree_area_render;
  GTK_WIDGET_CLASS (klass)->realize = gthree_area_realize;
  GTK_WIDGET_CLASS (klass)->unrealize = gthree_area_unrealize;
  GTK_WIDGET_CLASS (klass)->size_allocate = gthree_area_size_allocate;
  G_OBJECT_CLASS (klass)->finalize = gthree_area_finalize;

  area_signals[RESIZE] =
    g_signal_new ("resize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GthreeAreaClass, resize),
                  NULL, NULL,
                  _gthree_marshal_VOID__INT_INT,
                  G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);
}

static gboolean
gthree_area_render (GtkGLArea    *gl_area,
                    GdkGLContext *context)
{
  GthreeArea *area = GTHREE_AREA (gl_area);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  gthree_renderer_render (priv->renderer,
                          priv->scene,
                          priv->camera,
                          FALSE);

  return TRUE;
}

static void
gthree_area_size_allocate (GtkWidget     *widget,
                           GtkAllocation *allocation)
{
  GtkGLArea *glarea = GTK_GL_AREA (widget);

  GTK_WIDGET_CLASS (gthree_area_parent_class)->size_allocate (widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gtk_gl_area_make_current (glarea);
      g_signal_emit (glarea, area_signals[RESIZE], 0, allocation->width, allocation->height, NULL);
    }
}

static void
gthree_area_realize (GtkWidget *widget)
{
  GtkGLArea *glarea = GTK_GL_AREA (widget);
  GthreeArea *area = GTHREE_AREA(widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);
  GtkAllocation allocation;

  GTK_WIDGET_CLASS (gthree_area_parent_class)->realize (widget);

  gtk_gl_area_make_current (glarea);

  priv->renderer = gthree_renderer_new ();

  gtk_widget_get_allocation (widget, &allocation);
  g_signal_emit (glarea, area_signals[RESIZE], 0, allocation.width, allocation.height, NULL);
}

static void
gthree_area_unrealize (GtkWidget *widget)
{
  GthreeArea *area = GTHREE_AREA(widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  g_clear_object (&priv->renderer);

  GTK_WIDGET_CLASS (gthree_area_parent_class)->unrealize (widget);
}
