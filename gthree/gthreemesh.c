#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"
#include "gthreemeshmaterial.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"
#include "gthreeraycaster.h"

/* These are some graphene_ray_t helpers, they should probably be in graphene */

typedef enum {
  RAY_INTERSECTION_KIND_NONE,
  RAY_INTERSECTION_KIND_ENTER,
  RAY_INTERSECTION_KIND_LEAVE,
} RayIntersectionKind;

static RayIntersectionKind
ray_intersect_sphere (const graphene_ray_t *ray,
                      const graphene_sphere_t *sphere,
                      float *t_out)
{
  graphene_vec3_t v1, origin, direction, center;
  graphene_point3d_t center_point, origin_point;

  graphene_ray_get_origin (ray, &origin_point);
  graphene_point3d_to_vec3 (&origin_point, &origin);

  graphene_ray_get_direction (ray, &direction);

  graphene_sphere_get_center (sphere, &center_point);
  graphene_point3d_to_vec3 (&center_point, &center);

  graphene_vec3_subtract (&center, &origin, &v1);

  // (signed) distance along ray to point nearest sphere center
  float tca = graphene_vec3_dot (&v1, &direction);

  // square of distance from ray line to sphere center
  float d2 = graphene_vec3_dot (&v1, &v1) - tca * tca;

  float radius2 = graphene_sphere_get_radius (sphere) * graphene_sphere_get_radius (sphere);
  if (d2 > radius2)
    return RAY_INTERSECTION_KIND_NONE;

  // Distance to entry/exit point
  float thc = sqrtf (radius2 - d2);

  // t0 = first intersect point - entrance on front of sphere
  float t0 = tca - thc;

  // t1 = second intersect point - exit point on back of sphere
  float t1 = tca + thc;

  // test to see if both t0 and t1 are behind the ray - if so, no intersection
  if (t0 < 0 && t1 < 0)
    return RAY_INTERSECTION_KIND_NONE;

  // test to see if t0 is behind the ray:
  // if it is, the ray is inside the sphere, so return t1,
  // in order to always return an intersect point that is in front of the ray.
  if (t0 < 0)
    {
      if (t_out)
        *t_out = t1;
      return RAY_INTERSECTION_KIND_LEAVE;
    }

  // else t0 is in front of the ray, so return  t0
  if (t_out)
    *t_out = t0;
  return RAY_INTERSECTION_KIND_ENTER;
}

static gboolean
ray_intersects_sphere (const graphene_ray_t *ray,
                       const graphene_sphere_t *sphere)
{
  return ray_intersect_sphere (ray, sphere, NULL) != RAY_INTERSECTION_KIND_NONE;
}

static RayIntersectionKind
ray_intersect_box (const graphene_ray_t *ray,
                   const graphene_box_t *box,
                   float *t_out)
{
  graphene_point3d_t origin;
  graphene_vec3_t direction;
  float tmin, tmax, tymin, tymax, tzmin, tzmax;
  graphene_point3d_t min, max;
  float invdirx, invdiry, invdirz;

  graphene_ray_get_origin (ray, &origin);
  graphene_ray_get_direction (ray, &direction);

  invdirx = 1 / graphene_vec3_get_x (&direction);
  invdiry = 1 / graphene_vec3_get_y (&direction);
  invdirz = 1 / graphene_vec3_get_z (&direction);

  graphene_box_get_min (box, &min);
  graphene_box_get_max (box, &max);

  if (invdirx >= 0)
    {
      tmin = (min.x - origin.x) * invdirx;
      tmax = (max.x - origin.x) * invdirx;
    }
  else
    {
      tmin = (max.x - origin.x) * invdirx;
      tmax = (min.x - origin.x) * invdirx;
    }

  if (invdiry >= 0)
    {
      tymin = (min.y - origin.y) * invdiry;
      tymax = (max.y - origin.y) * invdiry;
    }
  else
    {
      tymin = (max.y - origin.y) * invdiry;
      tymax = (min.y - origin.y) * invdiry;
    }

  if ((tmin > tymax) || (tymin > tmax))
    return RAY_INTERSECTION_KIND_NONE;

  // These lines also handle the case where tmin or tmax is NaN
  // (result of 0 * Infinity). x !== x returns true if x is NaN
  if (tymin > tmin || tmin != tmin)
    tmin = tymin;

  if (tymax < tmax || tmax != tmax)
    tmax = tymax;

  if (invdirz >= 0)
    {
      tzmin = (min.z - origin.z) * invdirz;
      tzmax = (max.z - origin.z) * invdirz;
    }
  else
    {
      tzmin = (max.z - origin.z) * invdirz;
      tzmax = (min.z - origin.z) * invdirz;
    }

  if ((tmin > tzmax) || (tzmin > tmax))
    return RAY_INTERSECTION_KIND_NONE;

  if (tzmin > tmin || tmin != tmin)
    tmin = tzmin;

  if (tzmax < tmax || tmax != tmax)
    tmax = tzmax;

  //return point closest to the ray (positive side)
  if (tmax < 0)
    return RAY_INTERSECTION_KIND_NONE;

  if (tmin >= 0)
    {
      if (t_out)
        *t_out = tmin;
      return RAY_INTERSECTION_KIND_ENTER;
    }

  if (t_out)
    *t_out = tmax;
  return RAY_INTERSECTION_KIND_LEAVE;
}

