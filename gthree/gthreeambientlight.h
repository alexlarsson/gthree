#ifndef __GTHREE_AMBIENT_LIGHT_H__
#define __GTHREE_AMBIENT_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreelight.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_AMBIENT_LIGHT      (gthree_ambient_light_get_type ())
#define GTHREE_AMBIENT_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_AMBIENT_LIGHT, \
                                                                     GthreeAmbientLight))
#define GTHREE_IS_AMBIENT_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_AMBIENT_LIGHT))

struct _GthreeAmbientLight {
  GthreeLight parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAmbientLight, g_object_unref)

typedef struct {
  GthreeLightClass parent_class;

} GthreeAmbientLightClass;

GTHREE_API
GthreeAmbientLight *gthree_ambient_light_new (const graphene_vec3_t *color);
GTHREE_API
GType gthree_ambient_light_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_AMBIENTLIGHT_H__ */
