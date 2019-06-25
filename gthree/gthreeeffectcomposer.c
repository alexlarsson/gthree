#include <math.h>

#include "gthreeeffectcomposer.h"
#include "gthreerenderer.h"


typedef struct {
  GthreeRenderTarget *render_target1;
  GthreeRenderTarget *render_target2;

  GthreeRenderTarget *write_buffer;
  GthreeRenderTarget *read_buffer;

  gboolean render_to_screen;
  GPtrArray *passes;

  int width;
  int height;

  GthreePass *copy_pass;

} GthreeEffectComposerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeEffectComposer, gthree_effect_composer, G_TYPE_OBJECT)

static void
gthree_effect_composer_init (GthreeEffectComposer *composer)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);

  priv->passes = g_ptr_array_new_with_free_func (g_object_unref);
  priv->render_to_screen = TRUE;

  priv->copy_pass = gthree_shader_pass_new (gthree_get_shader_from_library ("copy"), NULL);
}

static void
gthree_effect_composer_finalize (GObject *obj)
{
  GthreeEffectComposer *composer = GTHREE_EFFECT_COMPOSER (obj);
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);

  if (priv->render_target1)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->render_target1));
      g_object_unref (priv->render_target1);
    }

  if (priv->render_target2)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->render_target2));
      g_object_unref (priv->render_target2);
    }

  g_ptr_array_unref (priv->passes);

  G_OBJECT_CLASS (gthree_effect_composer_parent_class)->finalize (obj);
}

static void
gthree_effect_composer_class_init (GthreeEffectComposerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_effect_composer_finalize;
}

GthreeEffectComposer *
gthree_effect_composer_new (void)
{
  GthreeEffectComposer *composer = g_object_new (GTHREE_TYPE_EFFECT_COMPOSER, NULL);

  return composer;
}

static void
swap_buffers (GthreeEffectComposer *composer)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);
  GthreeRenderTarget *tmp;

  tmp = priv->write_buffer;
  priv->write_buffer = priv->read_buffer;
  priv->read_buffer = tmp;
}

void
gthree_effect_composer_add_pass (GthreeEffectComposer *composer,
                                 GthreePass     *pass)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);
  g_ptr_array_add (priv->passes, g_object_ref (pass));

  if (priv->width != 0)
    gthree_pass_resize (pass, priv->width, priv->height);
}

void
gthree_effect_composer_insert_pass (GthreeEffectComposer *composer,
                                    GthreePass     *pass,
                                    int index)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);

  g_ptr_array_insert (priv->passes, index, g_object_ref (pass));

  if (priv->width != 0)
    gthree_pass_resize (pass, priv->width, priv->height);
}

static gboolean
should_render_pass_to_screen (GthreeEffectComposer *composer,
                              int index)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);
  int i;

  /* The last enabled pass should render to the screen, but also any
     directly previous passes that don't need swapping, as these will share
     buffers. */
  for (i = index + 1; i < priv->passes->len; i++)
    {
      GthreePass *pass = g_ptr_array_index (priv->passes, i);
      if (pass->enabled && pass->need_swap)
        return FALSE;
    }

  return TRUE;
}

