#include <math.h>
#include <epoxy/gl.h>

#include "gthreerendertarget.h"
#include "gthreetexture.h"
#include "gthreeprivate.h"

typedef struct {
#ifdef DEBUG_LABELS
  int instance_id;
#endif

  int width;
  int height;

  graphene_rect_t scissor;
  gboolean scissor_test;

  graphene_rect_t viewport;

  gboolean depth_buffer;
  gboolean stencil_buffer;
  gboolean is_cube;

  GthreeTexture *texture;
  GthreeTexture *depth_texture;

} GthreeRenderTargetPrivate;

typedef struct {
  GthreeResourceRealizeData parent;
  guint gl_framebuffer;
  guint gl_depthbuffer;
  guint gl_cube_framebuffers[6];
  guint gl_cube_depthbuffers[6];
} GthreeRenderTargetRealizeData;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeRenderTarget, gthree_render_target, GTHREE_TYPE_RESOURCE)

static void
gthree_render_target_init (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

#ifdef DEBUG_LABELS
  static int instance_count = 0;
  priv->instance_id = ++instance_count;
  g_autofree char *texture_name = g_strdup_printf ("rendertarget.%d.TEX", priv->instance_id);
#endif

  priv->scissor_test = FALSE;

  priv->texture = gthree_texture_new_empty (0, 0, GTHREE_MEMORY_FORMAT_R8G8B8A8);

#ifdef DEBUG_LABELS
  gthree_texture_set_name (priv->texture, texture_name);
#endif

  gthree_texture_set_wrap_s (priv->texture, GTHREE_WRAPPING_CLAMP);
  gthree_texture_set_wrap_t (priv->texture, GTHREE_WRAPPING_CLAMP);

  gthree_texture_set_generate_mipmaps (priv->texture, FALSE);
  gthree_texture_set_mag_filter (priv->texture, GTHREE_FILTER_LINEAR);
  gthree_texture_set_min_filter (priv->texture, GTHREE_FILTER_LINEAR);

  gthree_texture_set_encoding (priv->texture, GTHREE_ENCODING_FORMAT_LINEAR);
  gthree_texture_set_anisotropy (priv->texture, 1);

  priv->depth_buffer = TRUE;
  priv->stencil_buffer = TRUE;
}

static void
gthree_render_target_finalize (GObject *obj)
{
  GthreeRenderTarget *target = GTHREE_RENDER_TARGET (obj);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  g_clear_object (&priv->texture);
  g_clear_object (&priv->depth_texture);

  G_OBJECT_CLASS (gthree_render_target_parent_class)->finalize (obj);
}


static void
gthree_render_target_set_used (GthreeResource *resource,
                               gboolean        used)
{
  GthreeRenderTarget *target = GTHREE_RENDER_TARGET (resource);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  GTHREE_RESOURCE_CLASS (gthree_render_target_parent_class)->set_used (resource, used);

  if (priv->texture)
    gthree_resource_set_used (GTHREE_RESOURCE (priv->texture), used);

  if (priv->depth_texture)
    gthree_resource_set_used (GTHREE_RESOURCE (priv->depth_texture), used);
}

static void
gthree_render_target_unrealize (GthreeResource *resource,
                                GthreeRenderer *renderer)
{
  GthreeRenderTargetRealizeData *data = gthree_resource_get_data_for (resource, renderer);
  int i;

  if (data->gl_framebuffer)
    {
      gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_FRAMEBUFFER, data->gl_framebuffer);
      data->gl_framebuffer = 0;
    }

  if (data->gl_depthbuffer)
    {
      gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_RENDERBUFFER, data->gl_depthbuffer);
      data->gl_depthbuffer = 0;
    }

  for (i = 0; i < 6; i++)
    {
      if (data->gl_cube_framebuffers[i])
        {
          gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_FRAMEBUFFER, data->gl_cube_framebuffers[i]);
          data->gl_cube_framebuffers[i] = 0;
        }
      if (data->gl_cube_depthbuffers[i])
        {
          gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_RENDERBUFFER, data->gl_cube_depthbuffers[i]);
          data->gl_cube_depthbuffers[i] = 0;
        }
    }
}

static void
gthree_render_target_class_init (GthreeRenderTargetClass *klass)
{
  GthreeResourceClass *resource_class = GTHREE_RESOURCE_CLASS (klass);

  resource_class->realize_data_size = sizeof (GthreeRenderTargetRealizeData);

  G_OBJECT_CLASS (klass)->finalize = gthree_render_target_finalize;
  resource_class->unrealize = gthree_render_target_unrealize;
  resource_class->set_used = gthree_render_target_set_used;
}

