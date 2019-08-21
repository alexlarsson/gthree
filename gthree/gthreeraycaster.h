#ifndef __GTHREE_RAYCASTER_H__
#define __GTHREE_RAYCASTER_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreecamera.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_RAYCASTER            (gthree_raycaster_get_type ())
#define GTHREE_RAYCASTER(inst)           (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_RAYCASTER, \
                                                             GthreeRaycaster))
#define GTHREE_RAYCASTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_RAYCASTER, GthreeRaycasterClass))
#define GTHREE_IS_RAYCASTER(inst)        (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_RAYCASTER))
#define GTHREE_RAYCASTER_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_RAYCASTER, GthreeRaycasterClass))

typedef struct {
  GthreeObject *object;
  float distance;
  graphene_vec3_t point;
  graphene_vec3_t face[3];
  int face_index;
  graphene_vec2_t uv;
  graphene_vec2_t uv2;
} GthreeRayIntersection;

GTHREE_API
GType gthree_ray_intersection_get_type (void) G_GNUC_CONST;

GTHREE_API
void                   gthree_ray_intersection_free (GthreeRayIntersection *intersection);
GTHREE_API
GthreeRayIntersection *gthree_ray_intersection_copy (GthreeRayIntersection *intersection);
GTHREE_API
GthreeRayIntersection *gthree_ray_intersection_new (void);

struct _GthreeRaycaster {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;
} GthreeRaycasterClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeRaycaster, g_object_unref)

GTHREE_API
GType gthree_raycaster_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeRaycaster *gthree_raycaster_new (void);
GTHREE_API
void                 gthree_raycaster_set_ray  (GthreeRaycaster *raycaster,
                                                const graphene_ray_t *ray);
GTHREE_API
void                 gthree_raycaster_set_from_camera  (GthreeRaycaster *raycaster,
                                                        GthreeCamera *camera,
                                                        float x,
                                                        float y);
GTHREE_API
const graphene_ray_t *gthree_raycaster_get_ray (GthreeRaycaster *raycaster);
GTHREE_API
void                 gthree_raycaster_set_near  (GthreeRaycaster *raycaster,
                                                 float            near);
GTHREE_API
float                gthree_raycaster_get_near (GthreeRaycaster *raycaster);
GTHREE_API
void                 gthree_raycaster_set_far   (GthreeRaycaster *raycaster,
                                                 float            far);
GTHREE_API
float                gthree_raycaster_get_far   (GthreeRaycaster *raycaster);

GTHREE_API
GPtrArray           *gthree_raycaster_intersect_object (GthreeRaycaster *raycaster,
                                                        GthreeObject *object,
                                                        gboolean recurse,
                                                        GPtrArray *optional_target);
GTHREE_API
GPtrArray           *gthree_raycaster_intersect_objects (GthreeRaycaster *raycaster,
                                                         GthreeObject **objects,
                                                         int n_objects,
                                                         gboolean recurse,
                                                         GPtrArray *optional_target);



G_END_DECLS

#endif /* __GTHREE_RAYCASTER_H__ */
