#ifndef __GTHREE_LIGHT_H__
#define __GTHREE_LIGHT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gtk/gtk.h>

#include "gthreeobject.h"
#include "gthreeprogram.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_LIGHT      (gthree_light_get_type ())
#define GTHREE_LIGHT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_LIGHT, \
                                                             GthreeLight))
#define GTHREE_IS_LIGHT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_LIGHT))

typedef struct {
  GthreeObject parent;
} GthreeLight;

typedef struct {
  GthreeObjectClass parent_class;

  void          (*set_params) (GthreeLight *light,
                               GthreeProgramParameters *params);
  
} GthreeLightClass;

GType gthree_light_get_type (void) G_GNUC_CONST;

GthreeLight *gthree_light_new ();

gboolean gthree_light_get_is_only_shadow (GthreeLight *light);
gboolean gthree_light_get_casts_shadow (GthreeLight *light);

void     gthree_light_set_params (GthreeLight       *light,
				  GthreeProgramParameters *params);

G_END_DECLS

#endif /* __GTHREE_LIGHT_H__ */
