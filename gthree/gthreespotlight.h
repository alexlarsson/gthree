#ifndef __GTHREE_SPOT_LIGHT_H__
#define __GTHREE_SPOT_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreelight.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_SPOT_LIGHT      (gthree_spot_light_get_type ())
#define GTHREE_SPOT_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_SPOT_LIGHT, \
                                                                     GthreeSpotLight))
#define GTHREE_IS_SPOT_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_SPOT_LIGHT))

struct _GthreeSpotLight {
  GthreeLight parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSpotLight, g_object_unref)

typedef struct {
  GthreeLightClass parent_class;

} GthreeSpotLightClass;

GTHREE_API
GType gthree_spot_light_get_type (void) G_GNUC_CONST;
GTHREE_API
GthreeSpotLight *gthree_spot_light_new (const graphene_vec3_t *color,
                                        float intensity,
                                        float distance,
                                        float angle,
                                        float penumbra);

GTHREE_API
void          gthree_spot_light_set_distance (GthreeSpotLight *light,
                                              float            distance);
GTHREE_API
float         gthree_spot_light_get_distance (GthreeSpotLight *light);
GTHREE_API
void          gthree_spot_light_set_decay    (GthreeSpotLight *light,
                                              float            decay);
GTHREE_API
float         gthree_spot_light_get_decay    (GthreeSpotLight *light);
GTHREE_API
void          gthree_spot_light_set_target   (GthreeSpotLight *spot,
                                              GthreeObject    *object);
GTHREE_API
GthreeObject *gthree_spot_light_get_target   (GthreeSpotLight *spot);
GTHREE_API
void          gthree_spot_light_set_angle    (GthreeSpotLight *light,
                                              float            angle);
GTHREE_API
float         gthree_spot_light_get_angle    (GthreeSpotLight *light);
GTHREE_API
void          gthree_spot_light_set_penumbra (GthreeSpotLight *light,
                                              float            penumbra);
GTHREE_API
float         gthree_spot_light_get_penumbra (GthreeSpotLight *light);

G_END_DECLS

#endif /* __GTHREE_SPOTLIGHT_H__ */
