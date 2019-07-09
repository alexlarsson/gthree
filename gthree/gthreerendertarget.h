#ifndef __GTHREE_RENDER_TARGET_H__
#define __GTHREE_RENDER_TARGET_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreetypes.h>
#include <gthree/gthreeresource.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_RENDER_TARGET      (gthree_render_target_get_type ())
#define GTHREE_RENDER_TARGET(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_RENDER_TARGET, \
                                                  GthreeRenderTarget))
#define GTHREE_RENDER_TARGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_RENDER_TARGET, GthreeRenderTargetClass))
#define GTHREE_IS_RENDER_TARGET(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_RENDER_TARGET))
#define GTHREE_RENDER_TARGET_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_RENDER_TARGET, GthreeRenderTargetClass))


typedef struct {
  GthreeResource parent;
} GthreeRenderTarget;

typedef struct {
  GthreeResourceClass parent_class;

} GthreeRenderTargetClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeRenderTarget, g_object_unref)

GTHREE_API
GType gthree_render_target_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeRenderTarget *gthree_render_target_new_full (int width,
                                                   int height,
                                                   GthreeWrapping wrap_t,
                                                   GthreeWrapping wrap_s,
                                                   GthreeFilter mag_filter,
                                                   GthreeFilter min_filter,
                                                   GthreeTextureFormat format,
                                                   GthreeDataType data_type,
                                                   int anisotropy,
                                                   GthreeEncodingFormat encoding,
                                                   gboolean generate_mipmaps,
                                                   gboolean depth_buffer,
                                                   gboolean stencil_buffer,
                                                   GthreeTexture *depth_texture);

GTHREE_API
GthreeRenderTarget *gthree_render_target_new (int width,
                                              int height);
GTHREE_API
GthreeRenderTarget *gthree_render_target_clone (GthreeRenderTarget *target);

GTHREE_API
int            gthree_render_target_get_width         (GthreeRenderTarget *target);
GTHREE_API
int            gthree_render_target_get_height        (GthreeRenderTarget *target);
GTHREE_API
void           gthree_render_target_set_size          (GthreeRenderTarget *target,
                                                       int                 width,
                                                       int                 height);
GTHREE_API
GthreeTexture *gthree_render_target_get_texture       (GthreeRenderTarget *target);
GTHREE_API
gboolean       gthree_render_target_get_depth_buffer  (GthreeRenderTarget *target);
GTHREE_API
void           gthree_render_target_set_depth_buffer  (GthreeRenderTarget *target,
                                                       gboolean            depth_buffer);
GTHREE_API
gboolean       gthree_render_target_get_stencil_buffer  (GthreeRenderTarget *target);
GTHREE_API
void           gthree_render_target_set_stencil_buffer  (GthreeRenderTarget *target,
                                                       gboolean            stencil_buffer);
GTHREE_API
GthreeTexture *gthree_render_target_get_depth_texture (GthreeRenderTarget *target);
GTHREE_API
void           gthree_render_target_set_depth_texture (GthreeRenderTarget *target,
                                                       GthreeTexture *texture);
GTHREE_API
void           gthree_render_target_update_mipmap     (GthreeRenderTarget *target);

G_END_DECLS

#endif /* __GTHREE_RENDER_TARGET_H__ */