static gboolean
ray_intersects_box (const graphene_ray_t *ray,
                    const graphene_box_t *box)
{
  return ray_intersect_box (ray, box, NULL) != RAY_INTERSECTION_KIND_NONE;
}

/* This takes the separate vectors instead of a graphene_triangle_t so
   we don't have to create a temporary one for every vertex we extract
   from the buffers. It would be nice if we could initialize the
   vertices of a graphene_triangle_t in place so we could avoid such a
   copy. */
static RayIntersectionKind
ray_intersect_triangle (const graphene_ray_t *local_ray,
                        const graphene_vec3_t *vA,
                        const graphene_vec3_t *vB,
                        const graphene_vec3_t *vC,
                        float *t_out)
{
  graphene_point3d_t origin_point;
  graphene_vec3_t direction, origin;
  graphene_vec3_t diff, edge1, edge2, normal;
  RayIntersectionKind kind;

  // from http://www.geometrictools.com/GTEngine/Include/Mathematics/GteIntrRay3Triangle3.h

  graphene_ray_get_origin (local_ray, &origin_point);
  graphene_point3d_to_vec3 (&origin_point, &origin);
  graphene_ray_get_direction (local_ray, &direction);

  // Compute the offset origin, edges, and normal.
  graphene_vec3_subtract (vB, vA, &edge1);
  graphene_vec3_subtract (vC, vA, &edge2);
  graphene_vec3_cross (&edge1, &edge2, &normal);

  // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
  // E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
  //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
  //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
  //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
  float DdN = graphene_vec3_dot (&direction, &normal);
  float sign;

  if (DdN > 0)
    {
      kind = RAY_INTERSECTION_KIND_LEAVE;
      sign = 1;

    }
  else if (DdN < 0)
    {
      kind = RAY_INTERSECTION_KIND_ENTER;
      sign = -1;
      DdN = -DdN;
    }
  else
    {
      // Ray and triangle are parallel, call it a "no intersection"
      // even if the ray does intersect.
      return RAY_INTERSECTION_KIND_NONE;
    }

  graphene_vec3_subtract (&origin, vA, &diff);
  graphene_vec3_cross (&diff, &edge2, &edge2);
  float DdQxE2 = sign * graphene_vec3_dot (&direction, &edge2);

  // b1 < 0, no intersection
  if ( DdQxE2 < 0 )
    return RAY_INTERSECTION_KIND_NONE;

  graphene_vec3_cross (&edge1, &diff, &edge1);

  float DdE1xQ = sign * graphene_vec3_dot (&direction, &edge1);

  // b2 < 0, no intersection
  if ( DdE1xQ < 0 )
    return RAY_INTERSECTION_KIND_NONE;

  // b1+b2 > 1, no intersection
  if ( DdQxE2 + DdE1xQ > DdN )
    return RAY_INTERSECTION_KIND_NONE;

  // Line intersects triangle, check if ray does.
  float QdN = -sign * graphene_vec3_dot (&diff, &normal);

  // t < 0, no intersection
  if ( QdN < 0 )
    return RAY_INTERSECTION_KIND_NONE;

  if (t_out)
    *t_out = QdN / DdN;

  return kind;
}

