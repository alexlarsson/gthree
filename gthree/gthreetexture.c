#include <math.h>
#include <epoxy/gl.h>
#include <epoxy/egl.h>

#ifndef DRM_FORMAT_MOD_INVALID
#define DRM_FORMAT_MOD_INVALID ((1ULL << 56) - 1)
#endif

#include "gthreetexture.h"
#include "gthreeprivate.h"
#include "gthreeenums.h"
#include "gthreetypebuiltins.h"

enum
{
  LAST_SIGNAL
};

/*static guint texture_signals[LAST_SIGNAL] = { 0, };*/

enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_MEMORY_FORMAT,
  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

static void gthree_texture_real_load (GthreeTexture *texture,
                                      GthreeRenderer *renderer,
                                      int slot);
static void gthree_texture_real_unrealize (GthreeResource *resource,
                                           GthreeRenderer *renderer);

typedef struct {
  const guchar *data;
  GDestroyNotify data_destroy;
  gpointer data_destroy_data;
  int width;
  int height;
  gsize stride;
  GthreeMemoryFormat memory_format;

  char *name;
  char *uuid;
  GArray *mipmaps;
  GthreeMapping mapping;
  GthreeWrapping wrap_s;
  GthreeWrapping wrap_t;
  GthreeEncodingFormat encoding;

  GthreeFilter mag_filter;
  GthreeFilter min_filter;

  int anisotropy;

  graphene_vec2_t offset;
  graphene_vec2_t repeat;

  gboolean generate_mipmaps;
  gboolean premultiply_alpha;
  GthreeTextureDataFlags flags;
  int unpack_alignment;

  guint max_mip_level;

  guint pmrem_version;

  int channel;
} GthreeTexturePrivate;

typedef struct {
  GthreeResourceRealizeData parent;
  guint gl_texture;
} GthreeTextureRealizeData;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeTexture, gthree_texture, GTHREE_TYPE_RESOURCE)

int
gthree_memory_format_bytes_per_pixel (GthreeMemoryFormat format)
{
  switch (format)
    {
    case GTHREE_MEMORY_FORMAT_R8:
      return 1;
    case GTHREE_MEMORY_FORMAT_R8G8:
      return 2;
    case GTHREE_MEMORY_FORMAT_R8G8B8:
      return 3;
    case GTHREE_MEMORY_FORMAT_R8G8B8A8:
    case GTHREE_MEMORY_FORMAT_B8G8R8A8:
    case GTHREE_MEMORY_FORMAT_R8G8B8A8_PREMULTIPLIED:
    case GTHREE_MEMORY_FORMAT_B8G8R8A8_PREMULTIPLIED:
    case GTHREE_MEMORY_FORMAT_A8R8G8B8_PREMULTIPLIED:
      return 4;
    case GTHREE_MEMORY_FORMAT_R16G16B16A16_FLOAT:
      return 8;
    case GTHREE_MEMORY_FORMAT_R32G32B32A32_FLOAT:
      return 16;
    default:
      g_assert_not_reached ();
      return 4;
    }
}

