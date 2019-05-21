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

GPtrArray *gthree_object_get_object_buffers          (GthreeObject   *object);
void       gthree_object_call_before_render_callback (GthreeObject   *object,
                                                      GthreeScene    *scene,
                                                      GthreeCamera   *camera);

G_END_DECLS

#endif /* __GTHREE_OBJECT_PRIVATE_H__ */
