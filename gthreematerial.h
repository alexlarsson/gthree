#ifndef __GTHREE_MATERIAL_H__
#define __GTHREE_MATERIAL_H__

#include <gtk/gtk.h>

#include "gthreeobject.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_MATERIAL      (gthree_material_get_type ())
#define GTHREE_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_MATERIAL, \
                                                             GthreeMaterial))
#define GTHREE_IS_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_MATERIAL))

typedef struct {
  GObject parent;
} GthreeMaterial;

typedef struct {
  GObjectClass parent_class;

} GthreeMaterialClass;

GthreeMaterial *gthree_material_new ();
GType gthree_material_get_type (void) G_GNUC_CONST;

gboolean gthree_material_get_is_transparent (GthreeMaterial *material);


G_END_DECLS

#endif /* __GTHREE_MATERIAL_H__ */
