#ifndef __GTHREE_FACE_H__
#define __GTHREE_FACE_H__

#include <gtk/gtk.h>

#include <graphene.h>

G_BEGIN_DECLS

typedef struct _GthreeFace GthreeFace;

GthreeFace *gthree_face_new                (int              a,
                                            int              b,
                                            int              c);
void        gthree_face_set_normal         (GthreeFace      *face,
                                            graphene_vec3_t *normal);
void        gthree_face_set_color          (GthreeFace      *face,
                                            GdkRGBA         *color);
void        gthree_face_set_material_index (GthreeFace      *face,
                                            int              material_index);
void        gthree_face_destroy            (GthreeFace      *face);



G_END_DECLS

#endif /* __GTHREE_FACE_H__ */
