#include <epoxy/gl.h>

#include "gthreerawtexture.h"
#include "gthreeprivate.h"

typedef struct {
  GLenum target;
  GLenum internal_format;
  GLenum format;
  int width;
  int height;
  int depth;
  float *data;
} GthreeRawTexturePrivate;

typedef struct {
  GthreeResourceRealizeData parent;
  guint gl_texture;
} GthreeRawTextureRealizeData;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeRawTexture, gthree_raw_texture, GTHREE_TYPE_RESOURCE)

static void
gthree_raw_texture_finalize (GObject *obj)
{
  GthreeRawTexture *texture = GTHREE_RAW_TEXTURE (obj);
  GthreeRawTexturePrivate *priv = gthree_raw_texture_get_instance_private (texture);

  g_free (priv->data);

  G_OBJECT_CLASS (gthree_raw_texture_parent_class)->finalize (obj);
}

static void
gthree_raw_texture_real_unrealize (GthreeResource *resource,
                                  GthreeRenderer *renderer)
{
  GthreeRawTextureRealizeData *data = gthree_resource_get_data_for (resource, renderer);

  if (data->gl_texture != 0)
    {
      gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_TEXTURE, data->gl_texture);
      data->gl_texture = 0;
    }
}

static void
gthree_raw_texture_init (GthreeRawTexture *texture)
{
}

static void
gthree_raw_texture_class_init (GthreeRawTextureClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeResourceClass *resource_class = GTHREE_RESOURCE_CLASS (klass);

  gobject_class->finalize = gthree_raw_texture_finalize;
  resource_class->realize_data_size = sizeof (GthreeRawTextureRealizeData);
  resource_class->unrealize = gthree_raw_texture_real_unrealize;
}

static GthreeRawTexture *
gthree_raw_texture_new (GLenum target,
                        int    width,
                        int    height,
                        int    depth,
                        GLenum internal_format,
                        GLenum format,
                        const float *data)
{
  GthreeRawTexture *texture;
  GthreeRawTexturePrivate *priv;
  int data_size;

  texture = g_object_new (GTHREE_TYPE_RAW_TEXTURE, NULL);
  priv = gthree_raw_texture_get_instance_private (texture);

  priv->target = target;
  priv->width = width;
  priv->height = height;
  priv->depth = depth;
  priv->internal_format = internal_format;
  priv->format = format;

  int components;
  if (format == GL_RGBA)
    components = 4;
  else if (format == GL_RED)
    components = 1;
  else
    components = 4;

  data_size = width * height * MAX (depth, 1) * components;

  if (data)
    {
      priv->data = g_memdup2 (data, data_size * sizeof (float));
    }
  else
    {
      priv->data = g_new0 (float, data_size);
    }

  return texture;
}

GthreeRawTexture *
gthree_raw_texture_new_2d (int    width,
                           int    height,
                           GLenum internal_format,
                           GLenum format,
                           const float *data)
{
  return gthree_raw_texture_new (GL_TEXTURE_2D, width, height, 0,
                                internal_format, format, data);
}

GthreeRawTexture *
gthree_raw_texture_new_2d_array (int    width,
                                 int    height,
                                 int    depth,
                                 GLenum internal_format,
                                 GLenum format,
                                 const float *data)
{
  return gthree_raw_texture_new (GL_TEXTURE_2D_ARRAY, width, height, depth,
                                internal_format, format, data);
}

void
gthree_raw_texture_set_data (GthreeRawTexture *texture,
                             int    width,
                             int    height,
                             int    depth,
                             const float *data)
{
  GthreeRawTexturePrivate *priv = gthree_raw_texture_get_instance_private (texture);
  int components;

  if (priv->format == GL_RGBA)
    components = 4;
  else if (priv->format == GL_RED)
    components = 1;
  else
    components = 4;

  int data_size = width * height * MAX (depth, 1) * components;

  priv->width = width;
  priv->height = height;
  priv->depth = depth;

  g_free (priv->data);
  priv->data = g_memdup2 (data, data_size * sizeof (float));

  gthree_resource_mark_dirty (GTHREE_RESOURCE (texture));
}

static void
upload_texture (GthreeRawTexturePrivate *priv, GLenum target)
{
  if (target == GL_TEXTURE_2D_ARRAY)
    {
      glTexImage3D (GL_TEXTURE_2D_ARRAY, 0, priv->internal_format,
                    priv->width, priv->height, priv->depth,
                    0, priv->format, GL_FLOAT, priv->data);
    }
  else
    {
      glTexImage2D (GL_TEXTURE_2D, 0, priv->internal_format,
                    priv->width, priv->height,
                    0, priv->format, GL_FLOAT, priv->data);
    }
}

guint
gthree_raw_texture_realize (GthreeRawTexture *texture,
                            GthreeRenderer   *renderer)
{
  GthreeRawTexturePrivate *priv = gthree_raw_texture_get_instance_private (texture);
  GthreeRawTextureRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (texture), renderer);

  if (!gthree_resource_is_realized_for (GTHREE_RESOURCE (texture), renderer))
    {
      gthree_resource_set_realized_for (GTHREE_RESOURCE (texture), renderer);

      glGenTextures (1, &data->gl_texture);
      glBindTexture (priv->target, data->gl_texture);
      upload_texture (priv, priv->target);
      glTexParameteri (priv->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri (priv->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri (priv->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri (priv->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glBindTexture (priv->target, 0);

      gthree_resource_mark_clean_for (GTHREE_RESOURCE (texture), renderer);
    }
  else if (gthree_resource_get_dirty_for (GTHREE_RESOURCE (texture), renderer))
    {
      glBindTexture (priv->target, data->gl_texture);
      upload_texture (priv, priv->target);
      glBindTexture (priv->target, 0);

      gthree_resource_mark_clean_for (GTHREE_RESOURCE (texture), renderer);
    }

  return data->gl_texture;
}