void
gthree_memory_format_to_gl (GthreeMemoryFormat format,
                             GthreeGLFormatInfo *info)
{
  switch (format)
    {
    case GTHREE_MEMORY_FORMAT_R8G8B8A8:
    case GTHREE_MEMORY_FORMAT_R8G8B8A8_PREMULTIPLIED:
      info->gl_format = GL_RGBA;
      info->gl_type = GL_UNSIGNED_BYTE;
      info->gl_internal_format = GL_RGBA8;
      info->gl_internal_format_srgb = GL_SRGB8_ALPHA8;
      break;
    case GTHREE_MEMORY_FORMAT_R8G8B8:
      info->gl_format = GL_RGB;
      info->gl_type = GL_UNSIGNED_BYTE;
      info->gl_internal_format = GL_RGB8;
      info->gl_internal_format_srgb = GL_SRGB8;
      break;
    case GTHREE_MEMORY_FORMAT_B8G8R8A8:
    case GTHREE_MEMORY_FORMAT_B8G8R8A8_PREMULTIPLIED:
    case GTHREE_MEMORY_FORMAT_A8R8G8B8_PREMULTIPLIED:
      if (epoxy_is_desktop_gl ())
        {
          info->gl_format = GL_BGRA;
          info->gl_type = GL_UNSIGNED_INT_8_8_8_8_REV;
        }
      else
        {
          info->gl_format = GL_RGBA;
          info->gl_type = GL_UNSIGNED_BYTE;
        }
      info->gl_internal_format = GL_RGBA8;
      info->gl_internal_format_srgb = GL_SRGB8_ALPHA8;
      break;
    case GTHREE_MEMORY_FORMAT_R16G16B16A16_FLOAT:
      info->gl_format = GL_RGBA;
      info->gl_type = GL_HALF_FLOAT;
      info->gl_internal_format = GL_RGBA16F;
      info->gl_internal_format_srgb = 0;
      break;
    case GTHREE_MEMORY_FORMAT_R32G32B32A32_FLOAT:
      info->gl_format = GL_RGBA;
      info->gl_type = GL_FLOAT;
      info->gl_internal_format = GL_RGBA32F;
      info->gl_internal_format_srgb = 0;
      break;
    case GTHREE_MEMORY_FORMAT_R8:
      info->gl_format = GL_RED;
      info->gl_type = GL_UNSIGNED_BYTE;
      info->gl_internal_format = GL_R8;
      info->gl_internal_format_srgb = 0;
      break;
    case GTHREE_MEMORY_FORMAT_R8G8:
      info->gl_format = GL_RG;
      info->gl_type = GL_UNSIGNED_BYTE;
      info->gl_internal_format = GL_RG8;
      info->gl_internal_format_srgb = 0;
      break;
    default:
      g_assert_not_reached ();
      info->gl_format = GL_RGBA;
      info->gl_type = GL_UNSIGNED_BYTE;
      info->gl_internal_format = GL_RGBA8;
      info->gl_internal_format_srgb = GL_SRGB8_ALPHA8;
      break;
    }
}

static gboolean
memory_format_is_bgra (GthreeMemoryFormat format)
{
  return format == GTHREE_MEMORY_FORMAT_B8G8R8A8 ||
         format == GTHREE_MEMORY_FORMAT_B8G8R8A8_PREMULTIPLIED ||
         format == GTHREE_MEMORY_FORMAT_A8R8G8B8_PREMULTIPLIED;
}

gboolean
gthree_memory_format_needs_bgra_swizzle (GthreeMemoryFormat format)
{
  return memory_format_is_bgra (format) && !epoxy_is_desktop_gl ();
}

static void
gthree_texture_clear_data (GthreeTexturePrivate *priv)
{
  if (priv->data_destroy)
    priv->data_destroy (priv->data_destroy_data);
  priv->data = NULL;
  priv->data_destroy = NULL;
  priv->data_destroy_data = NULL;
}

static void
gthree_texture_notify_data_props (GthreeTexture      *texture,
                                  int                 old_width,
                                  int                 old_height,
                                  GthreeMemoryFormat  old_format)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  if (priv->width != old_width)
    g_object_notify_by_pspec (G_OBJECT (texture), obj_props[PROP_WIDTH]);
  if (priv->height != old_height)
    g_object_notify_by_pspec (G_OBJECT (texture), obj_props[PROP_HEIGHT]);
  if (priv->memory_format != old_format)
    g_object_notify_by_pspec (G_OBJECT (texture), obj_props[PROP_MEMORY_FORMAT]);
}

static void
gthree_texture_set_data (GthreeTexturePrivate *priv,
                         const guchar         *data,
                         int                   width,
                         int                   height,
                         gsize                 stride,
                         GthreeMemoryFormat    format,
                         GDestroyNotify        destroy,
                         gpointer              destroy_data)
{
  gthree_texture_clear_data (priv);
  priv->data = data;
  priv->width = width;
  priv->height = height;
  priv->stride = stride;
  priv->memory_format = format;
  priv->data_destroy = destroy;
  priv->data_destroy_data = destroy_data;
}

