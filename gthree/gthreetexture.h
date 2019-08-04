#ifndef __GTHREE_TEXTURE_H__
#define __GTHREE_TEXTURE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <graphene.h>
#include <gthree/gthreetypes.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeresource.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_TEXTURE      (gthree_texture_get_type ())
#define GTHREE_TEXTURE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_TEXTURE, GthreeTexture))
#define GTHREE_TEXTURE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_TEXTURE, GthreeTextureClass))
#define GTHREE_IS_TEXTURE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_TEXTURE))
#define GTHREE_IS_TEXTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_TEXTURE))
#define GTHREE_TEXTURE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_TEXTURE, GthreeTextureClass))

struct _GthreeTexture {
  GthreeResource parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeTexture, g_object_unref)

typedef struct {
  GthreeResourceClass parent_class;

  void (*load) (GthreeTexture *texture, int slot);
} GthreeTextureClass;

GTHREE_API
GType gthree_texture_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeTexture *gthree_texture_new (GdkPixbuf *pixbuf);

GTHREE_API
const graphene_vec2_t *gthree_texture_get_repeat           (GthreeTexture        *texture);
GTHREE_API
const graphene_vec2_t *gthree_texture_get_offset           (GthreeTexture        *texture);
GTHREE_API
gboolean               gthree_texture_get_generate_mipmaps (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_generate_mipmaps (GthreeTexture        *texture,
                                                            gboolean              generate_mipmaps);
GTHREE_API
void                   gthree_texture_set_mapping          (GthreeTexture        *texture,
                                                            GthreeMapping         mapping);
GTHREE_API
GthreeMapping          gthree_texture_get_mapping          (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_wrap_s           (GthreeTexture        *texture,
                                                            GthreeWrapping        wrap_s);
GTHREE_API
GthreeWrapping         gthree_texture_get_wrap_s           (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_wrap_t           (GthreeTexture        *texture,
                                                            GthreeWrapping        wrap_t);
GTHREE_API
GthreeWrapping         gthree_texture_get_wrap_t           (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_mag_filter       (GthreeTexture        *texture,
                                                            GthreeFilter          mag_filter);
GTHREE_API
GthreeFilter           gthree_texture_get_mag_filter       (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_min_filter       (GthreeTexture        *texture,
                                                            GthreeFilter          min_filter);
GTHREE_API
GthreeFilter           gthree_texture_get_min_filter       (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_flip_y           (GthreeTexture        *texture,
                                                            gboolean              flip_y);
GTHREE_API
gboolean               gthree_texture_get_flip_y           (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_encoding         (GthreeTexture        *texture,
                                                            GthreeEncodingFormat  encoding);
GTHREE_API
GthreeEncodingFormat   gthree_texture_get_encoding         (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_format           (GthreeTexture        *texture,
                                                            GthreeTextureFormat   format);
GTHREE_API
GthreeTextureFormat    gthree_texture_get_format           (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_data_type        (GthreeTexture        *texture,
                                                            GthreeDataType     type);
GTHREE_API
GthreeDataType         gthree_texture_get_data_type        (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_anisotropy       (GthreeTexture        *texture,
                                                            int                   anisotropy);
GTHREE_API
int                    gthree_texture_get_anisotropy       (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_copy_settings        (GthreeTexture        *texture,
                                                            GthreeTexture        *source);
GTHREE_API
void                   gthree_texture_set_name             (GthreeTexture        *texture,
                                                            const char           *name);
GTHREE_API
const char *           gthree_texture_get_name             (GthreeTexture        *texture);
GTHREE_API
void                   gthree_texture_set_uuid             (GthreeTexture        *texture,
                                                            const char           *uuid);
GTHREE_API
const char *           gthree_texture_get_uuid             (GthreeTexture        *texture);
int                    gthree_texture_get_gl_texture       (GthreeTexture        *texture);


G_END_DECLS

#endif /* __GTHREE_TEXTURE_H__ */
