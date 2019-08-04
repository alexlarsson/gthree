#ifndef __GTHREE_EFFECT_COMPOSER_H__
#define __GTHREE_EFFECT_COMPOSER_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreepass.h>

G_BEGIN_DECLS


typedef struct {
  GObject parent;
} GthreeEffectComposer;

typedef struct {
  GObjectClass parent_class;
} GthreeEffectComposerClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeEffectComposer, g_object_unref)

GTHREE_API
GType gthree_effect_composer_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeEffectComposer *gthree_effect_composer_new  (void);

GTHREE_API
void gthree_effect_composer_add_pass     (GthreeEffectComposer *composer,
                                          GthreePass     *pass);
GTHREE_API
void gthree_effect_composer_render       (GthreeEffectComposer *composer,
                                          GthreeRenderer       *renderer,
                                          float                 delta_time);
GTHREE_API
void gthree_effect_composer_reset        (GthreeEffectComposer *composer,
                                          GthreeRenderer       *renderer,
                                          GthreeRenderTarget   *render_target);
GTHREE_API
void gthree_effect_composer_set_size     (GthreeEffectComposer *composer,
                                          int                   width,
                                          int                   height);

G_END_DECLS

#endif /* __GTHREE_EFFECT_COMPOSER_H__ */