static void
gthree_texture_preprocess_data (GthreeTexturePrivate *priv)
{
  int bpp;
  gsize tight_stride;
  guchar *tmp;

  if ((priv->flags & GTHREE_TEXTURE_DATA_KEEP_LIVE) || !priv->data)
    return;

  if (!(priv->flags & GTHREE_TEXTURE_DATA_FLIP_Y))
    return;

  bpp = gthree_memory_format_bytes_per_pixel (priv->memory_format);
  tight_stride = priv->width * bpp;
  tmp = g_malloc (priv->height * tight_stride);

  for (int y = 0; y < priv->height; y++)
    memcpy (tmp + y * tight_stride,
            priv->data + (priv->height - 1 - y) * priv->stride,
            tight_stride);

  gthree_texture_clear_data (priv);
  priv->data = tmp;
  priv->stride = tight_stride;
  priv->data_destroy = g_free;
  priv->data_destroy_data = tmp;
}

GthreeTexture *
gthree_texture_new_from_memory (const guchar           *data,
                                int                     width,
                                int                     height,
                                gsize                   stride,
                                GthreeMemoryFormat      format,
                                GthreeTextureDataFlags  flags,
                                GDestroyNotify          destroy,
                                gpointer                user_data)
{
  GthreeTexture *texture = g_object_new (gthree_texture_get_type (), NULL);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->flags = flags;
  gthree_texture_set_data (priv, data, width, height, stride, format,
                           destroy, user_data);
  gthree_texture_preprocess_data (priv);

  return texture;
}

GthreeTexture *
gthree_texture_new_from_bytes (GBytes                 *bytes,
                               int                     width,
                               int                     height,
                               gsize                   stride,
                               GthreeMemoryFormat      format,
                               GthreeTextureDataFlags  flags)
{
  GthreeTexture *texture = g_object_new (gthree_texture_get_type (), NULL);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->flags = flags;
  if (bytes)
    gthree_texture_set_data (priv,
                             g_bytes_get_data (bytes, NULL),
                             width, height, stride, format,
                             (GDestroyNotify) g_bytes_unref, g_bytes_ref (bytes));
  gthree_texture_preprocess_data (priv);

  return texture;
}

static GthreeMemoryFormat
cairo_surface_memory_format (void)
{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  return GTHREE_MEMORY_FORMAT_B8G8R8A8_PREMULTIPLIED;
#else
  return GTHREE_MEMORY_FORMAT_A8R8G8B8_PREMULTIPLIED;
#endif
}

GthreeTexture *
gthree_texture_new_from_surface (cairo_surface_t *surface)
{
  GthreeTexture *texture;
  GthreeTexturePrivate *priv;

  g_return_val_if_fail (surface != NULL, NULL);
  g_return_val_if_fail (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE, NULL);

  cairo_surface_flush (surface);

  texture = g_object_new (gthree_texture_get_type (), NULL);
  priv = gthree_texture_get_instance_private (texture);

  priv->flags = GTHREE_TEXTURE_DATA_KEEP_LIVE;
  gthree_texture_set_data (priv,
                           cairo_image_surface_get_data (surface),
                           cairo_image_surface_get_width (surface),
                           cairo_image_surface_get_height (surface),
                           cairo_image_surface_get_stride (surface),
                           cairo_surface_memory_format (),
                           (GDestroyNotify) cairo_surface_destroy,
                           cairo_surface_reference (surface));
  gthree_texture_preprocess_data (priv);
  return texture;
}

void
gthree_texture_set_from_surface (GthreeTexture   *texture,
                                 cairo_surface_t *surface)
{
  GthreeTexturePrivate *priv;

  g_return_if_fail (GTHREE_IS_TEXTURE (texture));
  g_return_if_fail (surface != NULL);
  g_return_if_fail (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE);

  cairo_surface_flush (surface);

  priv = gthree_texture_get_instance_private (texture);
  {
    int old_width = priv->width;
    int old_height = priv->height;
    GthreeMemoryFormat old_format = priv->memory_format;

    gthree_texture_set_data (priv,
                             cairo_image_surface_get_data (surface),
                             cairo_image_surface_get_width (surface),
                             cairo_image_surface_get_height (surface),
                             cairo_image_surface_get_stride (surface),
                             cairo_surface_memory_format (),
                             (GDestroyNotify) cairo_surface_destroy,
                             cairo_surface_reference (surface));
    gthree_texture_preprocess_data (priv);
    gthree_texture_notify_data_props (texture, old_width, old_height, old_format);
  }
  gthree_texture_set_needs_update (texture);
}

