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

typedef struct {
  GthreeResourceClass parent_class;

  void (*load) (GthreeTexture *texture, int slot);
} GthreeTextureClass;

GType gthree_texture_get_type (void) G_GNUC_CONST;

GthreeTexture *gthree_texture_new (GdkPixbuf *pixbuf);

const graphene_vec2_t *gthree_texture_get_repeat           (GthreeTexture *texture);
const graphene_vec2_t *gthree_texture_get_offset           (GthreeTexture *texture);
gboolean               gthree_texture_get_generate_mipmaps (GthreeTexture *texture);
void                   gthree_texture_set_mapping          (GthreeTexture *texture,
                                                            GthreeMapping  mapping);
GthreeMapping          gthree_texture_get_mapping          (GthreeTexture *texture);

G_END_DECLS

#endif /* __GTHREE_TEXTURE_H__ */