GthreeRenderTarget *
gthree_render_target_new (int width,
                          int height)
{
  GthreeRenderTarget *target = g_object_new (GTHREE_TYPE_RENDER_TARGET, NULL);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  priv->width = width;
  priv->height = height;

  graphene_rect_init (&priv->scissor, 0, 0, width, height);
  graphene_rect_init (&priv->viewport, 0, 0, width, height);

  return target;
}

GthreeRenderTarget *
gthree_render_target_new_cube (int size)
{
  GthreeRenderTarget *target = g_object_new (GTHREE_TYPE_RENDER_TARGET, NULL);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  priv->width = size;
  priv->height = size;
  priv->is_cube = TRUE;

  graphene_rect_init (&priv->scissor, 0, 0, size, size);
  graphene_rect_init (&priv->viewport, 0, 0, size, size);

  gthree_texture_set_mapping (priv->texture, GTHREE_MAPPING_CUBE_REFLECTION);

  return target;
}

gboolean
gthree_render_target_get_is_cube (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->is_cube;
}

/**
 * gthree_render_target_clone:
 *
 * Returns: (transfer full):
 */
GthreeRenderTarget *
gthree_render_target_clone (GthreeRenderTarget *target)
{
  GthreeRenderTarget *clone;
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  GthreeRenderTargetPrivate *clone_priv;

  clone = gthree_render_target_new (priv->width, priv->height);
  clone_priv = gthree_render_target_get_instance_private (clone);

  clone_priv->scissor = priv->scissor;
  clone_priv->scissor_test = priv->scissor_test;

  clone_priv->viewport = priv->viewport;
  clone_priv->depth_buffer = priv->depth_buffer;
  clone_priv->stencil_buffer = priv->stencil_buffer;

  if (priv->depth_texture)
    clone_priv->depth_texture = g_object_ref (priv->depth_texture);

  gthree_texture_copy_settings (clone_priv->texture, priv->texture);

  return clone;
}

