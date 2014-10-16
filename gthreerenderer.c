#include <math.h>
#include <epoxy/gl.h>

#include "gthreerenderer.h"
#include "gthreeobjectprivate.h"

typedef struct {
  int width;
  int height;
  gboolean auto_clear;
  GdkRGBA clear_color;
  gboolean sort_objects;
  GthreeMaterial *override_material;

  float viewport_x;
  float viewport_y;
  float viewport_width;
  float viewport_height;

  /* Render state */
  graphene_matrix_t proj_screen_matrix;

  GthreeBlendMode old_blending;
  GthreeBlendEquation old_blend_equation;
  GthreeBlendSrcFactor old_blend_src;
  GthreeBlendDstFactor old_blend_dst;

  GPtrArray *opaque_objects;
  GPtrArray *transparent_objects;

} GthreeRendererPrivate;

static void gthree_set_default_gl_state (GthreeRenderer *renderer);

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
  priv->sort_objects = TRUE;
  priv->width = 1;
  priv->height = 1;

  priv->opaque_objects = g_ptr_array_new ();
  priv->transparent_objects = g_ptr_array_new ();

  priv->old_blending = -1;
  priv->old_blend_equation = -1;
  priv->old_blend_src = -1;
  priv->old_blend_dst = -1;

  gthree_set_default_gl_state (renderer);
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

static void
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

static gint
painter_sort_stable (gconstpointer  a, gconstpointer  b)
{
  /* TODO */
  return 0;
}

static gint
reverse_painter_sort_stable (gconstpointer  a, gconstpointer  b)
{
  /* TODO */
  return 0;
}

static void
set_blending (GthreeRenderer *renderer,
              GthreeBlendMode blending,
              GthreeBlendEquation blend_equation,
              GthreeBlendSrcFactor blend_src,
              GthreeBlendDstFactor blend_dst)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (blending != priv->old_blending)
    {
      switch (blending)
        {
        default:
        case GTHREE_BLEND_NO:
          glDisable (GL_BLEND);
          break;

        case GTHREE_BLEND_NORMAL:
          glEnable (GL_BLEND);
          glBlendEquationSeparate (GL_FUNC_ADD, GL_FUNC_ADD);
          glBlendFuncSeparate (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          break;

        case GTHREE_BLEND_ADDITIVE:
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_SRC_ALPHA, GL_ONE);
          break;

        case GTHREE_BLEND_SUBTRACTIVE:
          // TODO: Find blendFuncSeparate() combination
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
          break;

        case GTHREE_BLEND_MULTIPLY:
          // TODO: Find blendFuncSeparate() combination
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_ZERO, GL_SRC_COLOR);
          break;

        case GTHREE_BLEND_CUSTOM:
          glEnable (GL_BLEND);
          break;
        }
      priv->old_blending = blending;
    }

  if (blending == GTHREE_BLEND_CUSTOM)
    {
      if (blend_equation != priv->old_blend_equation)
        {
          glBlendEquation (blend_equation);
          priv->old_blend_equation = blend_equation;
        }

      if (blend_src != priv->old_blend_src || blend_dst != priv->old_blend_dst)
        {
          glBlendFunc (blend_src, blend_dst);

          priv->old_blend_src = blend_src;
          priv->old_blend_dst = blend_dst;
        }
    }
  else
    {
      priv->old_blend_equation = -1;
      priv->old_blend_src = -1;
      priv->old_blend_dst = -1;
    }
}

static void
unroll_buffer_material (GthreeRenderer *renderer,
                        GthreeObjectBuffer *object_buffer)
{
  // TODO
}

static void
project_object (GthreeRenderer *renderer,
                GthreeScene    *scene,
                GthreeObject   *object,
                GthreeCamera   *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GList *l, *object_buffers;
  GthreeObject *child;
  GthreeObjectIter iter;

  if (!gthree_object_get_visible (object))
    return;

  object_buffers = gthree_object_get_object_buffers (object);

  if (object_buffers != NULL  /* && ( !g_three_object_get_frustum_culled (object) || priv->frustum.intersectsObject (object))  */)
    {
      gthree_object_update (object);

      for (l = object_buffers; l != NULL; l = l->next)
        {
          GthreeObjectBuffer *object_buffer = l->data;

          unroll_buffer_material (renderer, object_buffer);

          if (priv->sort_objects)
            {
              /* TODO
              if (object.renderDepth != null)
                {
                  object_buffer->z = object.renderDepth;
                }
                else */
                {
                  graphene_vec4_t vector;

                  /* Get position */
                  graphene_matrix_get_row (gthree_object_get_world_matrix (object), 3, &vector);

                  /* project object position to screen */
                  graphene_matrix_transform_vec4 (&priv->proj_screen_matrix, &vector, &vector);
                  graphene_vec4_normalize (&vector, &vector);

                  object_buffer->z = graphene_vec4_get_z (&vector);
                }
            }
        }
    }

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    project_object (renderer, scene, child, camera);
}

static void
render_objects (GthreeRenderer *renderer,
                GPtrArray *render_list,
                GthreeCamera *camera,
                gpointer lights,
                gpointer fog,
                gboolean use_blending,
                GthreeMaterial *override_material)
{
  /* TODO */
}

void
gthree_renderer_render (GthreeRenderer *renderer,
                        GthreeScene    *scene,
                        GthreeCamera   *camera,
                        gboolean force_clear)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeMaterial *override_material;
  gpointer lights, fog;

  lights = NULL;
  fog = NULL;

  /* update scene graph */

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), FALSE);

  /* update camera matrices and frustum */

  if (gthree_object_get_parent (GTHREE_OBJECT (camera)) == NULL)
    gthree_object_update_matrix_world (GTHREE_OBJECT (camera), FALSE);

  if (priv->auto_clear)
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gthree_camera_update_matrix (camera);

  gthree_camera_get_proj_screen_matrix (camera, &priv->proj_screen_matrix);

  gthree_scene_realize_objects (scene);

  g_ptr_array_set_size (priv->opaque_objects, 0);
  g_ptr_array_set_size (priv->transparent_objects, 0);

  project_object (renderer, scene, GTHREE_OBJECT (scene), camera);

  if (priv->sort_objects)
    {
      g_ptr_array_sort (priv->opaque_objects, painter_sort_stable);
      g_ptr_array_sort (priv->transparent_objects, reverse_painter_sort_stable);
    }

  //this.setRenderTarget( renderTarget );

  if (priv->auto_clear || force_clear )
    {
      //this.clear( this.autoClearColor, this.autoClearDepth, this.autoClearStencil );
    }

  /* set matrices for regular objects (frustum culled) */


  override_material = gthree_scene_get_override_material (scene);
  if (override_material)
    {
      /* TODO 
      set_blending (renderer, override_material.blending, override_material.blend_equation, override_material.blendSrc, override_material.blendDst );
      this.setDepthTest( override_material.depthTest );
      this.setDepthWrite( override_material.depthWrite );
      setPolygonOffset( override_material.polygonOffset, override_material.polygonOffsetFactor, override_material.polygonOffsetUnits );

      render_objects (renderer, renderer->opaque_objects, camera, lights, fog, true, override_material );
      render_objects (renderer, renderer->transparent_objects, camera, lights, fog, true, override_material );
      */
    }
  else
    {
      // opaque pass (front-to-back order)

      set_blending (renderer, GTHREE_BLEND_NO, 0, 0, 0);
      render_objects (renderer, priv->opaque_objects, camera, lights, fog, FALSE, NULL);

      // transparent pass (back-to-front order)
      render_objects (renderer, priv->transparent_objects, camera, lights, fog, TRUE, NULL);
    }
}
