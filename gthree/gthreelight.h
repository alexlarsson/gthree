#ifndef __GTHREE_LIGHT_H__
#define __GTHREE_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_LIGHT      (gthree_light_get_type ())
#define GTHREE_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_LIGHT, GthreeLight))
#define GTHREE_LIGHT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_LIGHT, GthreeLightClass))
#define GTHREE_IS_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_LIGHT))
#define GTHREE_IS_LIGHT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_LIGHT))
#define GTHREE_LIGHT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_LIGHT, GthreeLightClass))

typedef struct {
  GthreeObject parent;
} GthreeLight;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeLight, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

  void          (*setup) (GthreeLight *light,
                          GthreeCamera *camera,
                          GthreeLightSetup *light_setup);

  gpointer padding[8];
} GthreeLightClass;

GTHREE_API
GType gthree_light_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeLight *gthree_light_new (void);

GTHREE_API
const graphene_vec3_t *gthree_light_get_color          (GthreeLight   *light);
GTHREE_API
void                   gthree_light_set_color          (GthreeLight   *light,
                                                        const graphene_vec3_t *color);
GTHREE_API
float                  gthree_light_get_intensity      (GthreeLight   *light);
GTHREE_API
void                   gthree_light_set_intensity      (GthreeLight   *light,
                                                        float          intensity);
GTHREE_API
GthreeLightShadow     *gthree_light_get_shadow         (GthreeLight   *light);

G_END_DECLS

#endif /* __GTHREE_LIGHT_H__ */