static GthreeMemoryFormat
pixbuf_memory_format (GdkPixbuf *pixbuf)
{
  return gdk_pixbuf_get_has_alpha (pixbuf)
    ? GTHREE_MEMORY_FORMAT_R8G8B8A8
    : GTHREE_MEMORY_FORMAT_R8G8B8;
}

GthreeTexture *
gthree_texture_new_from_pixbuf (GdkPixbuf *pixbuf)
{
  GthreeTexture *texture;
  GthreeTexturePrivate *priv;

  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

  texture = g_object_new (gthree_texture_get_type (), NULL);
  priv = gthree_texture_get_instance_private (texture);

  priv->flags = GTHREE_TEXTURE_DATA_FLIP_Y;
  gthree_texture_set_data (priv,
                           gdk_pixbuf_get_pixels (pixbuf),
                           gdk_pixbuf_get_width (pixbuf),
                           gdk_pixbuf_get_height (pixbuf),
                           gdk_pixbuf_get_rowstride (pixbuf),
                           pixbuf_memory_format (pixbuf),
                           g_object_unref, g_object_ref (pixbuf));
  gthree_texture_preprocess_data (priv);
  return texture;
}

void
gthree_texture_set_from_pixbuf (GthreeTexture *texture,
                                GdkPixbuf     *pixbuf)
{
  GthreeTexturePrivate *priv;

  g_return_if_fail (GTHREE_IS_TEXTURE (texture));
  g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

  priv = gthree_texture_get_instance_private (texture);
  {
    int old_width = priv->width;
    int old_height = priv->height;
    GthreeMemoryFormat old_format = priv->memory_format;

    gthree_texture_set_data (priv,
                             gdk_pixbuf_get_pixels (pixbuf),
                             gdk_pixbuf_get_width (pixbuf),
                             gdk_pixbuf_get_height (pixbuf),
                             gdk_pixbuf_get_rowstride (pixbuf),
                             pixbuf_memory_format (pixbuf),
                             g_object_unref, g_object_ref (pixbuf));
    gthree_texture_preprocess_data (priv);
    gthree_texture_notify_data_props (texture, old_width, old_height, old_format);
  }
  gthree_texture_set_needs_update (texture);
}

GthreeTexture *
gthree_texture_new_empty (int                 width,
                          int                 height,
                          GthreeMemoryFormat  format)
{
  GthreeTexture *texture = g_object_new (gthree_texture_get_type (), NULL);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->width = width;
  priv->height = height;
  priv->memory_format = format;

  return texture;
}

static void
gthree_texture_init (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->uuid = g_uuid_string_random ();

  priv->anisotropy = 1;
  priv->unpack_alignment = 4;
  priv->generate_mipmaps = TRUE;
  priv->wrap_s = GTHREE_WRAPPING_CLAMP;
  priv->wrap_t = GTHREE_WRAPPING_CLAMP;
  priv->mag_filter = GTHREE_FILTER_LINEAR;
  priv->min_filter = GTHREE_FILTER_LINEAR_MIPMAP_LINEAR;
  priv->mapping = GTHREE_MAPPING_UV;
  priv->encoding = GTHREE_ENCODING_FORMAT_LINEAR;

  priv->memory_format = GTHREE_MEMORY_FORMAT_R8G8B8A8;

  graphene_vec2_init (&priv->offset, 0, 0);
  graphene_vec2_init (&priv->repeat, 1, 1);
}

void
gthree_texture_copy_settings (GthreeTexture        *texture,
                              GthreeTexture        *source)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  GthreeTexturePrivate *source_priv = gthree_texture_get_instance_private (source);

  priv->mapping = source_priv->mapping;
  priv->wrap_s = source_priv->wrap_s;
  priv->wrap_t = source_priv->wrap_t;
  priv->encoding = source_priv->encoding;
  priv->memory_format = source_priv->memory_format;
  priv->min_filter = source_priv->min_filter;
  priv->mag_filter = source_priv->mag_filter;
  priv->anisotropy = source_priv->anisotropy;
  priv->offset = source_priv->offset;
  priv->repeat = source_priv->repeat;
  priv->generate_mipmaps = source_priv->generate_mipmaps;
  priv->premultiply_alpha = source_priv->premultiply_alpha;
  priv->unpack_alignment = source_priv->unpack_alignment;
}

