#ifndef __GTHREE_TEXTURE_H__
#define __GTHREE_TEXTURE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <graphene.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_TEXTURE      (gthree_texture_get_type ())
#define GTHREE_TEXTURE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_TEXTURE, \
                                                             GthreeTexture))
#define GTHREE_IS_TEXTURE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_TEXTURE))


typedef struct {
  GObject parent;
} GthreeTexture;

typedef struct {
  GObjectClass parent_class;

} GthreeTextureClass;

GType gthree_texture_get_type (void) G_GNUC_CONST;

GthreeTexture *gthree_texture_new (GdkPixbuf *pixbuf);

const graphene_vec2_t *gthree_texture_get_repeat (GthreeTexture *texture);
const graphene_vec2_t *gthree_texture_get_offset (GthreeTexture *texture);

G_END_DECLS

#endif /* __GTHREE_TEXTURE_H__ */
