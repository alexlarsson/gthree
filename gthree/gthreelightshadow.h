#ifndef __GTHREE_LIGHT_SHADOW_H__
#define __GTHREE_LIGHT_SHADOW_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <graphene.h>

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_LIGHT_SHADOW      (gthree_light_shadow_get_type ())
#define GTHREE_LIGHT_SHADOW(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                               GTHREE_TYPE_LIGHT_SHADOW, \
                                                               GthreeLightShadow))
#define GTHREE_IS_LIGHT_SHADOW(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                               GTHREE_TYPE_LIGHT_SHADOW))

struct _GthreeLightShadow {
  GObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeLightShadow, g_object_unref)

typedef struct {
  GObjectClass parent_class;

} GthreeLightShadowClass;

GTHREE_API
GType gthree_light_shadow_get_type (void) G_GNUC_CONST;

GTHREE_API
int gthree_light_shadow_get_map_width (GthreeLightShadow *shadow);
GTHREE_API
int gthree_light_shadow_get_map_height (GthreeLightShadow *shadow);
GTHREE_API
void gthree_light_shadow_set_map_size (GthreeLightShadow *shadow,
                                       int width,
                                       int height);
GTHREE_API
GthreeCamera * gthree_light_shadow_get_camera (GthreeLightShadow *shadow);
GTHREE_API
float gthree_light_shadow_get_bias (GthreeLightShadow *shadow);
GTHREE_API
void gthree_light_shadow_set_bias (GthreeLightShadow *shadow,
                                   float bias);
GTHREE_API
float gthree_light_shadow_get_radius (GthreeLightShadow *shadow);
GTHREE_API
void gthree_light_shadow_set_radius (GthreeLightShadow *shadow,
                                     float radiuso);


G_END_DECLS

#endif /* __GTHREE_LIGHT_SHADOW_H__ */
