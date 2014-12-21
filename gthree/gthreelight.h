#ifndef __GTHREE_LIGHT_H__
#define __GTHREE_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreeprogram.h>
#include <gdk/gdk.h>

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

typedef struct {
  GthreeObjectClass parent_class;

  void          (*set_params) (GthreeLight *light,
                               GthreeProgramParameters *params);
  
  void          (*setup) (GthreeLight *light,
			  GthreeLightSetup *light_setup);
} GthreeLightClass;

GType gthree_light_get_type (void) G_GNUC_CONST;

GthreeLight *gthree_light_new (void);

gboolean       gthree_light_get_is_visible     (GthreeLight   *light);
void           gthree_light_set_is_visible     (GthreeLight   *light,
						gboolean       visible);
gboolean       gthree_light_get_is_only_shadow (GthreeLight   *light);
void           gthree_light_set_is_only_shadow (GthreeLight   *light,
                                                gboolean       only_shadow);
gboolean       gthree_light_get_casts_shadow   (GthreeLight   *light);
void           gthree_light_set_casts_shadow   (GthreeLight   *light,
                                                gboolean       casts_shadow);
const GdkRGBA *gthree_light_get_color          (GthreeLight   *light);
void           gthree_light_set_color          (GthreeLight   *light,
						const GdkRGBA *color);

void           gthree_light_set_params         (GthreeLight   *light,
                                                GthreeProgramParameters *params);
void           gthree_light_setup              (GthreeLight   *light,
                                                GthreeLightSetup *setup);

G_END_DECLS

#endif /* __GTHREE_LIGHT_H__ */
