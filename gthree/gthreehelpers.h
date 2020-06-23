#ifndef __GTHREE_HELPERS_H__
#define __GTHREE_HELPERS_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_PLANE_HELPER      (gthree_plane_helper_get_type ())
#define GTHREE_PLANE_HELPER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_PLANE_HELPER, GthreePlaneHelper))
#define GTHREE_IS_PLANE_HELPER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_PLANE_HELPER))

typedef struct _GthreePlaneHelperClass GthreePlaneHelperClass;
typedef struct _GthreePlaneHelper GthreePlaneHelper;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePlaneHelper, g_object_unref)

GTHREE_API
GType gthree_plane_helper_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreePlaneHelper *gthree_plane_helper_new (const graphene_plane_t *plane,
                                            float size,
                                            const graphene_vec3_t *color);
GTHREE_API
void gthree_plane_helper_set_plane (GthreePlaneHelper *helper,
                                    const graphene_plane_t *plane);
GTHREE_API
const graphene_plane_t * gthree_plane_helper_get_plane (GthreePlaneHelper *helper);
GTHREE_API
void gthree_plane_helper_set_color (GthreePlaneHelper *helper,
                                    const graphene_vec3_t *color);
GTHREE_API
const graphene_vec3_t * gthree_plane_helper_get_color (GthreePlaneHelper *helper);
GTHREE_API
void gthree_plane_helper_set_size (GthreePlaneHelper *helper,
                                   float size);
GTHREE_API
float gthree_plane_helper_get_size (GthreePlaneHelper *helper);


#define GTHREE_TYPE_SKELETON_HELPER      (gthree_skeleton_helper_get_type ())
#define GTHREE_SKELETON_HELPER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_SKELETON_HELPER, GthreeSkeletonHelper))
#define GTHREE_IS_SKELETON_HELPER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_SKELETON_HELPER))

typedef struct _GthreeSkeletonHelperClass GthreeSkeletonHelperClass;
typedef struct _GthreeSkeletonHelper GthreeSkeletonHelper;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSkeletonHelper, g_object_unref)

GTHREE_API
GType gthree_skeleton_helper_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeSkeletonHelper *gthree_skeleton_helper_new (GthreeObject *root);

G_END_DECLS

#endif /* __GTHREE_HELPERS_H__ */
