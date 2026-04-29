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
  GBytes *face_bytes[6];
  int size;
  gsize stride;
  GthreeMemoryFormat format;
} GthreeCubeTexturePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeCubeTexture, gthree_cube_texture, GTHREE_TYPE_TEXTURE);

GthreeCubeTexture *
gthree_cube_texture_new_from_bytes (GBytes             *faces[6],
                                    int                 size,
                                    gsize               stride,
                                    GthreeMemoryFormat  format)
{
  GthreeCubeTexture *cube;
  GthreeCubeTexturePrivate *priv;

  cube = g_object_new (gthree_cube_texture_get_type (), NULL);
  priv = gthree_cube_texture_get_instance_private (cube);

  for (int i = 0; i < 6; i++)
    priv->face_bytes[i] = g_bytes_ref (faces[i]);
  priv->size = size;
  priv->stride = stride;
  priv->format = format;

  return cube;
}

int
gthree_cube_texture_get_size (GthreeCubeTexture *cube)
{
  GthreeCubeTexturePrivate *priv = gthree_cube_texture_get_instance_private (cube);
  return priv->size;
}

static void
gthree_cube_texture_init (GthreeCubeTexture *cube)
{
  gthree_texture_set_mapping (GTHREE_TEXTURE (cube), GTHREE_MAPPING_CUBE_REFLECTION);
}

static void
gthree_cube_texture_real_load (GthreeTexture *texture, GthreeRenderer *renderer, int slot)
{
  GthreeCubeTexture *cube = GTHREE_CUBE_TEXTURE (texture);
  GthreeCubeTexturePrivate *priv = gthree_cube_texture_get_instance_private (cube);

  gthree_texture_bind (texture, renderer, slot, GL_TEXTURE_CUBE_MAP);

  if (gthree_resource_get_dirty_for (GTHREE_RESOURCE (texture), renderer))
    {
      GthreeGLFormatInfo gl_info;
      guint gl_internal_format;
      gboolean needs_swizzle;

      gthree_memory_format_to_gl (priv->format, &gl_info);

      gl_internal_format = gl_info.gl_internal_format;
      if (gthree_texture_get_encoding (texture) == GTHREE_ENCODING_FORMAT_SRGB &&
          gl_info.gl_internal_format_srgb)
        gl_internal_format = gl_info.gl_internal_format_srgb;

      needs_swizzle = gthree_memory_format_needs_bgra_swizzle (priv->format);

      gthree_texture_set_parameters (GL_TEXTURE_CUBE_MAP, texture);

      for (int i = 0; i < 6; i++)
        {
          const guchar *data = g_bytes_get_data (priv->face_bytes[i], NULL);
          guchar *swizzled = NULL;

          if (needs_swizzle)
            {
              swizzled = g_malloc (priv->size * priv->size * 4);
              gthree_swizzle_bgra_to_rgba (swizzled, data, priv->size, priv->size, priv->stride, FALSE);
              data = swizzled;
            }

          glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl_internal_format,
                        priv->size, priv->size, 0,
                        gl_info.gl_format, gl_info.gl_type, data);

          g_free (swizzled);
        }

      if (gthree_texture_get_generate_mipmaps (texture))
        {
          glGenerateMipmap (GL_TEXTURE_CUBE_MAP);
          gthree_texture_set_max_mip_level (texture, log2 (priv->size));
        }

      gthree_resource_mark_clean_for (GTHREE_RESOURCE (texture), renderer);
    }
}

static void
gthree_cube_texture_finalize (GObject *obj)
{
  GthreeCubeTexture *cube = GTHREE_CUBE_TEXTURE (obj);
  GthreeCubeTexturePrivate *priv = gthree_cube_texture_get_instance_private (cube);

  for (int i = 0; i < 6; i++)
    g_clear_pointer (&priv->face_bytes[i], g_bytes_unref);

  G_OBJECT_CLASS (gthree_cube_texture_parent_class)->finalize (obj);
}

static void
gthree_cube_texture_class_init (GthreeCubeTextureClass *klass)
{
  GTHREE_TEXTURE_CLASS (klass)->load = gthree_cube_texture_real_load;
  G_OBJECT_CLASS (klass)->finalize = gthree_cube_texture_finalize;
}
