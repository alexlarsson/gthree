#ifndef __GTHREE_CUBE_TEXTURE_H__
#define __GTHREE_CUBE_TEXTURE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_CUBE_TEXTURE      (gthree_cube_texture_get_type ())
#define GTHREE_CUBE_TEXTURE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
								   GTHREE_TYPE_CUBE_TEXTURE, \
								   GthreeCubeTexture))
#define GTHREE_IS_CUBE_TEXTURE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
								   GTHREE_TYPE_CUBE_TEXTURE))

struct _GthreeCubeTexture {
  GthreeTexture parent;
};

typedef struct {
  GthreeTextureClass parent_class;

} GthreeCubeTextureClass;

GthreeCubeTexture *gthree_cube_texture_new (GdkPixbuf *px,
					    GdkPixbuf *nx,
					    GdkPixbuf *py,
					    GdkPixbuf *ny,
					    GdkPixbuf *pz,
					    GdkPixbuf *nz);
GthreeCubeTexture *gthree_cube_texture_new_from_array (GdkPixbuf *pixbufs[6]);

GType gthree_cube_texture_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_CUBETEXTURE_H__ */
