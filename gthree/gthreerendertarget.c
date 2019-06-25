#include <math.h>
#include <epoxy/gl.h>

#include "gthreerendertarget.h"
#include "gthreetexture.h"
#include "gthreeprivate.h"

typedef struct {
  int width;
  int height;

  graphene_rect_t scissor;
  gboolean scissor_test;

  graphene_rect_t viewport;

  gboolean depth_buffer;
  gboolean stencil_buffer;

  GthreeTexture *texture;
  GthreeTexture *depth_texture;

  guint gl_framebuffer;
  guint gl_depthbuffer;
} GthreeRenderTargetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeRenderTarget, gthree_render_target, GTHREE_TYPE_RESOURCE)

static void
gthree_render_target_init (GthreeRenderTarget *target)
{
  //GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

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
gthree_render_target_used (GthreeResource *resource)
{
  GthreeRenderTarget *target = GTHREE_RENDER_TARGET (resource);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  if (priv->texture)
    gthree_resource_use (GTHREE_RESOURCE (priv->texture));

  if (priv->depth_texture)
    gthree_resource_use (GTHREE_RESOURCE (priv->depth_texture));
}

static void
gthree_render_target_unused (GthreeResource *resource)
{
  GthreeRenderTarget *target = GTHREE_RENDER_TARGET (resource);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  if (priv->texture)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->texture));

  if (priv->depth_texture)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->depth_texture));
}

static void
gthree_render_target_unrealize (GthreeResource *resource)
{
  GthreeRenderTarget *target = GTHREE_RENDER_TARGET (resource);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  if (priv->gl_framebuffer)
    {
      glDeleteFramebuffers (1, &priv->gl_framebuffer);
      priv->gl_framebuffer = 0;
    }

  if (priv->gl_depthbuffer)
    {
      glDeleteRenderbuffers (1, &priv->gl_depthbuffer);
      priv->gl_depthbuffer = 0;
    }
}

static void
gthree_render_target_class_init (GthreeRenderTargetClass *klass)
{
  GthreeResourceClass *resource_class = GTHREE_RESOURCE_CLASS (klass);

  G_OBJECT_CLASS (klass)->finalize = gthree_render_target_finalize;
  resource_class->unrealize = gthree_render_target_unrealize;
  resource_class->unused = gthree_render_target_unused;
  resource_class->used = gthree_render_target_used;
}

GthreeRenderTarget *
gthree_render_target_new_full (int width,
                               int height,
                               GthreeWrapping wrap_t,
                               GthreeWrapping wrap_s,
                               GthreeFilter mag_filter,
                               GthreeFilter min_filter,
                               GthreeTextureFormat format,
                               GthreeDataType data_type,
                               int anisotropy,
                               GthreeEncodingFormat encoding,
                               gboolean generate_mipmaps,
                               gboolean depth_buffer,
                               gboolean stencil_buffer,
                               GthreeTexture *depth_texture)
{
  GthreeRenderTarget *target = g_object_new (GTHREE_TYPE_RENDER_TARGET, NULL);
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);

  priv->width = width;
  priv->height = height;

  graphene_rect_init (&priv->scissor, 0, 0, width, height);
  priv->scissor_test = FALSE;

  graphene_rect_init (&priv->viewport, 0, 0, width, height);

  priv->texture = gthree_texture_new (NULL);

  gthree_texture_set_wrap_s (priv->texture, wrap_s);
  gthree_texture_set_wrap_t (priv->texture, wrap_t);

  gthree_texture_set_mag_filter (priv->texture, mag_filter);
  gthree_texture_set_min_filter (priv->texture, min_filter);
  gthree_texture_set_encoding (priv->texture, encoding);
  gthree_texture_set_format (priv->texture, format);
  gthree_texture_set_data_type (priv->texture, data_type);
  gthree_texture_set_anisotropy (priv->texture, anisotropy);
  gthree_texture_set_generate_mipmaps (priv->texture, generate_mipmaps);

  priv->depth_buffer = depth_buffer;
  priv->stencil_buffer = stencil_buffer;

  if (depth_texture)
    priv->depth_texture = g_object_ref (depth_texture);

  return target;
}

