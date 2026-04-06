#ifndef __GTHREE_RAW_TEXTURE_H__
#define __GTHREE_RAW_TEXTURE_H__

#include <glib-object.h>
#include <epoxy/gl.h>
#include "gthreeresource.h"

G_BEGIN_DECLS

#define GTHREE_TYPE_RAW_TEXTURE      (gthree_raw_texture_get_type ())
#define GTHREE_RAW_TEXTURE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_RAW_TEXTURE, GthreeRawTexture))
#define GTHREE_IS_RAW_TEXTURE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_RAW_TEXTURE))

typedef struct _GthreeRawTexture      GthreeRawTexture;
typedef struct _GthreeRawTextureClass GthreeRawTextureClass;

struct _GthreeRawTexture {
  GthreeResource parent;
};

struct _GthreeRawTextureClass {
  GthreeResourceClass parent_class;
};

GType             gthree_raw_texture_get_type    (void) G_GNUC_CONST;

GthreeRawTexture *gthree_raw_texture_new_2d      (int    width,
                                                  int    height,
                                                  GLenum internal_format,
                                                  GLenum format,
                                                  const float *data);
GthreeRawTexture *gthree_raw_texture_new_2d_array (int    width,
                                                   int    height,
                                                   int    depth,
                                                   GLenum internal_format,
                                                   GLenum format,
                                                   const float *data);
void              gthree_raw_texture_set_data     (GthreeRawTexture *texture,
                                                   int    width,
                                                   int    height,
                                                   int    depth,
                                                   const float *data);
guint             gthree_raw_texture_realize      (GthreeRawTexture *texture,
                                                   GthreeRenderer   *renderer);

G_END_DECLS

#endif /* __GTHREE_RAW_TEXTURE_H__ */
