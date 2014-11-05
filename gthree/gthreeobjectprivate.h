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

GList *gthree_object_get_object_buffers (GthreeObject       *object);
void   gthree_object_add_buffer         (GthreeObject       *object,
                                         GthreeBuffer       *buffer,
                                         GthreeMaterial     *material);
void   gthree_object_remove_buffer      (GthreeObject       *object,
                                         GthreeBuffer       *buffer);
void   gthree_object_remove_buffers     (GthreeObject       *object);

GthreeMaterial * gthree_object_buffer_resolve_material (GthreeObjectBuffer *object_buffer);

G_END_DECLS

#endif /* __GTHREE_OBJECT_PRIVATE_H__ */