static void
gthree_texture_get_property (GObject    *obj,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GthreeTexture *texture = GTHREE_TEXTURE (obj);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  switch (prop_id)
    {
    case PROP_WIDTH:
      g_value_set_int (value, priv->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, priv->height);
      break;
    case PROP_MEMORY_FORMAT:
      g_value_set_enum (value, priv->memory_format);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_texture_finalize (GObject *obj)
{
  GthreeTexture *texture = GTHREE_TEXTURE (obj);
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  g_free (priv->uuid);
  g_free (priv->name);
  gthree_texture_clear_data (priv);

  G_OBJECT_CLASS (gthree_texture_parent_class)->finalize (obj);
}

static void
gthree_texture_class_init (GthreeTextureClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeResourceClass *resource_class = GTHREE_RESOURCE_CLASS (klass);

  resource_class->realize_data_size = sizeof (GthreeTextureRealizeData);

  gobject_class->finalize = gthree_texture_finalize;
  gobject_class->get_property = gthree_texture_get_property;

  resource_class->unrealize = gthree_texture_real_unrealize;

  klass->load = gthree_texture_real_load;

  obj_props[PROP_WIDTH] =
    g_param_spec_int ("width", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_HEIGHT] =
    g_param_spec_int ("height", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MEMORY_FORMAT] =
    g_param_spec_enum ("memory-format", NULL, NULL,
                       GTHREE_TYPE_MEMORY_FORMAT,
                       GTHREE_MEMORY_FORMAT_R8G8B8A8,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

void
gthree_texture_set_from_bytes (GthreeTexture      *texture,
                          GBytes             *bytes,
                          int                 width,
                          int                 height,
                          gsize               stride,
                          GthreeMemoryFormat  format)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  int old_width = priv->width;
  int old_height = priv->height;
  GthreeMemoryFormat old_format = priv->memory_format;

  if (bytes)
    {
      gthree_texture_set_data (priv,
                               g_bytes_get_data (bytes, NULL),
                               width, height, stride, format,
                               (GDestroyNotify) g_bytes_unref, g_bytes_ref (bytes));
      gthree_texture_preprocess_data (priv);
    }
  else
    {
      gthree_texture_clear_data (priv);
      priv->width = width;
      priv->height = height;
      priv->stride = stride;
      priv->memory_format = format;
    }

  gthree_texture_notify_data_props (texture, old_width, old_height, old_format);
  gthree_texture_set_needs_update (texture);
}

int
gthree_texture_get_width (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  return priv->width;
}

int
gthree_texture_get_height (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  return priv->height;
}

GthreeMemoryFormat
gthree_texture_get_memory_format (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  return priv->memory_format;
}

void
gthree_texture_set_needs_update (GthreeTexture *texture)
{
  gthree_resource_mark_dirty (GTHREE_RESOURCE (texture));
}

void
gthree_texture_set_needs_pmrem_update (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  priv->pmrem_version++;
}

guint
gthree_texture_get_pmrem_version (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  return priv->pmrem_version;
}

void
gthree_texture_set_mapping (GthreeTexture *texture,
                            GthreeMapping mapping)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->mapping = mapping;
}

GthreeMapping
gthree_texture_get_mapping (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->mapping;
}


void
gthree_texture_set_wrap_s (GthreeTexture *texture,
                           GthreeWrapping wrap_s)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->wrap_s = wrap_s;
}

GthreeWrapping
gthree_texture_get_wrap_s (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->wrap_s;
}

void
gthree_texture_set_wrap_t (GthreeTexture *texture,
                           GthreeWrapping wrap_t)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->wrap_t = wrap_t;
}

GthreeWrapping
gthree_texture_get_wrap_t (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->wrap_t;
}

void
gthree_texture_set_mag_filter (GthreeTexture *texture,
                               GthreeFilter   mag_filter)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->mag_filter = mag_filter;
}

