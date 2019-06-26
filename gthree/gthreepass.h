#ifndef __GTHREE_PASS_H__
#define __GTHREE_PASS_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreerendertarget.h>
#include <gthree/gthreeshader.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_PASS      (gthree_pass_get_type ())
#define GTHREE_PASS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_PASS, \
                                                  GthreePass))
#define GTHREE_PASS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_PASS, GthreePassClass))
#define GTHREE_IS_PASS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_PASS))
#define GTHREE_PASS_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_PASS, GthreePassClass))


typedef struct {
  GObject parent;

  // if set to true, the pass is processed by the composer
  gboolean enabled;

  // if set to true, the pass indicates to swap read and write buffer after rendering
  gboolean need_swap;

  // if set to true, the pass clears its buffer before rendering
  gboolean clear;

  // if set to true, the result of the pass is rendered to screen. This is set automatically by the composer.
  gboolean render_to_screen;
} GthreePass;

typedef struct {
  GObjectClass parent_class;

  void (*set_size) (GthreePass *pass,
                    int width,
                    int height);
  void (*render) (GthreePass *pass,
                  GthreeRenderer *renderer,
                  GthreeRenderTarget *write_buffer,
                  GthreeRenderTarget *read_buffer,
                  float delta_time,
                  gboolean mask_active);
} GthreePassClass;


GType gthree_pass_get_type (void) G_GNUC_CONST;

void gthree_pass_set_enabled (GthreePass         *pass,
                              gboolean            enabled);
void gthree_pass_set_clear   (GthreePass         *pass,
                              gboolean            clear);
void gthree_pass_resize      (GthreePass         *pass,
                              int                 width,
                              int                 height);
void gthree_pass_render      (GthreePass         *pass,
                              GthreeRenderer     *renderer,
                              GthreeRenderTarget *write_buffer,
                              GthreeRenderTarget *read_buffer,
                              float               delta_time,
                              gboolean            mask_active);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePass, g_object_unref)

typedef struct _GthreeFullscreenQuadPass GthreeFullscreenQuadPass;

#define GTHREE_TYPE_FULLSCREEN_QUAD_PASS      (gthree_fullscreen_quad_pass_get_type ())
#define GTHREE_FULLSCREEN_QUAD_PASS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_FULLSCREEN_QUAD_PASS, \
                                                  GthreeFullscreenQuadPass))
#define GTHREE_FULLSCREEN_QUAD_PASS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_PASS, GthreeFullscreenQuadPassClass))
#define GTHREE_IS_FULLSCREEN_QUAD_PASS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_FULLSCREEN_QUAD_PASS))
#define GTHREE_FULLSCREEN_QUAD_PASS_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_FULLSCREEN_QUAD_PASS, GthreeFullscreenQuadPassClass))

GType gthree_fullscreen_quad_pass_get_type (void) G_GNUC_CONST;

GthreePass *gthree_fullscreen_quad_pass_new  (GthreeMaterial *material);
void        gthree_fullscreen_quad_pass_set_material (GthreeFullscreenQuadPass *pass,
                                                      GthreeMaterial *material);

#define GTHREE_TYPE_EFFECT_COMPOSER      (gthree_effect_composer_get_type ())
#define GTHREE_EFFECT_COMPOSER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_EFFECT_COMPOSER, \
                                                  GthreeEffectComposer))
#define GTHREE_EFFECT_COMPOSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_EFFECT_COMPOSER, GthreeEffectComposerClass))
#define GTHREE_IS_EFFECT_COMPOSER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_EFFECT_COMPOSER))
#define GTHREE_EFFECT_COMPOSER_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_EFFECT_COMPOSER, GthreeEffectComposerClass))

typedef struct _GthreeShaderPass GthreeShaderPass;

#define GTHREE_TYPE_SHADER_PASS      (gthree_shader_pass_get_type ())
#define GTHREE_SHADER_PASS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_SHADER_PASS, \
                                                  GthreeShaderPass))
#define GTHREE_SHADER_PASS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_PASS, GthreeShaderPassClass))
#define GTHREE_IS_SHADER_PASS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_SHADER_PASS))
#define GTHREE_SHADER_PASS_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_SHADER_PASS, GthreeShaderPassClass))

GType gthree_shader_pass_get_type (void) G_GNUC_CONST;

GthreePass *gthree_shader_pass_new  (GthreeShader *shader, const char *texture_id);

typedef struct _GthreeRenderPass GthreeRenderPass;

#define GTHREE_TYPE_RENDER_PASS      (gthree_render_pass_get_type ())
#define GTHREE_RENDER_PASS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_RENDER_PASS, \
                                                  GthreeRenderPass))
#define GTHREE_RENDER_PASS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_PASS, GthreeRenderPassClass))
#define GTHREE_IS_RENDER_PASS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_RENDER_PASS))
#define GTHREE_RENDER_PASS_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_RENDER_PASS, GthreeRenderPassClass))

GType gthree_render_pass_get_type (void) G_GNUC_CONST;

GthreePass *gthree_render_pass_new  (GthreeScene *scene,
                                     GthreeCamera *camera,
                                     GthreeMaterial *override_material);

void gthree_render_pass_set_clear_depth (GthreeRenderPass *render_pass,
                                         gboolean          clear_depth);


typedef struct _GthreeClearPass GthreeClearPass;

#define GTHREE_TYPE_CLEAR_PASS      (gthree_clear_pass_get_type ())
#define GTHREE_CLEAR_PASS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_CLEAR_PASS, \
                                                  GthreeClearPass))
#define GTHREE_CLEAR_PASS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_PASS, GthreeClearPassClass))
#define GTHREE_IS_CLEAR_PASS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_CLEAR_PASS))
#define GTHREE_CLEAR_PASS_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_CLEAR_PASS, GthreeClearPassClass))

GType gthree_clear_pass_get_type (void) G_GNUC_CONST;

GthreePass *gthree_clear_pass_new  (const GdkRGBA *color);
void gthree_clear_pass_set_clear_depth (GthreeClearPass *clear_pass,
                                        gboolean clear_depth);

G_END_DECLS

#endif /* __GTHREE_PASS_H__ */
