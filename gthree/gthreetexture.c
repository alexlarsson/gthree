#include <math.h>
#include <epoxy/gl.h>

#include "gthreetexture.h"
#include "gthreeenums.h"

enum
{

  LAST_SIGNAL
};

/*static guint texture_signals[LAST_SIGNAL] = { 0, };*/

typedef struct {
  gboolean needs_update;

  GdkPixbuf *pixbuf;
  GArray *mipmaps;
  gpointer mapping;
  GthreeWrapping wrap_s;
  GthreeWrapping wrap_t;

  GthreeFilter mag_filter;
  GthreeFilter min_filter;

  int anisotropy;

  int format;
  int type;

  graphene_vec2_t offset;
  graphene_vec2_t repeat;

  gboolean generate_mipmaps;
  gboolean premultiply_alpha;
  gboolean flip_y;
  int unpack_alignment;

  guint gl_texture;
} GthreeTexturePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeTexture, gthree_texture, G_TYPE_OBJECT);

GthreeTexture *
gthree_texture_new (GdkPixbuf *pixbuf)
{
  GthreeTexture *texture;
  GthreeTexturePrivate *priv;

  texture = g_object_new (gthree_texture_get_type (),
                          NULL);

  priv = gthree_texture_get_instance_private (texture);
  priv->pixbuf = gdk_pixbuf_flip (pixbuf, FALSE);

  return texture;
}

static void
gthree_texture_init (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->anisotropy = 1;
  priv->unpack_alignment = 4;
  priv->flip_y = TRUE;
  priv->generate_mipmaps = TRUE;
  priv->wrap_s = GTHREE_WRAPPING_CLAMP;
  priv->wrap_t = GTHREE_WRAPPING_CLAMP;
  priv->mag_filter = GTHREE_FILTER_LINEAR;
  priv->min_filter = GTHREE_FILTER_LINEAR_MIPMAP_LINEAR;

  priv->format = GL_RGBA;
  priv->type = GL_UNSIGNED_BYTE;

  priv->needs_update = TRUE;

  graphene_vec2_init (&priv->offset, 0, 0);
  graphene_vec2_init (&priv->repeat, 1, 1);
}

static void
gthree_texture_finalize (GObject *obj)
{
  GthreeTexture *texture = GTHREE_TEXTURE (obj);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  g_clear_object (&priv->pixbuf);

  // TODO: This should be in unrealize to guarantee current context
  if (priv->gl_texture)
    glDeleteTextures (1, &priv->gl_texture);


  G_OBJECT_CLASS (gthree_texture_parent_class)->finalize (obj);
}

static void
gthree_texture_class_init (GthreeTextureClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_texture_finalize;

}

const graphene_vec2_t *
gthree_texture_get_repeat (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return &priv->repeat;
}

const graphene_vec2_t *
gthree_texture_get_offset (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return &priv->offset;
}

static guint
wrap_to_gl (GthreeWrapping wrap)
{
  switch (wrap)
    {
    default:
    case GTHREE_WRAPPING_REPEAT:
      return GL_REPEAT;
    case GTHREE_WRAPPING_CLAMP:
      return GL_CLAMP_TO_EDGE;
    case GTHREE_WRAPPING_MIRRORED:
      return GL_MIRRORED_REPEAT;
    }
}

static guint
filter_to_gl (GthreeFilter filter)
{
  switch (filter)
    {
    default:
    case GTHREE_FILTER_NEAREST:
      return GL_NEAREST;

    case GTHREE_FILTER_NEAREST_MIPMAP_NEAREST:
      return GL_NEAREST_MIPMAP_NEAREST;

    case GTHREE_FILTER_NEAREST_MIPMAP_LINEAR:
      return GL_NEAREST_MIPMAP_LINEAR;

    case GTHREE_FILTER_LINEAR:
      return GL_LINEAR;

    case GTHREE_FILTER_LINEAR_MIPMAP_NEAREST:
      return GL_LINEAR_MIPMAP_NEAREST;

    case GTHREE_FILTER_LINEAR_MIPMAP_LINEAR:
      return GL_LINEAR_MIPMAP_LINEAR;
    }
}

static guint
filter_fallback (GthreeFilter filter)
{
  switch (filter)
    {
    case GTHREE_FILTER_NEAREST:
    case GTHREE_FILTER_NEAREST_MIPMAP_NEAREST:
    case GTHREE_FILTER_NEAREST_MIPMAP_LINEAR:
      return GL_NEAREST;

    default:
      return GL_LINEAR;
    }
}

