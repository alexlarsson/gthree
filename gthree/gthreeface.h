#ifndef __GTHREE_FACE_H__
#define __GTHREE_FACE_H__

#include <gtk/gtk.h>

#include <graphene.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_FACE      (gthree_face_get_type ())
#define GTHREE_FACE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_FACE, \
                                                             GthreeFace))
#define GTHREE_IS_FACE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_FACE))

typedef struct _GthreeFace GthreeFace;

typedef struct {
  GObjectClass parent_class;

} GthreeFaceClass;

GType gthree_face_get_type (void) G_GNUC_CONST;

GthreeFace *gthree_face_new                (int              a,
                                            int              b,
                                            int              c);
void        gthree_face_set_normal         (GthreeFace      *face,
                                            graphene_vec3_t *normal);
void        gthree_face_set_color          (GthreeFace      *face,
                                            GdkRGBA         *color);
void        gthree_face_set_material_index (GthreeFace      *face,
                                            int              material_index);
int         gthree_face_get_material_index (GthreeFace      *face);
void        gthree_face_destroy            (GthreeFace      *face);



G_END_DECLS

#endif /* __GTHREE_FACE_H__ */
