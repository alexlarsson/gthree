#ifndef __GTHREE_SCENE_H__
#define __GTHREE_SCENE_H__

#include <gtk/gtk.h>

#include "gthreeobject.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_SCENE      (gthree_scene_get_type ())
#define GTHREE_SCENE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_SCENE, \
                                                             GthreeScene))
#define GTHREE_IS_SCENE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_SCENE))

typedef struct {
  GthreeObject parent;
} GthreeScene;

typedef struct {
  GthreeObjectClass parent_class;

} GthreeSceneClass;

GthreeScene *gthree_scene_new ();
GType gthree_scene_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_SCENE_H__ */
