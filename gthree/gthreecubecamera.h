#ifndef __GTHREE_CUBE_CAMERA_H__
#define __GTHREE_CUBE_CAMERA_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreerenderer.h>
#include <gthree/gthreerendertarget.h>
#include <gthree/gthreescene.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_CUBE_CAMERA      (gthree_cube_camera_get_type ())
#define GTHREE_CUBE_CAMERA(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                   GTHREE_TYPE_CUBE_CAMERA, \
                                                                   GthreeCubeCamera))
#define GTHREE_IS_CUBE_CAMERA(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                   GTHREE_TYPE_CUBE_CAMERA))

struct _GthreeCubeCamera {
  GthreeObject parent;
};

typedef struct {
  GthreeObjectClass parent_class;
} GthreeCubeCameraClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeCubeCamera, g_object_unref)

GTHREE_API
GType gthree_cube_camera_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeCubeCamera *gthree_cube_camera_new (float               near,
                                          float               far,
                                          GthreeRenderTarget *render_target);

GTHREE_API
void gthree_cube_camera_update (GthreeCubeCamera *cube_camera,
                                GthreeRenderer   *renderer,
                                GthreeScene      *scene);

GTHREE_API
GthreeRenderTarget *gthree_cube_camera_get_render_target (GthreeCubeCamera *cube_camera);

G_END_DECLS

#endif /* __GTHREE_CUBE_CAMERA_H__ */