GthreeRenderTarget *
gthree_render_target_new (int width,
                          int height)
{
  return gthree_render_target_new_full (width, height,
                                        GTHREE_WRAPPING_CLAMP, GTHREE_WRAPPING_CLAMP,
                                        GTHREE_FILTER_LINEAR, GTHREE_FILTER_LINEAR,
                                        GTHREE_TEXTURE_FORMAT_RGBA, GTHREE_DATA_TYPE_UNSIGNED_BYTE,
                                        1, GTHREE_ENCODING_FORMAT_SRGB,
                                        FALSE, TRUE, TRUE,
                                        NULL);
}

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
        glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_STENCIL, priv->width, priv->height);

      glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl_renderbuffer);
    }
  else
    {
      guint gl_format = gthree_texture_format_to_gl (gthree_texture_get_format (priv->texture));
      guint gl_type = gthree_texture_data_type_to_gl (gthree_texture_get_data_type (priv->texture));
      guint gl_internal_format = gthree_texture_get_internal_gl_format (gl_format, gl_type);
      if (is_multisample)
        {
#ifdef TODO
          var samples = getRenderTargetSamples( renderTarget );
          _gl.renderbufferStorageMultisample( _gl.RENDERBUFFER, samples, glInternalFormat,
                                              renderTarget.width, renderTarget.height );
#endif
        }
      else
        glRenderbufferStorage (GL_RENDERBUFFER, gl_internal_format, priv->width, priv->height);
    }
  glBindRenderbuffer (GL_RENDERBUFFER, 0);
}