/**
 * gthree_render_target_get_texture:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_render_target_get_texture (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->texture;
}

int
gthree_render_target_get_width (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->width;
}

int
gthree_render_target_get_height (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->height;
}

void
gthree_render_target_set_size (GthreeRenderTarget *target,
                               int width,
                               int height)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  priv->width = width;
  priv->height = height;

  graphene_rect_init (&priv->scissor, 0, 0, width, height);
  graphene_rect_init (&priv->viewport, 0, 0, width, height);
}

gboolean
gthree_render_target_get_depth_buffer (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->depth_buffer;
}

void
gthree_render_target_set_depth_buffer (GthreeRenderTarget *target,
                                       gboolean            depth_buffer)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  priv->depth_buffer = depth_buffer;
}

gboolean
gthree_render_target_get_stencil_buffer (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->stencil_buffer;
}

void
gthree_render_target_set_stencil_buffer (GthreeRenderTarget *target,
                                         gboolean            stencil_buffer)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  priv->stencil_buffer = stencil_buffer;
}

/**
 * gthree_render_target_get_depth_texture:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_render_target_get_depth_texture (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->depth_texture;
}

void
gthree_render_target_set_depth_texture (GthreeRenderTarget *target,
                                        GthreeTexture *texture)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  g_set_object (&priv->depth_texture, texture);
}

// Setup storage for internal depth/stencil buffers and bind to correct framebuffer
static void
setup_renderbuffer_storage (GthreeRenderTarget *render_target, guint gl_renderbuffer, gboolean is_multisample)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (render_target);

  glBindRenderbuffer (GL_RENDERBUFFER, gl_renderbuffer);
  if (priv->depth_buffer && ! priv->stencil_buffer )
    {
      if (is_multisample)
        {
#ifdef TODO
          var samples = getRenderTargetSamples( renderTarget );
          _gl.renderbufferStorageMultisample( _gl.RENDERBUFFER, samples, _gl.DEPTH_COMPONENT16, renderTarget.width, renderTarget.height );
#endif
        }
      else
        glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, priv->width, priv->height);
      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gl_renderbuffer);
    }
  else if (priv->depth_buffer && priv->stencil_buffer )
    {
      if (is_multisample)
        {
#ifdef TODO
          var samples = getRenderTargetSamples( renderTarget );
          _gl.renderbufferStorageMultisample( _gl.RENDERBUFFER, samples, _gl.DEPTH_STENCIL, renderTarget.width, renderTarget.height );
#endif
        }
      else
        glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, priv->width, priv->height);

      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl_renderbuffer);
    }
  else
    {
      GthreeGLFormatInfo gl_info;
      gthree_memory_format_to_gl (gthree_texture_get_memory_format (priv->texture), &gl_info);
      if (is_multisample)
        {
#ifdef TODO
          var samples = getRenderTargetSamples( renderTarget );
          _gl.renderbufferStorageMultisample( _gl.RENDERBUFFER, samples, glInternalFormat,
                                              renderTarget.width, renderTarget.height );
#endif
        }
      else
        glRenderbufferStorage (GL_RENDERBUFFER, gl_info.gl_internal_format, priv->width, priv->height);
    }
  glBindRenderbuffer (GL_RENDERBUFFER, 0);
}

// Setup GL resources for a non-texture depth buffer
static void
setup_depth_renderbuffer (GthreeRenderTarget *render_target, GthreeRenderTargetRealizeData *data, GthreeRenderer *renderer)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (render_target);

  if (priv->depth_texture)
    {
      if (priv->is_cube)
        {
          g_error ("target.depthTexture not supported in Cube render targets");
        }

      glBindFramebuffer (GL_FRAMEBUFFER, data->gl_framebuffer);

      gthree_texture_realize (priv->depth_texture, renderer);
      gthree_texture_bind (priv->depth_texture, renderer, -1, GL_TEXTURE_2D);

      glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                    priv->width, priv->height, 0,
                    GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

      GthreeFilter min_filter = gthree_texture_get_min_filter (priv->depth_texture);
      GthreeFilter mag_filter = gthree_texture_get_mag_filter (priv->depth_texture);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       min_filter == GTHREE_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       mag_filter == GTHREE_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_TEXTURE_2D,
                              gthree_texture_get_gl_texture (priv->depth_texture, renderer),
                              0);
    }
  else
    {
      if (priv->is_cube)
        {
          int i;
          for (i = 0; i < 6; i++)
            {
              glBindFramebuffer (GL_FRAMEBUFFER, data->gl_cube_framebuffers[i]);
              glGenRenderbuffers (1, &data->gl_cube_depthbuffers[i]);
#ifdef DEBUG_LABELS
              {
                g_autofree char *label = g_strdup_printf ("rendertarget.%d.RB.cube_depth.%d", priv->instance_id, i);
                glObjectLabel (GL_RENDERBUFFER, data->gl_cube_depthbuffers[i], strlen (label), label);
              }
#endif
              setup_renderbuffer_storage (render_target, data->gl_cube_depthbuffers[i], FALSE);
            }
        }
      else
        {
          glBindFramebuffer (GL_FRAMEBUFFER, data->gl_framebuffer);
          glGenRenderbuffers (1, &data->gl_depthbuffer);
#ifdef DEBUG_LABELS
          {
            g_autofree char *label = g_strdup_printf ("rendertarget.%d.RB.depth", priv->instance_id);
            glObjectLabel (GL_RENDERBUFFER, data->gl_depthbuffer, strlen (label), label);
          }
#endif
          setup_renderbuffer_storage (render_target, data->gl_depthbuffer, FALSE);
        }
    }

  glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

guint
gthree_render_target_get_gl_framebuffer (GthreeRenderTarget *target,
                                         GthreeRenderer *renderer)
{
  GthreeRenderTargetRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (target), renderer);
  return data->gl_framebuffer;
}

guint
gthree_render_target_get_gl_framebuffer_for_face (GthreeRenderTarget *target,
                                                  GthreeRenderer *renderer,
                                                  int face)
{
  GthreeRenderTargetRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (target), renderer);
  g_return_val_if_fail (face >= 0 && face < 6, 0);
  return data->gl_cube_framebuffers[face];
}

const graphene_rect_t *
gthree_render_target_get_viewport (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return &priv->viewport;
}

static gboolean
texture_needs_generate_mipmaps (GthreeTexture *texture)
{
  GthreeFilter min_filter = gthree_texture_get_min_filter (texture);

  return
    gthree_texture_get_generate_mipmaps (texture) &&
    min_filter != GTHREE_FILTER_NEAREST &&
    min_filter != GTHREE_FILTER_LINEAR;
}

static void
generate_mipmap (guint target,
                 GthreeTexture *texture,
                 int width, int height)
{
  glGenerateMipmap (target);
  gthree_texture_set_max_mip_level (texture, log2 (MAX (width, height)));
}

void
gthree_render_target_update_mipmap (GthreeRenderTarget *target,
                                    GthreeRenderer *renderer)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  if (texture_needs_generate_mipmaps (priv->texture))
    {
      guint gl_target = priv->is_cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
      gthree_texture_bind (priv->texture, renderer, -1, gl_target);
      generate_mipmap (gl_target, priv->texture, priv->width, priv->height);
      glBindTexture (gl_target, 0);
    }
}

void
gthree_render_target_realize (GthreeRenderTarget *target,
                              GthreeRenderer *renderer)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  GthreeRenderTargetRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (target), renderer);
  GthreeTexture *texture;

  if (priv->is_cube)
    {
      if (data->gl_cube_framebuffers[0])
        return;
    }
  else
    {
      if (data->gl_framebuffer)
        return;
    }

  gthree_resource_set_realized_for (GTHREE_RESOURCE (target), renderer);

  texture = priv->texture;

  if (priv->is_cube)
    {
      int i;
      GthreeGLFormatInfo gl_info;

      glGenFramebuffers (6, data->gl_cube_framebuffers);
#ifdef DEBUG_LABELS
      for (i = 0; i < 6; i++)
        {
          g_autofree char *label = g_strdup_printf ("rendertarget.%d.FB.cube.%d", priv->instance_id, i);
          glObjectLabel (GL_FRAMEBUFFER, data->gl_cube_framebuffers[i], strlen (label), label);
        }
#endif

      gthree_texture_bind (texture, renderer, -1, GL_TEXTURE_CUBE_MAP);
      gthree_texture_set_parameters (GL_TEXTURE_CUBE_MAP, texture);

      gthree_memory_format_to_gl (gthree_texture_get_memory_format (texture), &gl_info);

      for (i = 0; i < 6; i++)
        {
          glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl_info.gl_internal_format,
                        priv->width, priv->height, 0, gl_info.gl_format, gl_info.gl_type, NULL);
          glBindFramebuffer (GL_FRAMEBUFFER, data->gl_cube_framebuffers[i]);
          glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                  gthree_texture_get_gl_texture (texture, renderer), 0);
        }

      if (texture_needs_generate_mipmaps (texture))
        generate_mipmap (GL_TEXTURE_CUBE_MAP, texture, priv->width, priv->height);

      glBindTexture (GL_TEXTURE_CUBE_MAP, 0);
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }
  else
    {
      glGenFramebuffers (1, &data->gl_framebuffer);
#ifdef DEBUG_LABELS
      {
        g_autofree char *label = g_strdup_printf ("rendertarget.%d.FB", priv->instance_id);
        glObjectLabel (GL_FRAMEBUFFER, data->gl_framebuffer, strlen (label), label);
      }
#endif

      gthree_texture_bind (texture, renderer, -1, GL_TEXTURE_2D);
      gthree_texture_set_parameters (GL_TEXTURE_2D, texture);
      gthree_texture_setup_framebuffer (texture, renderer,
                                        priv->width,
                                        priv->height,
                                        data->gl_framebuffer,
                                        GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
      if (texture_needs_generate_mipmaps (texture))
        generate_mipmap (GL_TEXTURE_2D, texture, priv->width, priv->height);
      glBindTexture (GL_TEXTURE_2D, 0);
    }

  if (priv->depth_buffer)
    setup_depth_renderbuffer (target, data, renderer);
}

void
gthree_render_target_download (GthreeRenderTarget *target,
                               GthreeRenderer *renderer,
                               guchar     *data,
                               gsize       stride,
                               gboolean    flip_y)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  cairo_rectangle_int_t all = { 0, 0, priv->width, priv->height };

  gthree_render_target_download_area (target, renderer, &all, data, stride, flip_y);
}

void
gthree_render_target_download_area (GthreeRenderTarget *target,
                                    GthreeRenderer *renderer,
                                    const cairo_rectangle_int_t *area,
                                    guchar     *data,
                                    gsize       stride,
                                    gboolean    flip_y)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  gthree_texture_bind (priv->texture, renderer, 0, GL_TEXTURE_2D);

  glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                             GL_TEXTURE_2D, gthree_texture_get_gl_texture (priv->texture, renderer), 0);
  glPixelStorei (GL_PACK_ALIGNMENT, 4);
  glPixelStorei (GL_PACK_ROW_LENGTH, stride / 4);

  glReadPixels (area->x, area->y, area->width, area->height, GL_RGBA, GL_UNSIGNED_BYTE, data);

  if (flip_y)
    {
      g_autofree guchar *row = g_malloc (stride);
      for (int i = 0; i < area->height / 2; i++)
        {
          guchar *top_row = data + i * stride;
          guchar *bottom_row = data + (area->height - 1 - i) * stride;
          memcpy (row, top_row, stride);
          memcpy (top_row, bottom_row, stride);
          memcpy (bottom_row, row, stride);
        }
    }

  glPixelStorei (GL_PACK_ROW_LENGTH, 0);
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
}
