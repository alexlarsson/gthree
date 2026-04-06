#ifndef __GTHREE_SKY_H__
#define __GTHREE_SKY_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemesh.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_SKY      (gthree_sky_get_type ())
#define GTHREE_SKY(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                          GTHREE_TYPE_SKY, \
                                                          GthreeSky))
#define GTHREE_IS_SKY(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                          GTHREE_TYPE_SKY))

typedef struct {
  GthreeMesh parent;
} GthreeSky;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSky, g_object_unref)

typedef struct {
  GthreeMeshClass parent_class;
} GthreeSkyClass;

GTHREE_API
GType gthree_sky_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeSky *gthree_sky_new (void);

GTHREE_API
void  gthree_sky_set_turbidity        (GthreeSky             *sky,
                                       float                  turbidity);
GTHREE_API
float gthree_sky_get_turbidity        (GthreeSky             *sky);

GTHREE_API
void  gthree_sky_set_rayleigh         (GthreeSky             *sky,
                                       float                  rayleigh);
GTHREE_API
float gthree_sky_get_rayleigh         (GthreeSky             *sky);

GTHREE_API
void  gthree_sky_set_mie_coefficient  (GthreeSky             *sky,
                                       float                  mie_coefficient);
GTHREE_API
float gthree_sky_get_mie_coefficient  (GthreeSky             *sky);

GTHREE_API
void  gthree_sky_set_mie_directional_g (GthreeSky            *sky,
                                        float                 mie_directional_g);
GTHREE_API
float gthree_sky_get_mie_directional_g (GthreeSky            *sky);

GTHREE_API
void  gthree_sky_set_sun_position     (GthreeSky             *sky,
                                       const graphene_vec3_t  *sun_position);
GTHREE_API
const graphene_vec3_t *gthree_sky_get_sun_position (GthreeSky *sky);

GTHREE_API
void  gthree_sky_set_up               (GthreeSky             *sky,
                                       const graphene_vec3_t  *up);

G_END_DECLS

#endif /* __GTHREE_SKY_H__ */
