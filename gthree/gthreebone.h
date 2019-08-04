#ifndef __GTHREE_BONE_H__
#define __GTHREE_BONE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_BONE      (gthree_bone_get_type ())
#define GTHREE_BONE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_BONE, GthreeBone))
#define GTHREE_BONE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_BONE, GthreeBoneClass))
#define GTHREE_IS_BONE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_BONE))
#define GTHREE_IS_BONE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_BONE))
#define GTHREE_BONE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_BONE, GthreeBoneClass))

struct _GthreeBone {
  GthreeObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeBone, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

} GthreeBoneClass;

GTHREE_API
GType gthree_bone_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeBone *gthree_bone_new (void);

G_END_DECLS

#endif /* __GTHREE_BONE_H__ */