GthreeFilter
gthree_texture_get_mag_filter (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->mag_filter;
}

void
gthree_texture_set_min_filter (GthreeTexture *texture,
                               GthreeFilter   min_filter)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->min_filter = min_filter;
}

GthreeFilter
gthree_texture_get_min_filter (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->min_filter;
}

gboolean
gthree_texture_get_generate_mipmaps (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->generate_mipmaps;
}

void
gthree_texture_set_generate_mipmaps (GthreeTexture *texture,
                                     gboolean generate_mipmaps)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->generate_mipmaps = generate_mipmaps;
}

void
gthree_texture_set_repeat (GthreeTexture *texture,
                           const graphene_vec2_t *repeat)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->repeat = *repeat;
}

const graphene_vec2_t *
gthree_texture_get_repeat (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return &priv->repeat;
}

void
gthree_texture_set_offset (GthreeTexture *texture,
                           const graphene_vec2_t *offset)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->offset = *offset;
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

void
gthree_texture_set_parameters (guint texture_type, GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  glTexParameteri (texture_type, GL_TEXTURE_WRAP_S, wrap_to_gl (priv->wrap_s));
  glTexParameteri (texture_type, GL_TEXTURE_WRAP_T, wrap_to_gl (priv->wrap_t));
  glTexParameteri (texture_type, GL_TEXTURE_MAG_FILTER, filter_to_gl (priv->mag_filter));
  glTexParameteri (texture_type, GL_TEXTURE_MIN_FILTER, filter_to_gl (priv->min_filter));
}

static void
gthree_texture_real_unrealize (GthreeResource *resource,
                               GthreeRenderer *renderer)
{
  GthreeTextureRealizeData *data = gthree_resource_get_data_for (resource, renderer);

  g_assert (data->gl_texture != 0);

  gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_TEXTURE, data->gl_texture);

  data->gl_texture = 0;
}

void
gthree_texture_realize (GthreeTexture *texture, GthreeRenderer *renderer)
{
  GthreeTextureRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (texture), renderer);

  if (!data->gl_texture)
    {
      gthree_resource_set_realized_for (GTHREE_RESOURCE (texture), renderer);

      glGenTextures (1, &data->gl_texture);
#ifdef DEBUG_LABELS
      {
        GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

        if (priv->name)
          glObjectLabel (GL_TEXTURE, data->gl_texture, strlen (priv->name), priv->name);
      }
#endif
    }
}

void
gthree_texture_set_gl_texture (GthreeTexture *texture, GthreeRenderer *renderer, guint gl_texture)
{
  GthreeTextureRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (texture), renderer);

  if (data->gl_texture && data->gl_texture != gl_texture)
    gthree_renderer_lazy_delete (renderer, GTHREE_RESOURCE_KIND_TEXTURE, data->gl_texture);

  if (!data->parent.realized_for)
    gthree_resource_set_realized_for (GTHREE_RESOURCE (texture), renderer);

  data->gl_texture = gl_texture;
}

