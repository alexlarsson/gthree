#ifndef __GTHREE_LIGHT_H__
#define __GTHREE_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreeprogram.h>
#include <gthree/gthreecamera.h>
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

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeLight, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

  void          (*setup) (GthreeLight *light,
                          GthreeCamera *camera,
                          GthreeLightSetup *light_setup);
} GthreeLightClass;

GType gthree_light_get_type (void) G_GNUC_CONST;

GthreeLight *gthree_light_new (void);

const GdkRGBA *gthree_light_get_color          (GthreeLight   *light);
void           gthree_light_set_color          (GthreeLight   *light,
                                                const GdkRGBA *color);
float          gthree_light_get_intensity      (GthreeLight   *light);
void           gthree_light_set_intensity      (GthreeLight   *light,
                                                float          intensity);

G_END_DECLS

#endif /* __GTHREE_LIGHT_H__ */
