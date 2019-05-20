#ifndef __GTHREE_OBJECT_PRIVATE_H__
#define __GTHREE_OBJECT_PRIVATE_H__

#include <gthree/gthreeobject.h>
#include <gthree/gthreebufferprivate.h>
#include <gthree/gthreematerial.h>

typedef struct {
  GthreeObject *object;
  GthreeBuffer *buffer;
  float z;
  GthreeMaterial *material;
} GthreeObjectBuffer;

G_BEGIN_DECLS

typedef enum {
              GTHREE_SCENE_STATE_DETACHED, /* Not in scene, not realized */
              GTHREE_SCENE_STATE_REMOVED,  /* Removed from scene, not unrealized */
              GTHREE_SCENE_STATE_ADDED,    /* Added to scene, not realized */
              GTHREE_SCENE_STATE_ATTACHED, /* In scene, realized */
} GthreeSceneState;

gboolean         gthree_object_get_realized                (GthreeObject     *object);
GthreeSceneState gthree_object_get_scene_state             (GthreeObject     *object);
void             gthree_object_set_scene_state             (GthreeObject     *object,
                                                            GthreeSceneState  state);
GList *          gthree_object_get_object_buffers          (GthreeObject     *object);
void             gthree_object_add_buffer                  (GthreeObject     *object,
                                                            GthreeBuffer     *buffer,
                                                            GthreeMaterial   *material);
void             gthree_object_remove_buffer               (GthreeObject     *object,
                                                            GthreeBuffer     *buffer);
void             gthree_object_remove_buffers              (GthreeObject     *object);
void             gthree_object_call_before_render_callback (GthreeObject     *object,
                                                            GthreeScene      *scene,
                                                            GthreeCamera     *camera);


GthreeMaterial * gthree_object_buffer_resolve_material (GthreeObjectBuffer *object_buffer);

G_END_DECLS

#endif /* __GTHREE_OBJECT_PRIVATE_H__ */