gboolean
gthree_texture_set_from_dmabuf (GthreeTexture  *texture,
                                GthreeRenderer *renderer,
                                int             fd,
                                uint32_t        fourcc,
                                uint64_t        modifier,
                                int             width,
                                int             height,
                                uint32_t        offset,
                                uint32_t        stride,
                                GError        **error)
{
  if (!epoxy_has_egl ())
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "DMABUF import requires EGL support");
      return FALSE;
    }

  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  EGLDisplay egl_display = eglGetCurrentDisplay ();

  if (egl_display == EGL_NO_DISPLAY)
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "No EGL display available");
      return FALSE;
    }

  if (!epoxy_has_egl_extension (egl_display, "EGL_EXT_image_dma_buf_import"))
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "EGL_EXT_image_dma_buf_import not available");
      return FALSE;
    }

  EGLAttrib attribs[17];
  int i = 0;

  attribs[i++] = EGL_WIDTH;
  attribs[i++] = width;
  attribs[i++] = EGL_HEIGHT;
  attribs[i++] = height;
  attribs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
  attribs[i++] = fourcc;
  attribs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
  attribs[i++] = fd;
  attribs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
  attribs[i++] = offset;
  attribs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
  attribs[i++] = stride;

  if (modifier != DRM_FORMAT_MOD_INVALID &&
      epoxy_has_egl_extension (egl_display, "EGL_EXT_image_dma_buf_import_modifiers"))
    {
      attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
      attribs[i++] = (EGLAttrib)(modifier & 0xFFFFFFFF);
      attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
      attribs[i++] = (EGLAttrib)(modifier >> 32);
    }

  attribs[i++] = EGL_NONE;

  EGLImage image = eglCreateImage (egl_display, EGL_NO_CONTEXT,
                                   EGL_LINUX_DMA_BUF_EXT,
                                   NULL, attribs);
  if (image == EGL_NO_IMAGE)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "eglCreateImage failed (EGL error 0x%x)",
                   eglGetError ());
      return FALSE;
    }

  GLuint gl_tex;
  glGenTextures (1, &gl_tex);
  glBindTexture (GL_TEXTURE_2D, gl_tex);
  glEGLImageTargetTexture2DOES (GL_TEXTURE_2D, image);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  eglDestroyImage (egl_display, image);

  gthree_texture_set_gl_texture (texture, renderer, gl_tex);

  int old_width = priv->width;
  int old_height = priv->height;
  priv->width = width;
  priv->height = height;
  gthree_texture_notify_data_props (texture, old_width, old_height, priv->memory_format);

  return TRUE;
}

void
gthree_texture_bind (GthreeTexture *texture, GthreeRenderer *renderer, int slot, int target)
{
  GthreeTextureRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (texture), renderer);

  if (!data->gl_texture)
    gthree_texture_realize (texture, renderer);

  if (slot >= 0)
    glActiveTexture (GL_TEXTURE0 + slot);
  glBindTexture (target, data->gl_texture);
}

void
gthree_texture_setup_framebuffer (GthreeTexture *texture,
                                  GthreeRenderer *renderer,
                                  int width,
                                  int height,
                                  guint framebuffer,
                                  int attachment,
                                  int texture_target)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  GthreeGLFormatInfo gl_info;
  GthreeTextureRealizeData *data = gthree_resource_get_data_for (GTHREE_RESOURCE (texture), renderer);

  gthree_memory_format_to_gl (priv->memory_format, &gl_info);

  glTexImage2D (texture_target, 0, gl_info.gl_internal_format,
                width, height, 0, gl_info.gl_format, gl_info.gl_type, 0);
  glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture2D (GL_FRAMEBUFFER, attachment, texture_target,
                          data->gl_texture, 0);
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

static int
gthree_texture_get_gl_target (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  switch (priv->mapping)
    {
    case GTHREE_MAPPING_CUBE_REFLECTION:
    case GTHREE_MAPPING_CUBE_REFRACTION:
      return GL_TEXTURE_CUBE_MAP;
    default:
      return GL_TEXTURE_2D;
    }
}

void
gthree_swizzle_bgra_to_rgba (guchar *dst, const guchar *src, guint width, guint height, gsize stride, gboolean flip_y)
{
  for (guint y = 0; y < height; y++)
    {
      guint src_y = flip_y ? (height - 1 - y) : y;
      const guchar *row = src + src_y * stride;
      guchar *dst_row = dst + y * width * 4;
      for (guint x = 0; x < width; x++)
        {
          dst_row[x*4+0] = row[x*4+2];
          dst_row[x*4+1] = row[x*4+1];
          dst_row[x*4+2] = row[x*4+0];
          dst_row[x*4+3] = row[x*4+3];
        }
    }
}

