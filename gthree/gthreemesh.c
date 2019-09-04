#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"
#include "gthreemeshmaterial.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"
#include "gthreeraycaster.h"

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
                    const graphene_triangle_t *triangle,
                    graphene_point3d_t *local_intersection_point)
{
  GthreeSide side = GTHREE_SIDE_FRONT;
  graphene_ray_intersection_kind_t kind;
  graphene_point3d_t world_intersection_point;
  float t, distance;
  graphene_point3d_t world_origin;
  GthreeRayIntersection *intersection;

  if (material)
    side = gthree_material_get_side (material);

  kind = graphene_ray_intersect_triangle (local_ray, triangle, &t);

  switch (side)
    {
    case GTHREE_SIDE_FRONT:
      if (kind != GRAPHENE_RAY_INTERSECTION_KIND_ENTER)
        return NULL;
      break;
    case GTHREE_SIDE_BACK:
      if (kind != GRAPHENE_RAY_INTERSECTION_KIND_LEAVE)
        return NULL;
      break;
    default: // double sided
      if (kind == GRAPHENE_RAY_INTERSECTION_KIND_NONE)
        return NULL;
      break;
    }

  graphene_ray_get_position_at (local_ray, t, local_intersection_point);

  graphene_matrix_transform_point3d (gthree_object_get_world_matrix (object),
                                     local_intersection_point, &world_intersection_point);

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
  graphene_vec2_t uvA, uvB, uvC;
  graphene_vec3_t morphA, morphB, morphC;
  GthreeRayIntersection *intersection;
  graphene_triangle_t triangle;
  graphene_point3d_t local_intersection_point;

  if (material != NULL &&
      GTHREE_IS_MESH_MATERIAL (material) &&
      gthree_mesh_material_get_morph_targets (GTHREE_MESH_MATERIAL (material)) &&
      morph_position &&  priv->morph_target_influences)
    {
      graphene_vec3_t vA, vB, vC;

      graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, a), &vA);
      graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, b), &vB);
      graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, c), &vC);

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

      graphene_triangle_init_from_vec3 (&triangle, &vA, &vB, &vC);
    }
  else
    {
      graphene_triangle_init_from_float (&triangle,
                                         gthree_attribute_peek_float_at (position, a),
                                         gthree_attribute_peek_float_at (position, b),
                                         gthree_attribute_peek_float_at (position, c));
    }

  intersection = check_intersection (object, material, raycaster, local_ray, &triangle, &local_intersection_point);
  if (intersection)
    {
      intersection->face_index = face_index;
      intersection->face = triangle; // NOTE: In object coords, like three.js
      intersection->material_index = material_index;

      if (uv)
        {
          gthree_attribute_get_vec2 (uv, a, &uvA);
          gthree_attribute_get_vec2 (uv, b, &uvB);
          gthree_attribute_get_vec2 (uv, c, &uvC);

          graphene_triangle_get_uv (&intersection->face,
                                    &local_intersection_point,
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
  if (!graphene_ray_intersects_sphere (world_ray, &world_sphere))
    return;

  graphene_matrix_inverse (gthree_object_get_world_matrix (object), &inverse_matrix);
  graphene_matrix_transform_ray (&inverse_matrix, world_ray, &local_ray);

  // Check boundingBox before continuing
  if (!graphene_ray_intersects_box (&local_ray, gthree_geometry_get_bounding_box  (priv->geometry)))
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
