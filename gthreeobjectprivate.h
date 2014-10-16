#ifndef __GTHREE_OBJECT_PRIVATE_H__
#define __GTHREE_OBJECT_PRIVATE_H__

#include "gthreeobject.h"
#include "gthreebufferprivate.h"

G_BEGIN_DECLS

void gthree_object_add_buffer (GthreeObject *object,
                               GthreeBuffer *buffer);
void gthree_object_remove_buffer (GthreeObject *object,
                                  GthreeBuffer *buffer);
void gthree_object_remove_buffers (GthreeObject *object);

G_END_DECLS

#endif /* __GTHREE_OBJECT_PRIVATE_H__ */