static void
triangle_get_uv (const graphene_triangle_t *triangle,
                 const graphene_point3d_t *point,
                 const graphene_vec2_t *uvA,
                 const graphene_vec2_t *uvB,
                 const graphene_vec2_t *uvC,
                 graphene_vec2_t *uv_out)
{
  // TODO
}

typedef struct {
  GthreeGeometry *geometry;
  GPtrArray *materials;
  GthreeDrawMode draw_mode;

  GArray *morph_target_influences; /* array of floats */
  GHashTable *morph_target_dictionary; /* Map from morph name to index in attributes for each name */
} GthreeMeshPrivate;

enum {
  PROP_0,

  PROP_GEOMETRY,
  PROP_MATERIALS,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMesh, gthree_mesh, GTHREE_TYPE_OBJECT)

GthreeMesh *
gthree_mesh_new (GthreeGeometry *geometry,
                 GthreeMaterial *material)
{
  g_autoptr(GPtrArray) materials = g_ptr_array_new_with_free_func (g_object_unref);

  if (material)
    g_ptr_array_add (materials, g_object_ref (material));

  GthreeMesh *mesh =
    g_object_new (gthree_mesh_get_type (),
                  "geometry", geometry,
                  "materials", materials,
                  NULL);

  gthree_mesh_update_morph_targets (mesh);

  return mesh;
}

static void
gthree_mesh_init (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  priv->materials = g_ptr_array_new_with_free_func (g_object_unref);
  priv->draw_mode = GTHREE_DRAW_MODE_TRIANGLES;
}

static void
gthree_mesh_finalize (GObject *obj)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  g_clear_object (&priv->geometry);
  g_ptr_array_unref (priv->materials);

  if (priv->morph_target_influences)
    g_array_unref (priv->morph_target_influences);
  if (priv->morph_target_dictionary)
    g_hash_table_unref (priv->morph_target_dictionary);

  G_OBJECT_CLASS (gthree_mesh_parent_class)->finalize (obj);
}

static void
gthree_mesh_update (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry);

  //material.attributes && clearCustomAttributes( material );
}

gboolean
gthree_mesh_has_morph_targets (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->morph_target_influences != NULL;
}

GArray *
gthree_mesh_get_morph_targets (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->morph_target_influences;
}

void
gthree_mesh_set_morph_targets (GthreeMesh     *mesh,
                               GArray *morph_targets)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  int i;
  int len;

  if (priv->morph_target_influences == NULL ||
      morph_targets == NULL)
    return;

  len = MIN (morph_targets->len, priv->morph_target_influences->len);
  for (i = 0; i < len; i++)
    {
      float v = g_array_index (morph_targets, float, i);
      g_array_index (priv->morph_target_influences, float, i) = v;
    }
}

void
gthree_mesh_update_morph_targets (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  g_autoptr(GList) names = NULL;
  const char *first_name;
  GPtrArray *attributes;

  names = gthree_geometry_get_morph_attributes_names (priv->geometry);
  if (names == NULL)
    return;
  first_name = names->data;

  attributes = gthree_geometry_get_morph_attributes (priv->geometry, first_name);
  if (attributes)
    {
      priv->morph_target_influences = g_array_new (FALSE, FALSE, sizeof (float));
      priv->morph_target_dictionary = g_hash_table_new (g_str_hash, g_str_equal);

      for (int m = 0; m < attributes->len; m++)
        {
          GthreeAttribute *attribute = g_ptr_array_index (attributes, m);
          const char *name = gthree_attribute_get_name (attribute);
          float zero = 0;
          g_array_append_val (priv->morph_target_influences, zero);
          g_hash_table_insert (priv->morph_target_dictionary,
                               (char *)name, GINT_TO_POINTER(m));
        }
    }
}

static void
gthree_mesh_fill_render_list (GthreeObject   *object,
                              GthreeRenderList *list)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  gthree_geometry_fill_render_list (priv->geometry, list, NULL, priv->materials, object);
}

