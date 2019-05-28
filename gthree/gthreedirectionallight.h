#ifndef __GTHREE_DIRECTIONAL_LIGHT_H__
#define __GTHREE_DIRECTIONAL_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreelight.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_DIRECTIONAL_LIGHT      (gthree_directional_light_get_type ())
#define GTHREE_DIRECTIONAL_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_DIRECTIONAL_LIGHT, \
                                                                     GthreeDirectionalLight))
#define GTHREE_IS_DIRECTIONAL_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_DIRECTIONAL_LIGHT))

struct _GthreeDirectionalLight {
  GthreeLight parent;
};

typedef struct {
  GthreeLightClass parent_class;

} GthreeDirectionalLightClass;

GType gthree_directional_light_get_type (void) G_GNUC_CONST;

GthreeDirectionalLight *gthree_directional_light_new (const GdkRGBA *color,
                                                      float intensity);

void          gthree_directional_light_set_target    (GthreeDirectionalLight *directional,
                                                      GthreeObject           *target);
GthreeObject *gthree_directional_light_get_target    (GthreeDirectionalLight *directional);


G_END_DECLS

#endif /* __GTHREE_DIRECTIONALLIGHT_H__ */
