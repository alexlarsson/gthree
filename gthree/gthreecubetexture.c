#include <math.h>
#include <epoxy/gl.h>

#include "gthreecubetexture.h"
#include "gthreeprivate.h"

enum {
  GTHREE_CUBE_FACE_PX,
  GTHREE_CUBE_FACE_NX,
  GTHREE_CUBE_FACE_PY,
  GTHREE_CUBE_FACE_NY,
  GTHREE_CUBE_FACE_PZ,
  GTHREE_CUBE_FACE_NZ
};

typedef struct {
  GdkPixbuf *pixbufs[6];
} GthreeCubeTexturePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeCubeTexture, gthree_cube_texture, GTHREE_TYPE_TEXTURE);

GthreeCubeTexture *
gthree_cube_texture_new (GdkPixbuf *px,
                         GdkPixbuf *nx,
                         GdkPixbuf *py,
                         GdkPixbuf *ny,
                         GdkPixbuf *pz,
                         GdkPixbuf *nz)
{
  GthreeCubeTexture *cube;
  GthreeCubeTexturePrivate *priv;
  int i = 0;

  cube = g_object_new (gthree_cube_texture_get_type (),
                           NULL);

  priv = gthree_cube_texture_get_instance_private (cube);

  priv->pixbufs[i++] = g_object_ref (px);
  priv->pixbufs[i++] = g_object_ref (nx);
  priv->pixbufs[i++] = g_object_ref (py);
  priv->pixbufs[i++] = g_object_ref (ny);
  priv->pixbufs[i++] = g_object_ref (pz);
  priv->pixbufs[i++] = g_object_ref (nz);

  return cube;
}

GthreeCubeTexture *
gthree_cube_texture_new_from_array (GdkPixbuf *pixbufs[6])
{
  return gthree_cube_texture_new (pixbufs[0], pixbufs[1], pixbufs[2], pixbufs[3], pixbufs[4], pixbufs[5]);
}

static void
gthree_cube_texture_init (GthreeCubeTexture *cube)
{
}

static gboolean
is_power_of_two (guint value)
{
  return value != 0 && (value & (value - 1)) == 0;
}

static void
gthree_cube_texture_real_load (GthreeTexture *texture, GthreeRenderer *renderer, int slot)
{
  GthreeCubeTexture *cube = GTHREE_CUBE_TEXTURE (texture);
  GthreeCubeTexturePrivate *priv = gthree_cube_texture_get_instance_private (cube);
  int i;
  //int max_cubemap_size;
  GdkPixbuf *cube_pixbufs[6];
  //gboolean autoScaleCubemaps = TRUE; // TODO: Pass from renderer

  gthree_texture_bind (texture, renderer, slot, GL_TEXTURE_CUBE_MAP);

  if (gthree_texture_get_needs_update (texture))
    {
      guint width, height;
      gboolean is_compressed = FALSE; //texture instanceof THREE.CompressedTexture;
      guint gl_format, gl_type;
      gboolean is_image_power_of_two = is_power_of_two (width) && is_power_of_two (height);

      for (i = 0; i < 6; i++)
        {
#ifdef TODO
          if ( autoScaleCubemaps && ! is_compressed )
            {
              glGetIntegerv (GL_MAX_CUBE_MAP_TEXTURE_SIZE, &max_cubemap_size);
              cubeImage[ i ] = clampToMaxSize( texture.image[ i ], max_cubemap_size);
            }
          else
#endif
            cube_pixbufs[i] = g_object_ref (priv->pixbufs[i]);
        }

      width = gdk_pixbuf_get_width (cube_pixbufs[0]);
      height = gdk_pixbuf_get_height (cube_pixbufs[0]);
      is_image_power_of_two = is_power_of_two (width) && is_power_of_two (height);

      gl_format = gdk_pixbuf_get_has_alpha (cube_pixbufs[0]) ? GL_RGBA : GL_RGB;
      gl_type = GL_UNSIGNED_BYTE;

      gthree_texture_set_parameters (GL_TEXTURE_CUBE_MAP, texture, is_image_power_of_two);

      for (i = 0; i < 6; i++)
        {
          if (!is_compressed)
            {
              glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl_format, width, height, 0, gl_format, gl_type,
                            gdk_pixbuf_get_pixels (cube_pixbufs[i]));
            }
#ifdef TODO
          else
            {
              var mipmap, mipmaps = cubeImage[ i ].mipmaps;
              for ( var j = 0, jl = mipmaps.length; j < jl; j ++ )
                {
                  mipmap = mipmaps[ j ];
                  if ( texture.format !== THREE.RGBAFormat )
                    {
                      _gl.compressedTexImage2D( _gl.TEXTURE_CUBE_MAP_POSITIVE_X + i, j, glFormat, mipmap.width, mipmap.height, 0, mipmap.data );
                    }
                  else
                    {
                      _gl.texImage2D( _gl.TEXTURE_CUBE_MAP_POSITIVE_X + i, j, glFormat, mipmap.width, mipmap.height, 0, glFormat, glType, mipmap.data );
                    }
                }
            }
#endif
        }

      if (gthree_texture_get_generate_mipmaps (texture) && is_image_power_of_two)
        {
          glGenerateMipmap (GL_TEXTURE_CUBE_MAP);
          gthree_texture_set_max_mip_level (texture, log2 (MAX (width, height)));
        }

      gthree_texture_set_needs_update (texture, FALSE);
    }
}

static void
gthree_cube_texture_finalize (GObject *obj)
{
  GthreeCubeTexture *cube = GTHREE_CUBE_TEXTURE (obj);
  GthreeCubeTexturePrivate *priv = gthree_cube_texture_get_instance_private (cube);
  int i;

  for (i = 0; i < 6; i++)
    g_clear_object (&priv->pixbufs[i]);

  G_OBJECT_CLASS (gthree_cube_texture_parent_class)->finalize (obj);
}

static void
gthree_cube_texture_class_init (GthreeCubeTextureClass *klass)
{
  GTHREE_TEXTURE_CLASS (klass)->load = gthree_cube_texture_real_load;
  G_OBJECT_CLASS (klass)->finalize = gthree_cube_texture_finalize;
}
