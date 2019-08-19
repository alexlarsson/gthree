#ifndef __GTHREE_DIRECTIONAL_LIGHT_SHADOW_H__
#define __GTHREE_DIRECTIONAL_LIGHT_SHADOW_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreelightshadow.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_DIRECTIONAL_LIGHT_SHADOW      (gthree_directional_light_get_type ())
#define GTHREE_DIRECTIONAL_LIGHT_SHADOW(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_DIRECTIONAL_LIGHT_SHADOW, \
                                                                     GthreeDirectionalLight))
#define GTHREE_IS_DIRECTIONAL_LIGHT_SHADOW(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_DIRECTIONAL_LIGHT_SHADOW))

typedef struct _GthreeDirectionalLightShadow {
  GthreeLightShadow parent;
} GthreeDirectionalLightShadow;

typedef struct {
  GthreeLightShadowClass parent_class;

} GthreeDirectionalLightShadowClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeDirectionalLightShadow, g_object_unref)

GTHREE_API
GType gthree_directional_light_shadow_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_DIRECTIONALLIGHT_H__ */