static void
gthree_texture_real_load (GthreeTexture *texture, GthreeRenderer *renderer, int slot)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);
  int gl_target = gthree_texture_get_gl_target (texture);

  gthree_texture_bind (texture, renderer, slot, gl_target);

  if (gthree_resource_get_dirty_for (GTHREE_RESOURCE (texture), renderer) && priv->data)
    {
      GthreeGLFormatInfo gl_info;
      guint gl_internal_format;
      const guchar *data;
      guchar *tmp = NULL;
      gboolean needs_swizzle;
      int bpp;

      gthree_memory_format_to_gl (priv->memory_format, &gl_info);

      gl_internal_format = gl_info.gl_internal_format;
      if (priv->encoding == GTHREE_ENCODING_FORMAT_SRGB && gl_info.gl_internal_format_srgb)
        gl_internal_format = gl_info.gl_internal_format_srgb;

      glPixelStorei (GL_UNPACK_ALIGNMENT, priv->unpack_alignment);

      bpp = gthree_memory_format_bytes_per_pixel (priv->memory_format);

      if (priv->stride != (gsize)(priv->width * bpp))
        glPixelStorei (GL_UNPACK_ROW_LENGTH, priv->stride / bpp);

      gthree_texture_set_parameters (GL_TEXTURE_2D, texture);

      data = priv->data;
      needs_swizzle = gthree_memory_format_needs_bgra_swizzle (priv->memory_format);

      gboolean needs_flip = (priv->flags & GTHREE_TEXTURE_DATA_FLIP_Y) &&
                             (priv->flags & GTHREE_TEXTURE_DATA_KEEP_LIVE);

      if (needs_swizzle)
        {
          tmp = g_malloc (priv->width * priv->height * 4);
          gthree_swizzle_bgra_to_rgba (tmp, data, priv->width, priv->height, priv->stride, needs_flip);
          data = tmp;
          if (priv->stride != (gsize)(priv->width * bpp))
            glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
        }
      else if (needs_flip)
        {
          tmp = g_malloc (priv->height * priv->stride);
          for (int y = 0; y < priv->height; y++)
            memcpy (tmp + y * priv->stride,
                    data + (priv->height - 1 - y) * priv->stride,
                    priv->stride);
          data = tmp;
        }

      glTexImage2D (GL_TEXTURE_2D, 0, gl_internal_format,
                    priv->width, priv->height, 0,
                    gl_info.gl_format, gl_info.gl_type, data);

      g_free (tmp);

      if (priv->stride != (gsize)(priv->width * bpp))
        glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

      if (priv->generate_mipmaps)
        {
          glGenerateMipmap (GL_TEXTURE_2D);
          gthree_texture_set_max_mip_level (texture, log2 (MAX (priv->width, priv->height)));
        }

      gthree_resource_mark_clean_for (GTHREE_RESOURCE (texture), renderer);
    }
}

void
gthree_texture_load (GthreeTexture *texture, GthreeRenderer *renderer, int slot)
{
  GthreeTextureClass *class = GTHREE_TEXTURE_GET_CLASS(texture);

  class->load (texture, renderer, slot);
}

void
gthree_texture_set_max_mip_level (GthreeTexture *texture,
                                  int level)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->max_mip_level = level;
}

int
gthree_texture_get_max_mip_level (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->max_mip_level;
}

void
gthree_texture_set_encoding (GthreeTexture *texture,
                             GthreeEncodingFormat encoding)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->encoding = encoding;
}

GthreeEncodingFormat
gthree_texture_get_encoding (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->encoding;
}

void
gthree_texture_set_name (GthreeTexture *texture,
                         const char *name)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  g_free (priv->name);
  priv->name = g_strdup (name);
}

const char *
gthree_texture_get_name (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->name;
}

void
gthree_texture_set_uuid (GthreeTexture *texture,
                         const char *uuid)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  g_free (priv->uuid);
  priv->uuid = g_strdup (uuid);
}

const char *
gthree_texture_get_uuid (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->uuid;
}

void
gthree_texture_set_anisotropy (GthreeTexture *texture,
                               int anisotropy)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->anisotropy = anisotropy;
}

int
gthree_texture_get_anisotropy (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->anisotropy;
}

void
gthree_texture_set_channel (GthreeTexture *texture,
                            int            channel)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  priv->channel = channel;
}

int
gthree_texture_get_channel (GthreeTexture *texture)
{
  GthreeTexturePrivate *priv = gthree_texture_get_instance_private (texture);

  return priv->channel;
}

int
gthree_texture_get_gl_texture (GthreeTexture *texture,
                               GthreeRenderer *renderer)
{
  GthreeTextureRealizeData *data = gthree_resource_get_data_for ((GthreeResource *)texture, renderer);

  return data->gl_texture;
}