static void
set_texture_parameters (guint texture_type, GthreeTexture *texture, gboolean is_image_power_of_two)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  if (is_image_power_of_two)
    {
      glTexParameteri (texture_type, GL_TEXTURE_WRAP_S, wrap_to_gl (priv->wrap_s));
      glTexParameteri (texture_type, GL_TEXTURE_WRAP_T, wrap_to_gl (priv->wrap_t));
      glTexParameteri (texture_type, GL_TEXTURE_MAG_FILTER, filter_to_gl (priv->mag_filter));
      glTexParameteri (texture_type, GL_TEXTURE_MIN_FILTER, filter_to_gl (priv->min_filter ) );
    }
  else
    {
      glTexParameteri( texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameteri( texture_type, GL_TEXTURE_MAG_FILTER, filter_fallback (priv->mag_filter));
      glTexParameteri( texture_type, GL_TEXTURE_MIN_FILTER, filter_fallback (priv->min_filter));
    }

#if TODO
  if ( _glExtensionTextureFilterAnisotropic && texture.type !== THREE.FloatType ) {
    if ( texture.anisotropy > 1 || texture.__oldAnisotropy ) {
      glTexParameterf( texture_type, _glExtensionTextureFilterAnisotropic.TEXTURE_MAX_ANISOTROPY_EXT, Math.min( texture.anisotropy, _maxAnisotropy ) );
      texture.__oldAnisotropy = texture.anisotropy;
    }
  }
#endif
}

static gboolean
is_power_of_two (guint value)
{
  return value != 0 && (value & (value - 1)) == 0;
}

void
gthree_texture_load (GthreeTexture *texture, int slot)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  if (!priv->gl_texture)
    glGenTextures(1, &priv->gl_texture);

  glActiveTexture (GL_TEXTURE0 + slot);
  glBindTexture (GL_TEXTURE_2D, priv->gl_texture);

  if (priv->needs_update)
    {
      guint width = gdk_pixbuf_get_width (priv->pixbuf);
      guint height = gdk_pixbuf_get_height (priv->pixbuf);
      guint gl_format, gl_type;
      gboolean is_image_power_of_two = is_power_of_two (width) && is_power_of_two (height);

      //glPixelStorei( GL_UNPACK_FLIP_Y_WEBGL, texture.flipY );
      //glPixelStorei( GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, texture.premultiplyAlpha );
      glPixelStorei (GL_UNPACK_ALIGNMENT, priv->unpack_alignment);

      gl_format = gdk_pixbuf_get_has_alpha (priv->pixbuf) ? GL_RGBA : GL_RGB;
      gl_type = priv->type;

      set_texture_parameters (GL_TEXTURE_2D, texture, is_image_power_of_two);

      //var mipmap, mipmaps = texture.mipmaps;

#if TODO
      if ( texture instanceof THREE.DataTexture )
        {
          // use manually created mipmaps if available
          // if there are no manual mipmaps
          // set 0 level mipmap and then use GL to generate other mipmap levels
          if ( mipmaps.length > 0 && isImagePowerOfTwo )
            {
              for ( var i = 0, il = mipmaps.length; i < il; i ++ ) {
                mipmap = mipmaps[ i ];
                _gl.texImage2D( _gl.TEXTURE_2D, i, glFormat, mipmap.width, mipmap.height, 0, glFormat, glType, mipmap.data );
              }
              texture.generateMipmaps = false;
            }
          else
            {
              _gl.texImage2D( _gl.TEXTURE_2D, 0, glFormat, image.width, image.height, 0, glFormat, glType, image.data );
            }
        }
      else if ( texture instanceof THREE.CompressedTexture )
        {
          for ( var i = 0, il = mipmaps.length; i < il; i ++ )
            {
              mipmap = mipmaps[ i ];
              if ( texture.format !== THREE.RGBAFormat )
                {
                  _gl.compressedTexImage2D( _gl.TEXTURE_2D, i, glFormat, mipmap.width, mipmap.height, 0, mipmap.data );
                }
              else
                {
                  _gl.texImage2D( _gl.TEXTURE_2D, i, glFormat, mipmap.width, mipmap.height, 0, glFormat, glType, mipmap.data );
                }
            }
        }
      else
#endif
        {
          // regular Texture (image, video, canvas)

          // use manually created mipmaps if available
          // if there are no manual mipmaps
          // set 0 level mipmap and then use GL to generate other mipmap levels

#ifdef TODO
          if ( mipmaps.length > 0 && isImagePowerOfTwo )
            {
              for ( var i = 0, il = mipmaps.length; i < il; i ++ ) {
                mipmap = mipmaps[ i ];
                _gl.texImage2D( _gl.TEXTURE_2D, i, glFormat, glFormat, glType, mipmap );
              }
              texture.generateMipmaps = false;
            }
          else
#endif
            {
              glTexImage2D (GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, gl_type,
                            gdk_pixbuf_get_pixels (priv->pixbuf));
            }
        }

      if (priv->generate_mipmaps && is_image_power_of_two)
        glGenerateMipmap (GL_TEXTURE_2D);

      priv->needs_update = FALSE;
    }
}
