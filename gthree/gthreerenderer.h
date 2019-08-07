#ifndef __GTHREE_RENDERER_H__
#define __GTHREE_RENDERER_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreescene.h>
#include <gthree/gthreecamera.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreerendertarget.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_RENDERER      (gthree_renderer_get_type ())
#define GTHREE_RENDERER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                            GTHREE_TYPE_RENDERER, \
                                                            GthreeRenderer))
#define GTHREE_IS_RENDERER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                            GTHREE_TYPE_RENDERER))

struct _GthreeRenderer {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;

} GthreeRendererClass;

GTHREE_API
GthreeRenderer *gthree_renderer_new ();
GTHREE_API
GType gthree_renderer_get_type (void) G_GNUC_CONST;

GTHREE_API
void                gthree_renderer_set_size                  (GthreeRenderer     *renderer,
                                                               int                 width,
                                                               int                 height);
GTHREE_API
int                 gthree_renderer_get_width                 (GthreeRenderer     *renderer);
GTHREE_API
int                 gthree_renderer_get_height                (GthreeRenderer     *renderer);
GTHREE_API
int                 gthree_renderer_get_drawing_buffer_width  (GthreeRenderer     *renderer);
GTHREE_API
int                 gthree_renderer_get_drawing_buffer_height (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_autoclear             (GthreeRenderer     *renderer,
                                                               gboolean            auto_clear);
GTHREE_API
gboolean            gthree_renderer_get_autoclear             (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_autoclear_color       (GthreeRenderer     *renderer,
                                                               gboolean            clear_color);
GTHREE_API
gboolean            gthree_renderer_get_autoclear_color       (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_autoclear_depth       (GthreeRenderer     *renderer,
                                                               gboolean            clear_depth);
GTHREE_API
gboolean            gthree_renderer_get_autoclear_depth       (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_autoclear_stencil     (GthreeRenderer     *renderer,
                                                               gboolean            clear_stencil);
GTHREE_API
gboolean            gthree_renderer_get_autoclear_stencil     (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_clear_color           (GthreeRenderer     *renderer,
                                                               GdkRGBA            *color);
GTHREE_API
const GdkRGBA      *gthree_renderer_get_clear_color           (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_gamma_factor          (GthreeRenderer     *renderer,
                                                               float               factor);
GTHREE_API
float               gthree_renderer_get_gamma_factor          (GthreeRenderer     *renderer);
GTHREE_API
int                 gthree_renderer_get_n_clipping_planes     (GthreeRenderer     *renderer);
GTHREE_API
const graphene_plane_t *gthree_renderer_get_clipping_plane    (GthreeRenderer     *renderer,
                                                               int                 index);
GTHREE_API
void                gthree_renderer_set_clipping_plane        (GthreeRenderer     *renderer,
                                                               int                 index,
                                                               const graphene_plane_t *plane);
GTHREE_API
void                gthree_renderer_add_clipping_plane        (GthreeRenderer     *renderer,
                                                               const graphene_plane_t *plane);
GTHREE_API
void                gthree_renderer_remove_clipping_plane     (GthreeRenderer     *renderer,
                                                               int                 index);
GTHREE_API
void                gthree_renderer_remove_all_clipping_planes(GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_set_render_target         (GthreeRenderer     *renderer,
                                                               GthreeRenderTarget *target,
                                                               int                 active_cube_target,
                                                               int                 active_mipmap_level);
GTHREE_API
GthreeRenderTarget *gthree_renderer_get_render_target         (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_clear                     (GthreeRenderer     *renderer,
                                                               gboolean            color,
                                                               gboolean            depth,
                                                               gboolean            stencil);
GTHREE_API
void                gthree_renderer_clear_depth               (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_clear_color               (GthreeRenderer     *renderer);
GTHREE_API
void                gthree_renderer_render                    (GthreeRenderer     *renderer,
                                                               GthreeScene        *scene,
                                                               GthreeCamera       *camera);


G_END_DECLS

#endif /* __GTHREE_RENDERER_H__ */
