#include "gthreeraycaster.h"
#include "gthreeperspectivecamera.h"
#include "gthreeorthographiccamera.h"

typedef struct {
  graphene_ray_t ray;
  float near;
  float far;
} GthreeRaycasterPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeRaycaster, gthree_raycaster, G_TYPE_OBJECT)

void
gthree_ray_intersection_free (GthreeRayIntersection *intersection)
{
  g_clear_object (&intersection->object);
  g_free (intersection);
}

GthreeRayIntersection *
gthree_ray_intersection_copy (GthreeRayIntersection *intersection)
{
  GthreeRayIntersection *copy = gthree_ray_intersection_new (NULL);
  *copy = *intersection;
  if (copy->object)
    g_object_ref (copy->object);

  return copy;
}

GthreeRayIntersection *
gthree_ray_intersection_new (GthreeObject *object)
{
  GthreeRayIntersection *intersection = g_new0 (GthreeRayIntersection, 1);

  intersection->face_index = -1;
  intersection->material_index = -1;
  if (object)
    intersection->object = g_object_ref (object);

  return intersection;
}

static gint
compare_intersection (gconstpointer a,
                      gconstpointer b)
{
  const GthreeRayIntersection *aa = *(const GthreeRayIntersection **)a;
  const GthreeRayIntersection *bb = *(const GthreeRayIntersection **)b;

  if (aa->distance < bb->distance)
    return -1;
  else if (aa->distance > bb->distance)
    return 1;
  return 0;
}

G_DEFINE_BOXED_TYPE (GthreeRayIntersection, gthree_ray_intersection,
                     gthree_ray_intersection_copy,
                     gthree_ray_intersection_free);

static void
gthree_raycaster_init (GthreeRaycaster *raycaster)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  graphene_ray_init (&priv->ray, graphene_point3d_zero (), graphene_vec3_x_axis ());
  priv->near = 0;
  priv->far = INFINITY;
}

static void
gthree_raycaster_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_raycaster_parent_class)->finalize (obj);
}

static void
gthree_raycaster_class_init (GthreeRaycasterClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_raycaster_finalize;
}

GthreeRaycaster *
gthree_raycaster_new (void)
{
  return g_object_new (gthree_raycaster_get_type (), NULL);
}

void
gthree_raycaster_set_ray  (GthreeRaycaster *raycaster,
                           const graphene_ray_t *ray)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  priv->ray = *ray;
}

void
gthree_raycaster_set_from_camera  (GthreeRaycaster *raycaster,
                                   GthreeCamera *camera,
                                   float x,
                                   float y)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);
  graphene_vec3_t origin, direction;

  if (GTHREE_IS_PERSPECTIVE_CAMERA (camera))
    {
      graphene_vec4_t origin4;
      graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (camera)), 3, &origin4);

      graphene_vec4_get_xyz (&origin4, &origin);

      gthree_camera_unproject (camera, graphene_vec3_init (&direction, x, y, 0.5),
                               &direction);
      graphene_vec3_subtract (&direction, &origin, &direction);
      graphene_vec3_normalize (&direction, &direction);
    }
  else if (GTHREE_IS_ORTHOGRAPHIC_CAMERA (camera))
    {
      // set origin in plane of camera
      graphene_vec3_init (&origin, x, y,
                          (gthree_camera_get_near (camera) + gthree_camera_get_far (camera)) / (gthree_camera_get_near (camera) - gthree_camera_get_far (camera)));
      gthree_camera_unproject (camera, &origin, &origin);

      graphene_matrix_transform_vec3 (gthree_object_get_world_matrix (GTHREE_OBJECT (camera)),
                                      graphene_vec3_init (&direction, 0, 0, -1),
                                      &direction);
    }
  else
    g_assert_not_reached ();

  graphene_ray_init_from_vec3 (&priv->ray, &origin, &direction);

}


const graphene_ray_t *
gthree_raycaster_get_ray (GthreeRaycaster *raycaster)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  return &priv->ray;
}

void
gthree_raycaster_set_near (GthreeRaycaster *raycaster,
                            float            near)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  priv->near = near;
}

float
gthree_raycaster_get_near (GthreeRaycaster *raycaster)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  return priv->near;
}

void
gthree_raycaster_set_far (GthreeRaycaster *raycaster,
                            float            far)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  priv->far = far;
}

float
gthree_raycaster_get_far (GthreeRaycaster *raycaster)
{
  GthreeRaycasterPrivate *priv = gthree_raycaster_get_instance_private (raycaster);

  return priv->far;
}

void
intersect_object (GthreeRaycaster *raycaster,
                  GthreeObject *object,
                  gboolean recurse,
                  GPtrArray *intersections)
{
  if (!gthree_object_get_visible (object))
    return;

  gthree_object_raycast (object, raycaster, intersections);

  if (recurse)
    {
      GthreeObjectIter iter;
      GthreeObject *child;

      gthree_object_iter_init (&iter, object);
      while (gthree_object_iter_next (&iter, &child))
        intersect_object (raycaster, child, TRUE, intersections);
    }
}

GPtrArray *
gthree_raycaster_intersect_objects (GthreeRaycaster *raycaster,
                                    GthreeObject **objects,
                                    int n_objects,
                                    gboolean recurse,
                                    GPtrArray *optional_target)
{
  GPtrArray *target = NULL;
  int i;

  if (optional_target != NULL)
    target = g_ptr_array_ref (optional_target);
  else
    target = g_ptr_array_new_with_free_func ((GDestroyNotify)gthree_ray_intersection_free);

  for (i = 0; i < n_objects; i++)
    intersect_object (raycaster, objects[i], recurse, target);

  g_ptr_array_sort (target, compare_intersection);

  return target;
}


GPtrArray *
gthree_raycaster_intersect_object (GthreeRaycaster *raycaster,
                                   GthreeObject *object,
                                   gboolean recurse,
                                   GPtrArray *optional_target)
{
  return gthree_raycaster_intersect_objects (raycaster,
                                             &object, 1,
                                             recurse,
                                             optional_target);
}
