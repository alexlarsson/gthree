#ifndef __GTHREE_SCENE_H__
#define __GTHREE_SCENE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>

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

GType  gthree_scene_get_type (void) G_GNUC_CONST;

GthreeScene *gthree_scene_new ();

void            gthree_scene_realize_objects       (GthreeScene  *scene);
GthreeMaterial *gthree_scene_get_override_material (GthreeScene  *scene);
GList *         gthree_scene_get_lights            (GthreeScene  *scene);
GdkGLContext *  gthree_scene_get_context           (GthreeScene  *scene);
void            gthree_scene_set_context           (GthreeScene  *scene,
                                                    GdkGLContext *context);

G_END_DECLS

#endif /* __GTHREE_SCENE_H__ */
