#ifndef __GTHREE_SKELETON_H__
#define __GTHREE_SKELETON_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthreebone.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_SKELETON      (gthree_skeleton_get_type ())
#define GTHREE_SKELETON(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                               GTHREE_TYPE_SKELETON, \
                                                               GthreeSkeleton))
#define GTHREE_IS_SKELETON(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                               GTHREE_TYPE_SKELETON))


struct _GthreeSkeleton {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;

} GthreeSkeletonClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSkeleton, g_object_unref)

GTHREE_API
GType gthree_skeleton_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeSkeleton *gthree_skeleton_new  (GthreeBone **bones,
                                      int n_bones,
                                      graphene_matrix_t *bone_inverses);

GTHREE_API
int         gthree_skeleton_get_n_bones        (GthreeSkeleton *skeleton);
GTHREE_API
GthreeBone *gthree_skeleton_get_bone           (GthreeSkeleton *skeleton,
                                                int             index);
GTHREE_API
GthreeBone *gthree_skeleton_get_bone_by_name   (GthreeSkeleton *skeleton,
                                                const char     *name);
GTHREE_API
void        gthree_skeleton_calculate_inverses (GthreeSkeleton *skeleton);
GTHREE_API
void        gthree_skeleton_pose               (GthreeSkeleton *skeleton);


G_END_DECLS

#endif /* __GTHREE_SKELETON_H__ */