static gboolean
gthree_mesh_in_frustum (GthreeObject *object,
                        const graphene_frustum_t *frustum)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  graphene_sphere_t sphere;

  if (!priv->geometry)
    return FALSE;

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static GthreeRayIntersection *
check_intersection (GthreeObject *object,
                    GthreeMaterial *material,
                    GthreeRaycaster *raycaster,
                    const graphene_ray_t *local_ray,
                    const graphene_vec3_t *vA,
                    const graphene_vec3_t *vB,
                    const graphene_vec3_t *vC)
{
  GthreeSide side = GTHREE_SIDE_FRONT;
  RayIntersectionKind kind;
  graphene_point3d_t local_intersection_point;
  graphene_point3d_t world_intersection_point;
  float t, distance;
  graphene_point3d_t world_origin;
  GthreeRayIntersection *intersection;

  if (material)
    side = gthree_material_get_side (material);

  kind = ray_intersect_triangle (local_ray, vC, vB, vA, &t);

  switch (side)
    {
    case GTHREE_SIDE_FRONT:
      if (kind != RAY_INTERSECTION_KIND_ENTER)
        return NULL;
      break;
    case GTHREE_SIDE_BACK:
      if (kind != RAY_INTERSECTION_KIND_LEAVE)
        return NULL;
      break;
    default: // double sided
      if (kind == RAY_INTERSECTION_KIND_NONE)
        return NULL;
      break;
    }

  graphene_ray_get_position_at (local_ray, t, &local_intersection_point);

  if (t < gthree_raycaster_get_near (raycaster) ||
      t > gthree_raycaster_get_far (raycaster))
    return NULL;

  graphene_matrix_transform_point3d (gthree_object_get_world_matrix (object),
                                     &local_intersection_point, &world_intersection_point);

  graphene_ray_get_origin (gthree_raycaster_get_ray (raycaster), &world_origin);

  distance = graphene_point3d_distance (&world_intersection_point, &world_origin, NULL);

  if (distance < gthree_raycaster_get_near (raycaster) ||
      distance > gthree_raycaster_get_far (raycaster))
    return NULL;

  intersection = gthree_ray_intersection_new (object);

  intersection->distance = distance;
  intersection->point = world_intersection_point;

 return intersection;
}

static void
do_geometry_intersection (GthreeObject *object,
                          GthreeMaterial *material,
                          GthreeRaycaster *raycaster,
                          const graphene_ray_t *local_ray,
                          GthreeAttribute *position,
                          GPtrArray *morph_position,
                          GthreeAttribute *uv,
                          GPtrArray *intersections,
                          int a,
                          int b,
                          int c,
                          int face_index,
                          int material_index)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  graphene_vec3_t vA, vB, vC;
  graphene_vec2_t uvA, uvB, uvC;
  graphene_vec3_t morphA, morphB, morphC;
  GthreeRayIntersection *intersection;

  graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, a), &vA);
  graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, b), &vB);
  graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, c), &vC);

  if (material != NULL &&
      GTHREE_IS_MESH_MATERIAL (material) &&
      gthree_mesh_material_get_morph_targets (GTHREE_MESH_MATERIAL (material)) &&
      morph_position &&  priv->morph_target_influences)
    {
      graphene_vec3_init (&morphA, 0, 0, 0);
      graphene_vec3_init (&morphB, 0, 0, 0);
      graphene_vec3_init (&morphC, 0, 0, 0);

      for (int i = 0; i < priv->morph_target_influences->len; i++)
        {
          float influence = g_array_index (priv->morph_target_influences, float, i);
          GthreeAttribute *attribute = g_ptr_array_index (morph_position, i);
          graphene_vec3_t tmpA, tmpB, tmpC;

          if (influence == 0)
            continue;

          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (attribute, a), &tmpA);
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (attribute, b), &tmpB);
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (attribute, c), &tmpC);

          graphene_vec3_subtract (&tmpA, &vA, &tmpA);
          graphene_vec3_scale (&tmpA, influence, &tmpA);
          graphene_vec3_add (&morphA, &tmpA, &morphA);

          graphene_vec3_subtract (&tmpB, &vB, &tmpB);
          graphene_vec3_scale (&tmpB, influence, &tmpB);
          graphene_vec3_add (&morphB, &tmpB, &morphB);

          graphene_vec3_subtract (&tmpC, &vC, &tmpC);
          graphene_vec3_scale (&tmpC, influence, &tmpC);
          graphene_vec3_add (&morphC, &tmpC, &morphC);

        }

      graphene_vec3_add (&vA, &morphA, &vA);
      graphene_vec3_add (&vB, &morphB, &vB);
      graphene_vec3_add (&vC, &morphC, &vC);
    }

  intersection = check_intersection (object, material, raycaster, local_ray, &vA, &vB, &vC);
  if (intersection)
    {
      intersection->face_index = face_index;
      graphene_triangle_init_from_vec3 (&intersection->face,
                                        &vA, &vB, &vC); // NOTE: In object coords, like three.js
      intersection->material_index = material_index;

      if (uv)
        {
          gthree_attribute_get_vec2 (uv, a, &uvA);
          gthree_attribute_get_vec2 (uv, b, &uvB);
          gthree_attribute_get_vec2 (uv, c, &uvC);

          triangle_get_uv (&intersection->face,
                           &intersection->point,
                           &uvA, &uvB, &uvC,
                           &intersection->uv);
        }

      g_ptr_array_add (intersections, intersection);
    }
}

