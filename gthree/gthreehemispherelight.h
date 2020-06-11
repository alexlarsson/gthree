#ifndef __GTHREE_HEMISPHERE_LIGHT_H__
#define __GTHREE_HEMISPHERE_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreelight.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_HEMISPHERE_LIGHT      (gthree_hemisphere_light_get_type ())
#define GTHREE_HEMISPHERE_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                       GTHREE_TYPE_HEMISPHERE_LIGHT, \
                                                                       GthreeHemisphereLight))
#define GTHREE_IS_HEMISPHERE_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                       GTHREE_TYPE_HEMISPHERE_LIGHT))

struct _GthreeHemisphereLight {
  GthreeLight parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeHemisphereLight, g_object_unref)

typedef struct {
  GthreeLightClass parent_class;

} GthreeHemisphereLightClass;

GTHREE_API
GthreeHemisphereLight *gthree_hemisphere_light_new (const graphene_vec3_t *sky_color,
                                                    const graphene_vec3_t *ground_color,
                                                    float                  intensity);
GTHREE_API
GType gthree_hemisphere_light_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_HEMISPHERELIGHT_H__ */
