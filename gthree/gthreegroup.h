#ifndef __GTHREE_GROUP_H__
#define __GTHREE_GROUP_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_GROUP      (gthree_group_get_type ())
#define GTHREE_GROUP(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_GROUP, GthreeGroup))
#define GTHREE_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_GROUP, GthreeGroupClass))
#define GTHREE_IS_GROUP(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_GROUP))
#define GTHREE_IS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_GROUP))
#define GTHREE_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_GROUP, GthreeGroupClass))

struct _GthreeGroup {
  GthreeObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeGroup, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

} GthreeGroupClass;

GType gthree_group_get_type (void) G_GNUC_CONST;

GthreeGroup *gthree_group_new (void);

G_END_DECLS

#endif /* __GTHREE_GROUP_H__ */
