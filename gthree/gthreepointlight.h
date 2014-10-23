#ifndef __GTHREE_POINT_LIGHT_H__
#define __GTHREE_POINT_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include "gthreelight.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_POINT_LIGHT      (gthree_point_light_get_type ())
#define GTHREE_POINT_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_POINT_LIGHT, \
                                                                     GthreePointLight))
#define GTHREE_IS_POINT_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_POINT_LIGHT))

struct _GthreePointLight {
  GthreeLight parent;
};

typedef struct {
  GthreeLightClass parent_class;

} GthreePointLightClass;

GthreePointLight *gthree_point_light_new (const GdkRGBA *color,
					  float intensity,
					  float distance);
GType gthree_point_light_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_POINTLIGHT_H__ */