static void
gthree_mesh_raycast (GthreeObject *object,
                     GthreeRaycaster *raycaster,
                     GPtrArray *intersections)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  graphene_sphere_t world_sphere;
  const graphene_ray_t *world_ray;
  graphene_ray_t local_ray;
  graphene_matrix_t inverse_matrix;
  GthreeAttribute *index, *position, *uv;
  GPtrArray *morph_position;
  int n_groups, i;
  int start, end, j, jl;

  if (priv->materials->len == 0)
    return;

  world_ray = gthree_raycaster_get_ray (raycaster);

  // Checking boundingSphere distance to ray

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &world_sphere);

  world_ray = gthree_raycaster_get_ray (raycaster);
  if (!ray_intersects_sphere (world_ray, &world_sphere))
    return;

  graphene_matrix_inverse (gthree_object_get_world_matrix (object), &inverse_matrix);
  graphene_matrix_transform_ray (&inverse_matrix, world_ray, &local_ray);

  // Check boundingBox before continuing
  if (!ray_intersects_box (&local_ray, gthree_geometry_get_bounding_box  (priv->geometry)))
    return;

  n_groups = gthree_geometry_get_n_groups (priv->geometry);
  index = gthree_geometry_get_index (priv->geometry);

  position = gthree_geometry_get_position (priv->geometry);
  if (position == NULL)
    return;

  uv = gthree_geometry_get_attribute (priv->geometry, "uv");
  morph_position = gthree_geometry_get_morph_attributes (priv->geometry, "position");

  int draw_range_start = gthree_geometry_get_draw_range_start (priv->geometry);
  int draw_range_end = gthree_geometry_get_draw_range_count (priv->geometry);
  if (draw_range_end < 0)
    draw_range_end = gthree_geometry_get_vertex_count (priv->geometry);
  else
    draw_range_end = MIN (gthree_geometry_get_vertex_count (priv->geometry), draw_range_start + draw_range_end);

  // TODO: Should we check material.visible here? three.js doesn't
  if (index)
    {
      // Indexed geometry
      if (priv->materials->len > 1)
        {
          // Use groups
          for (i = 0; i < n_groups; i++)
            {
              GthreeGeometryGroup *group = gthree_geometry_get_group (priv->geometry, i);
              GthreeMaterial *material = gthree_mesh_get_material (mesh, group->material_index);

              start = MAX (group->start, draw_range_start);
              end = MIN(group->start + group->count, draw_range_end);

              for (j = start, jl = end; j < jl; j += 3 )
                {
                  int a = gthree_attribute_get_uint (index, j + 0);
                  int b = gthree_attribute_get_uint (index, j + 1);
                  int c = gthree_attribute_get_uint (index, j + 2);

                  do_geometry_intersection (object, material, raycaster, &local_ray,
                                            position, morph_position, uv, intersections,
                                            a, b, c, j / 3, i);
                }
            }
        }
      else
        {
          // No groups
          GthreeMaterial *material = gthree_mesh_get_material (mesh, 0);

          for (j = draw_range_start, jl = draw_range_end; j < jl; j += 3 )
            {
              int a = gthree_attribute_get_uint (index, j + 0);
              int b = gthree_attribute_get_uint (index, j + 1);
              int c = gthree_attribute_get_uint (index, j + 2);

              do_geometry_intersection (object, material, raycaster, &local_ray,
                                        position, morph_position, uv, intersections,
                                        a, b, c, j / 3, 0);
            }
        }
    }
  else
    {
      // Non-indexed geometry
      if (priv->materials->len > 1)
        {
          // Use groups
          for (i = 0; i < n_groups; i++)
            {
              GthreeGeometryGroup *group = gthree_geometry_get_group (priv->geometry, i);
              GthreeMaterial *material = gthree_mesh_get_material (mesh, group->material_index);

              start = MAX (group->start, draw_range_start);
              end = MIN(group->start + group->count, draw_range_end);

              for (j = start, jl = end; j < jl; j += 3 )
                {
                  int a = j + 0;
                  int b = j + 1;
                  int c = j + 2;

                  do_geometry_intersection (object, material, raycaster, &local_ray,
                                            position, morph_position, uv, intersections,
                                            a, b, c, j / 3, i);
                }
            }
        }
      else
        {
          // No groups
          GthreeMaterial *material = gthree_mesh_get_material (mesh, 0);

          if (material == NULL)
            return;

          for (j = draw_range_start, jl = draw_range_end; j < jl; j += 3 )
            {
                  int a = j + 0;
                  int b = j + 1;
                  int c = j + 2;

                  do_geometry_intersection (object, material, raycaster, &local_ray,
                                            position, morph_position, uv, intersections,
                                            a, b, c, j / 3, 0);
            }
        }
    }
}

