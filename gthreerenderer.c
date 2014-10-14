#include <math.h>
#include <epoxy/gl.h>

#include "gthreerenderer.h"

typedef struct {
  int width;
  int height;
  gboolean auto_clear;
  GdkRGBA clear_color;

  float viewport_x;
  float viewport_y;
  float viewport_width;
  float viewport_height;
} GthreeRendererPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeRenderer, gthree_renderer, G_TYPE_OBJECT);

GthreeRenderer *
gthree_renderer_new ()
{
  GthreeRenderer *renderer;

  renderer = g_object_new (gthree_renderer_get_type (),
                           NULL);

  return renderer;
}

static void
gthree_renderer_init (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear = TRUE;
  priv->width = 1;
  priv->height = 1;
}

static void
gthree_renderer_finalize (GObject *obj)
{
  //GthreeRenderer *renderer = GTHREE_RENDERER (obj);
  //GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  G_OBJECT_CLASS (gthree_renderer_parent_class)->finalize (obj);
}

static void
gthree_renderer_class_init (GthreeRendererClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_renderer_finalize;
}

void
gthree_renderer_set_viewport (GthreeRenderer *renderer,
                              float           x,
                              float           y,
                              float           width,
                              float           height)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->viewport_x = x;
  priv->viewport_y = y;
  priv->viewport_width = width;
  priv->viewport_height = height;

  glViewport (x, y, width, height);
}

void
gthree_renderer_set_size (GthreeRenderer *renderer,
                          int             width,
                          int             height)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->width = width;
  priv->height = height;

  gthree_renderer_set_viewport (renderer, 0, 0, width, height);
}

void
gthree_renderer_set_autoclear (GthreeRenderer *renderer,
                               gboolean        auto_clear)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear = !!auto_clear;
}

void
gthree_renderer_set_clear_color (GthreeRenderer *renderer,
                                 GdkRGBA        *color)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->clear_color = *color;
}

void
gthree_set_default_gl_state (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  glClearColor (0, 0, 0, 1);
  glClearDepth (1);
  glClearStencil (0);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  glFrontFace (GL_CCW);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);

  glEnable (GL_BLEND);
  glBlendEquation (GL_FUNC_ADD);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport (priv->viewport_x, priv->viewport_y, priv->viewport_width, priv->viewport_height);
  glClearColor (priv->clear_color.red, priv->clear_color.green, priv->clear_color.blue, priv->clear_color.alpha );
};

void
gthree_renderer_clear (GthreeRenderer *renderer)
{
}

void
gthree_renderer_render (GthreeRenderer *renderer,
                        GthreeScene    *scene,
                        GthreeCamera   *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), FALSE);
  if (gthree_object_get_parent (GTHREE_OBJECT (camera)) == NULL)
    gthree_object_update_matrix_world (GTHREE_OBJECT (camera), FALSE);

  if (priv->auto_clear)
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