// deltaTime value is in seconds
void
gthree_effect_composer_render (GthreeEffectComposer *composer,
                               GthreeRenderer *renderer,
                               float delta_time)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);
  g_autoptr(GthreeRenderTarget) current_render_target = NULL;
  gboolean mask_active;
  int i;

  if (priv->render_target1 == NULL)
    gthree_effect_composer_reset (composer, renderer, NULL);

  current_render_target = gthree_renderer_get_render_target (renderer);
  if (current_render_target)
    g_object_ref (current_render_target);

  mask_active = FALSE;
  for (i = 0; i < priv->passes->len; i++)
    {
      GthreePass *pass = g_ptr_array_index (priv->passes, i);

      if (!pass->enabled)
        continue;

      pass->render_to_screen = priv->render_to_screen && should_render_pass_to_screen (composer, i);
      gthree_pass_render (pass, renderer,
                          priv->write_buffer, priv->read_buffer,
                          delta_time, mask_active);

      if (pass->need_swap)
        {
          if (mask_active)
            {
#ifdef TODO
              var context = this.renderer.context;
              context.stencilFunc( context.NOTEQUAL, 1, 0xffffffff );
              this.copyPass.render( this.renderer, this.writeBuffer, this.readBuffer, deltaTime );
              context.stencilFunc( context.EQUAL, 1, 0xffffffff );
#endif
            }
          swap_buffers (composer);

#ifdef TODO
          if (THREE.MaskPass !== undefined)
            {
              if (pass instanceof THREE.MaskPass)
                {
                  mask_active = TRUE;
                }
              else if (pass instanceof THREE.ClearMaskPass)
                {
                  mask_active = FALSE;
                }
            }
#endif
        }
    }

  gthree_renderer_set_render_target (renderer, current_render_target, 0, 0);
}

void
gthree_effect_composer_reset (GthreeEffectComposer *composer,
                              GthreeRenderer *renderer,
                              GthreeRenderTarget   *render_target)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);
  int old_width, old_height, i;

  old_width = priv->width;
  old_height = priv->height;

  if (priv->render_target1)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->render_target1));
      g_object_unref (priv->render_target1);
    }

  if (priv->render_target2)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->render_target2));
      g_object_unref (priv->render_target2);
    }

  if (render_target == NULL)
    {
      priv->width = gthree_renderer_get_width (renderer);
      priv->height = gthree_renderer_get_height (renderer);

      priv->render_target1 = gthree_render_target_new_full (priv->width,
                                                            priv->height,
                                                            GTHREE_WRAPPING_CLAMP,
                                                            GTHREE_WRAPPING_CLAMP,
                                                            GTHREE_FILTER_LINEAR,
                                                            GTHREE_FILTER_LINEAR,
                                                            GTHREE_TEXTURE_FORMAT_RGBA,
                                                            GTHREE_DATA_TYPE_UNSIGNED_BYTE,
                                                            1, GTHREE_ENCODING_FORMAT_SRGB,
                                                            FALSE, TRUE, FALSE, NULL);
    }
  else
    {
      priv->render_target1 = g_object_ref (render_target);
      priv->width = gthree_render_target_get_width (render_target);
      priv->height = gthree_render_target_get_height (render_target);
    }

  gthree_resource_use (GTHREE_RESOURCE (priv->render_target1));

  priv->render_target2 = gthree_render_target_clone (priv->render_target1);
  gthree_resource_use (GTHREE_RESOURCE (priv->render_target2));

  priv->write_buffer = priv->render_target1;
  priv->read_buffer = priv->render_target2;

  if (priv->width != old_width || priv->height != old_height)
    {
      for (i = 0; i < priv->passes->len; i++)
        {
          GthreePass *pass = g_ptr_array_index (priv->passes, i);
          gthree_pass_resize (pass, priv->width, priv->height);
        }
    }
}

void
gthree_effect_composer_set_size (GthreeEffectComposer *composer,
                                 int                   width,
                                 int                   height)
{
  GthreeEffectComposerPrivate *priv = gthree_effect_composer_get_instance_private (composer);
  int effective_width, effective_height, i;

  priv->width = width;
  priv->height = height;

  effective_width = priv->width; // * this._pixelRatio;
  effective_height = priv->height; // * this._pixelRatio;

  if (priv->render_target1)
    gthree_render_target_set_size (priv->render_target1,
                                   effective_width, effective_height);
  if (priv->render_target2)
    gthree_render_target_set_size (priv->render_target2,
                                   effective_width, effective_height);

  for (i = 0; i < priv->passes->len; i++)
    {
      GthreePass *pass = g_ptr_array_index (priv->passes, i);
      gthree_pass_resize (pass, effective_width, effective_height);
    }
}
