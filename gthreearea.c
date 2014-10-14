#include <math.h>
#include <epoxy/gl.h>

#include "gthreearea.h"
#include "gthreerenderer.h"

typedef struct {
  GthreeRenderer *renderer;
  GthreeScene *scene;
  GthreeCamera *camera;
  guint tick;
} GthreeAreaPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeArea, gthree_area, GTK_TYPE_GL_AREA);

static gboolean gthree_area_render        (GtkGLArea     *area,
                                           GdkGLContext  *context);
static void     gthree_area_size_allocate (GtkWidget     *widget,
                                           GtkAllocation *allocation);
static void     gthree_area_realize       (GtkWidget     *widget);
static gboolean gthree_area_tick          (GtkWidget     *widget,
                                               GdkFrameClock *frame_clock,
                                               gpointer       user_data);

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
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  priv->renderer = gthree_renderer_new ();

  priv->tick = gtk_widget_add_tick_callback (GTK_WIDGET (area), gthree_area_tick, area, NULL);
}

static void
gthree_area_finalize (GObject *obj)
{
  GthreeArea *area = GTHREE_AREA (obj);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  g_clear_object (&priv->scene);
  g_clear_object (&priv->camera);

  g_clear_object (&priv->renderer);
  gtk_widget_remove_tick_callback (GTK_WIDGET (area), priv->tick);

  G_OBJECT_CLASS (gthree_area_parent_class)->finalize (obj);
}

static void
gthree_area_class_init (GthreeAreaClass *klass)
{
  GTK_GL_AREA_CLASS (klass)->render = gthree_area_render;
  GTK_WIDGET_CLASS (klass)->realize = gthree_area_realize;
  GTK_WIDGET_CLASS (klass)->size_allocate = gthree_area_size_allocate;
  G_OBJECT_CLASS (klass)->finalize = gthree_area_finalize;
}

static gboolean
gthree_area_render (GtkGLArea    *gl_area,
                    GdkGLContext *context)
{
  GthreeArea *area = GTHREE_AREA (gl_area);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  gthree_renderer_render (priv->renderer,
                          priv->scene,
                          priv->camera);

  return TRUE;
}

/* new window size or exposure */
static void
reshape (GthreeArea *area, int width, int height)
{
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);

  gthree_renderer_set_size (priv->renderer, width, height);
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
      reshape (GTHREE_AREA (glarea), allocation->width, allocation->height);
    }
}

static void
gthree_area_realize (GtkWidget *widget)
{
  GtkGLArea *glarea = GTK_GL_AREA (widget);
  GthreeArea *area = GTHREE_AREA(widget);
  GtkAllocation allocation;

  GTK_WIDGET_CLASS (gthree_area_parent_class)->realize (widget);

  gtk_gl_area_make_current (glarea);

  gtk_widget_get_allocation (widget, &allocation);
  reshape (area, allocation.width, allocation.height);
}

static gboolean
gthree_area_tick (GtkWidget     *widget,
                      GdkFrameClock *frame_clock,
                      gpointer       user_data)
{
  /*
  GthreeArea *area = GTHREE_AREA (widget);
  GthreeAreaPrivate *priv = gthree_area_get_instance_private (area);
  */

  return G_SOURCE_CONTINUE;
}
