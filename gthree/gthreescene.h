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

struct _GthreeScene {
  GthreeObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeScene, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

} GthreeSceneClass;

GTHREE_API
GType  gthree_scene_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeScene *gthree_scene_new ();

GTHREE_API
GthreeMaterial *gthree_scene_get_override_material  (GthreeScene   *scene);
GTHREE_API
void            gthree_scene_set_override_material  (GthreeScene   *scene,
                                                    GthreeMaterial *material);
GTHREE_API
const graphene_vec3_t * gthree_scene_get_background_color   (GthreeScene   *scene);
GTHREE_API
void            gthree_scene_set_background_color   (GthreeScene   *scene,
                                                     const graphene_vec3_t *color);
GTHREE_API
float           gthree_scene_get_background_alpha   (GthreeScene   *scene);
GTHREE_API
void            gthree_scene_set_background_alpha   (GthreeScene   *scene,
                                                     float          alpha);
GTHREE_API
GthreeTexture * gthree_scene_get_background_texture (GthreeScene   *scene);
GTHREE_API
void            gthree_scene_set_background_texture (GthreeScene   *scene,
                                                     GthreeTexture *texture);

G_END_DECLS

#endif /* __GTHREE_SCENE_H__ */
