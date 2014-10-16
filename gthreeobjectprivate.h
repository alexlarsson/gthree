#ifndef __GTHREE_OBJECT_PRIVATE_H__
#define __GTHREE_OBJECT_PRIVATE_H__

#include "gthreeobject.h"
#include "gthreebufferprivate.h"
#include "gthreematerial.h"

G_BEGIN_DECLS

typedef struct {
  GthreeObject *object;
  GthreeBuffer *buffer;
  GthreeMaterial *material;
  float z;
} GthreeObjectBuffer;

void   gthree_object_buffer_free        (GthreeObjectBuffer *object_buffer);
GList *gthree_object_get_object_buffers (GthreeObject       *object);
void   gthree_object_add_buffer         (GthreeObject       *object,
                                         GthreeBuffer       *buffer);
void   gthree_object_remove_buffer      (GthreeObject       *object,
                                         GthreeBuffer       *buffer);
void   gthree_object_remove_buffers     (GthreeObject       *object);


G_END_DECLS

#endif /* __GTHREE_OBJECT_PRIVATE_H__ */
