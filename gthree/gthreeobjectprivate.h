#ifndef __GTHREE_OBJECT_PRIVATE_H__
#define __GTHREE_OBJECT_PRIVATE_H__

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>

G_BEGIN_DECLS

void       gthree_object_set_direct_uniforms  (GthreeObject          *object,
                                               GthreeProgram         *program,
                                               GthreeRenderer *renderer);
void       gthree_object_fill_render_list (GthreeObject   *object,
                                           GthreeRenderList *list);
void       gthree_object_call_before_render_callback (GthreeObject   *object,
                                                      GthreeScene    *scene,
                                                      GthreeCamera   *camera);

G_END_DECLS

#endif /* __GTHREE_OBJECT_PRIVATE_H__ */