// Setup GL resources for a non-texture depth buffer
static void
setup_depth_renderbuffer (GthreeRenderTarget *render_target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (render_target);
  gboolean is_cube = FALSE;

#ifdef TODO
  is_cube = ( renderTarget.isWebGLRenderTargetCube === true );
#endif

  if (priv->depth_texture)
    {
      if (is_cube)
        {
          g_error ("target.depthTexture not supported in Cube render targets");
        }
#ifdef TODO
    setupDepthTexture( renderTargetProperties.__webglFramebuffer, renderTarget );
#endif
    }
  else
    {
      if (is_cube)
        {
#ifdef TODO
          renderTargetProperties.__webglDepthbuffer = [];
          for ( var i = 0; i < 6; i ++ )
            {
              _gl.bindFramebuffer( _gl.FRAMEBUFFER, renderTargetProperties.__webglFramebuffer[ i ] );
              renderTargetProperties.__webglDepthbuffer[ i ] = _gl.createRenderbuffer();
              setupRenderBufferStorage( renderTargetProperties.__webglDepthbuffer[ i ], renderTarget );
            }
#endif
        }
      else
        {
          glBindFramebuffer (GL_FRAMEBUFFER, priv->gl_framebuffer);
          glGenRenderbuffers (1, &priv->gl_depthbuffer);
          setup_renderbuffer_storage (render_target, priv->gl_depthbuffer, FALSE);
        }
    }

  glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

static gboolean
is_power_of_two (guint value)
{
  return value != 0 && (value & (value - 1)) == 0;
}

gboolean
gthree_render_target_is_power_of_two (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return is_power_of_two (priv->width) && is_power_of_two (priv->height);
}

guint
gthree_render_target_get_gl_framebuffer (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return priv->gl_framebuffer;
}

const graphene_rect_t *
gthree_render_target_get_viewport (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  return &priv->viewport;
}

void
gthree_render_target_realize (GthreeRenderTarget *target)
{
  GthreeRenderTargetPrivate *priv = gthree_render_target_get_instance_private (target);
  gboolean is_cube = FALSE, is_multisample = FALSE, supports_mips = FALSE;
  GthreeTexture *texture;

  if (priv->gl_framebuffer)
    return;

  gthree_resource_set_realized_for (GTHREE_RESOURCE (target), gdk_gl_context_get_current ());
  glGenFramebuffers (1, &priv->gl_framebuffer);

  texture = priv->texture;

#ifdef TODO
  is_cube = ( renderTarget.isWebGLRenderTargetCube === true );
  is_multisample = ( renderTarget.isWebGLMultisampleRenderTarget === true );
#endif
  supports_mips = gthree_render_target_is_power_of_two (target);

  // Setup framebuffer
  if (is_cube)
    {
#ifdef TODO
      renderTargetProperties.__webglFramebuffer = [];
      for ( var i = 0; i < 6; i ++ ) {
        renderTargetProperties.__webglFramebuffer[ i ] = _gl.createFramebuffer();
      }
#endif
    }
  else
    {
      if (is_multisample)
        {
#ifdef TODO
          renderTargetProperties.__webglMultisampledFramebuffer = _gl.createFramebuffer();
          renderTargetProperties.__webglColorRenderbuffer = _gl.createRenderbuffer();

          _gl.bindRenderbuffer( _gl.RENDERBUFFER, renderTargetProperties.__webglColorRenderbuffer );
          var glFormat = utils.convert( renderTarget.texture.format );
          var glType = utils.convert( renderTarget.texture.type );
          var glInternalFormat = getInternalFormat( glFormat, glType );
          var samples = getRenderTargetSamples( renderTarget );
          _gl.renderbufferStorageMultisample( _gl.RENDERBUFFER, samples, glInternalFormat, renderTarget.width, renderTarget.height );

          _gl.bindFramebuffer( _gl.FRAMEBUFFER, renderTargetProperties.__webglMultisampledFramebuffer );
          _gl.framebufferRenderbuffer( _gl.FRAMEBUFFER, _gl.COLOR_ATTACHMENT0, _gl.RENDERBUFFER, renderTargetProperties.__webglColorRenderbuffer );
          _gl.bindRenderbuffer( _gl.RENDERBUFFER, null );

          if ( renderTarget.depthBuffer )
            {
              renderTargetProperties.__webglDepthRenderbuffer = _gl.createRenderbuffer();
              setupRenderBufferStorage( renderTargetProperties.__webglDepthRenderbuffer, renderTarget, true );
            }
          _gl.bindFramebuffer( _gl.FRAMEBUFFER, null );
#endif
        }
    }

  // Setup color buffer
  if (is_cube)
    {
#ifdef TODO
      state.bindTexture( _gl.TEXTURE_CUBE_MAP, textureProperties.__webglTexture );
      setTextureParameters( _gl.TEXTURE_CUBE_MAP, renderTarget.texture, supportsMips );

      for ( var i = 0; i < 6; i ++ )
        {
          setupFrameBufferTexture( renderTargetProperties.__webglFramebuffer[ i ], renderTarget,
                                   _gl.COLOR_ATTACHMENT0, _gl.TEXTURE_CUBE_MAP_POSITIVE_X + i );
        }

      if ( textureNeedsGenerateMipmaps( renderTarget.texture, supportsMips ) )
        {
          generateMipmap( _gl.TEXTURE_CUBE_MAP, renderTarget.texture, renderTarget.width, renderTarget.height );
        }
      state.bindTexture( _gl.TEXTURE_CUBE_MAP, null );
#endif
    }
  else
    {
      gthree_texture_bind (texture, -1, GL_TEXTURE_2D);
      gthree_texture_set_parameters (GL_TEXTURE_2D, texture, supports_mips);
      gthree_texture_setup_framebuffer (texture,
                                        priv->width,
                                        priv->height,
                                        priv->gl_framebuffer,
                                        GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
#ifdef TODO
      if ( textureNeedsGenerateMipmaps( renderTarget.texture, supportsMips ) )
        generateMipmap( _gl.TEXTURE_2D, renderTarget.texture, renderTarget.width, renderTarget.height );
#endif
      glBindTexture (GL_TEXTURE_2D, 0);
    }

  // Setup depth and stencil buffers
  if (priv->depth_buffer)
    setup_depth_renderbuffer (target);
}