static void
gthree_mesh_set_property (GObject *obj,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  switch (prop_id)
    {
    case PROP_GEOMETRY:
      g_set_object (&priv->geometry, g_value_get_object (value));
      break;

    case PROP_MATERIALS:
      gthree_mesh_set_materials (mesh, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_get_property (GObject *obj,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  switch (prop_id)
    {
    case PROP_GEOMETRY:
      g_value_set_object (value, priv->geometry);
      break;

    case PROP_MATERIALS:
      g_value_set_boxed (value, priv->materials);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

GthreeMaterial *
gthree_mesh_get_material (GthreeMesh *mesh,
                          int index)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  if (index >= priv->materials->len)
    return NULL;

  return g_ptr_array_index (priv->materials, index);
}

int
gthree_mesh_get_n_materials (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->materials->len;
}

void
gthree_mesh_set_materials (GthreeMesh *mesh,
                           GPtrArray *materials)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  int i;

  // Clear all
  g_ptr_array_set_size (priv->materials, 0);

  g_ptr_array_set_size (priv->materials, materials->len);
  for (i = 0; i < materials->len; i++)
    {
      GthreeMaterial *src = g_ptr_array_index (materials, i);
      if (src)
        g_ptr_array_index (priv->materials, i) = g_object_ref (src);
    }
}

void
gthree_mesh_add_material (GthreeMesh *mesh,
                          GthreeMaterial *material)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  g_ptr_array_add (priv->materials, g_object_ref (material));
}

void
gthree_mesh_set_material (GthreeMesh *mesh,
                          int index,
                          GthreeMaterial *material)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  g_autoptr(GthreeMaterial) old_material = NULL;

  if (priv->materials->len < index + 1)
    g_ptr_array_set_size (priv->materials, index + 1);

  old_material = g_ptr_array_index (priv->materials, index);
  g_ptr_array_index (priv->materials, index) = g_object_ref (material);
}

GthreeGeometry *
gthree_mesh_get_geometry (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->geometry;
}

static void
gthree_mesh_class_init (GthreeMeshClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_mesh_set_property;
  gobject_class->get_property = gthree_mesh_get_property;
  gobject_class->finalize = gthree_mesh_finalize;

  object_class->in_frustum = gthree_mesh_in_frustum;
  object_class->update = gthree_mesh_update;
  object_class->fill_render_list = gthree_mesh_fill_render_list;
  object_class->raycast = gthree_mesh_raycast;

  obj_props[PROP_GEOMETRY] =
    g_param_spec_object ("geometry", "Geometry", "Geometry",
                         GTHREE_TYPE_GEOMETRY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MATERIALS] =
    g_param_spec_boxed ("materials", "Materials", "Materials",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}


GthreeDrawMode
gthree_mesh_get_draw_mode (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->draw_mode;
}

void
gthree_mesh_set_draw_mode (GthreeMesh *mesh,
                           GthreeDrawMode mode)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  priv->draw_mode = mode;
}
