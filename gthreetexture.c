#include <math.h>
#include <epoxy/gl.h>

#include "gthreetexture.h"
#include "gthreeenums.h"

enum
{
  UPDATE,

  LAST_SIGNAL
};

static guint texture_signals[LAST_SIGNAL] = { 0, };

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
  priv->pixbuf = g_object_ref (pixbuf);

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

  graphene_vec2_init (&priv->offset, 0, 0);
  graphene_vec2_init (&priv->repeat, 1, 1);
}

static void
gthree_texture_finalize (GObject *obj)
{
  GthreeTexture *texture = GTHREE_TEXTURE (obj);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  g_clear_object (&priv->pixbuf);

  G_OBJECT_CLASS (gthree_texture_parent_class)->finalize (obj);
}

static void
gthree_texture_class_init (GthreeTextureClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_texture_finalize;

  texture_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GthreeTextureClass, update),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}
