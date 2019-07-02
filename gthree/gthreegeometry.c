#include <math.h>
#include <epoxy/gl.h>

#include "gthreegeometry.h"
#include "gthreeprivate.h"
#include "gthreemultimaterial.h"
#include "gthreelinebasicmaterial.h"
#include "gthreeobjectprivate.h"
#include "gthreeattribute.h"

typedef struct {
  GthreeAttribute *index;
  GthreeAttribute *wireframe_index;
  GHashTable *attributes; // intern string to GthreeAttribute
  GArray *groups;

  // map attributename e.g. "position" -> GPtrArray of alternate GthreeAttribute arrays for it
  // where GthreeAttribute.name == name of "emote", and array is always in same order
  GHashTable *morph_attributes;

  graphene_box_t bounding_box;
  graphene_sphere_t bounding_sphere;

  guint bounding_box_set;
  guint bounding_sphere_set;

  gint draw_range_start;
  gint draw_range_count;
} GthreeGeometryPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeGeometry, gthree_geometry, G_TYPE_OBJECT);

static void
drop_attribute (GthreeAttribute *attribute)
{
  if (attribute)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (attribute));
      g_object_unref (attribute);
    }
}

static void
gthree_geometry_init (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  priv->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify)drop_attribute);
  priv->groups = g_array_new (FALSE, TRUE, sizeof (GthreeGeometryGroup));

  priv->draw_range_start = 0;
  priv->draw_range_count = -1;
}

static void
gthree_geometry_finalize (GObject *obj)
{
  GthreeGeometry *geometry = GTHREE_GEOMETRY (obj);
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (priv->index)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->index));
  g_clear_object (&priv->index);
  if (priv->wireframe_index)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->wireframe_index));
  g_clear_object (&priv->wireframe_index);
  g_hash_table_unref (priv->attributes);
  if (priv->morph_attributes)
    g_hash_table_unref (priv->morph_attributes);
  g_array_unref (priv->groups);

  g_array_unref (geometry->influences);

  G_OBJECT_CLASS (gthree_geometry_parent_class)->finalize (obj);
}

static void
gthree_geometry_class_init (GthreeGeometryClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_geometry_finalize;
}

GthreeGeometry *
gthree_geometry_new ()
{
  GthreeGeometry *geometry;

  geometry = g_object_new (gthree_geometry_get_type (),
                         NULL);

  return geometry;
}

GthreeAttribute *
gthree_geometry_add_attribute (GthreeGeometry  *geometry,
                               const char *name,
                               GthreeAttribute *attribute)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  name = g_intern_string (name);

  g_hash_table_insert (priv->attributes, (char *)name, g_object_ref (attribute));
  gthree_resource_use (GTHREE_RESOURCE (attribute));

  return attribute;
}

void
gthree_geometry_remove_attribute (GthreeGeometry  *geometry,
                                  const char *name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_hash_table_remove (priv->attributes, name);
}

GthreeAttribute *
gthree_geometry_get_attribute (GthreeGeometry  *geometry,
                               const char *name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return g_hash_table_lookup (priv->attributes, name);
}

gboolean
gthree_geometry_has_attribute (GthreeGeometry  *geometry,
                               const char *name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return g_hash_table_lookup (priv->attributes, name) != NULL;
}


GthreeAttribute *
gthree_geometry_get_index (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->index;
}

GthreeAttribute *
gthree_geometry_get_wireframe_index (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  int i;

  if (priv->wireframe_index == NULL)
    {
      if (priv->index != NULL)
        {
          int orig_count = gthree_attribute_get_count (priv->index);
          priv->wireframe_index = gthree_attribute_new ("wireframeIndex",
                                                        GTHREE_ATTRIBUTE_TYPE_UINT32,
                                                        orig_count * 2, 1, FALSE);
          gthree_resource_use (GTHREE_RESOURCE (priv->wireframe_index));
          for (i = 0; i < orig_count; i += 3)
            {
              int a = gthree_attribute_get_uint (priv->index, i + 0);
              int b = gthree_attribute_get_uint (priv->index, i + 1);
              int c = gthree_attribute_get_uint (priv->index, i + 2);

              gthree_attribute_set_uint (priv->wireframe_index, i * 2 + 0, a);
              gthree_attribute_set_uint (priv->wireframe_index, i * 2 + 1, b);
              gthree_attribute_set_uint (priv->wireframe_index, i * 2 + 2, b);
              gthree_attribute_set_uint (priv->wireframe_index, i * 2 + 3, c);
              gthree_attribute_set_uint (priv->wireframe_index, i * 2 + 4, c);
              gthree_attribute_set_uint (priv->wireframe_index, i * 2 + 5, a);
            }
        }
      else
        {
          g_error ("TODO: Implement wgthree_geometry_get_wireframe_index for non-indexed geometries");
        }
    }

  return priv->wireframe_index;
}

void
gthree_geometry_set_index (GthreeGeometry  *geometry,
                           GthreeAttribute *index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_object_ref (index);
  gthree_resource_use (GTHREE_RESOURCE (index));

  if (priv->index)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->index));
  g_clear_object (&priv->index);
  if (priv->wireframe_index)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->wireframe_index));
  g_clear_object (&priv->wireframe_index);
  priv->index = index;
}

GthreeAttribute *
gthree_geometry_get_position (GthreeGeometry  *geometry)
{
  return gthree_geometry_get_attribute (geometry, "position");
}

int
gthree_geometry_get_position_count (GthreeGeometry *geometry)
{
  GthreeAttribute *position = gthree_geometry_get_position (geometry);
  if (position)
    return gthree_attribute_get_count (position);;
  return 0;
}

// I.e. number of indexes if indexed, and nr of positions otherwise
int
gthree_geometry_get_vertex_count (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeAttribute *position;

  if (priv->index)
    return gthree_attribute_get_count (priv->index);

  position = gthree_geometry_get_position (geometry);
  if (position)
    return gthree_attribute_get_count (position);

  return 0;
}

GthreeAttribute *
gthree_geometry_get_normal (GthreeGeometry  *geometry)
{
  return gthree_geometry_get_attribute (geometry, "normal");
}

GthreeAttribute *
gthree_geometry_get_color (GthreeGeometry  *geometry)
{
  return gthree_geometry_get_attribute (geometry, "color");
}

GthreeAttribute *
gthree_geometry_get_uv (GthreeGeometry  *geometry)
{
  return gthree_geometry_get_attribute (geometry, "uv");
}

void
gthree_geometry_add_morph_attribute (GthreeGeometry *geometry,
                                     const char *name,
                                     GthreeAttribute *attribute)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GPtrArray *attributes;

  if (priv->morph_attributes == NULL)
    priv->morph_attributes = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                    g_free, (GDestroyNotify)g_ptr_array_unref);

  attributes = g_hash_table_lookup (priv->morph_attributes, name);
  if (attributes == NULL)
    {
      attributes = g_ptr_array_new_with_free_func ((GDestroyNotify)drop_attribute);
      g_hash_table_insert (priv->morph_attributes, g_strdup (name), attributes);
    }

  g_ptr_array_add (attributes, g_object_ref (attribute));
  gthree_resource_use (GTHREE_RESOURCE (attribute));
}

void
gthree_geometry_remove_morph_attributes (GthreeGeometry *geometry,
                                         const char *name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (priv->morph_attributes == NULL)
    return;

  g_hash_table_remove (priv->morph_attributes, name);
}

GPtrArray *
gthree_geometry_get_morph_attributes (GthreeGeometry  *geometry,
                                      const char *name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (priv->morph_attributes == NULL)
    return NULL;

  return g_hash_table_lookup (priv->morph_attributes, name);
}

gboolean
gthree_geometry_has_morph_attributes (GthreeGeometry          *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return (priv->morph_attributes != NULL);
}

GList *
gthree_geometry_get_morph_attributes_names (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (priv->morph_attributes == NULL)
    return NULL;

  return g_hash_table_get_keys (priv->morph_attributes);
}

void
gthree_geometry_add_group (GthreeGeometry  *geometry,
                           int start,
                           int count,
                           int material_index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeGeometryGroup group = { start, count, material_index };

  g_array_append_val (priv->groups, group);
}

void
gthree_geometry_clear_groups (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  g_array_set_size (priv->groups, 0);

}

int
gthree_geometry_get_n_groups (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  return priv->groups->len;
}

GthreeGeometryGroup *
gthree_geometry_get_group (GthreeGeometry  *geometry, int index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  g_assert (index < priv->groups->len);

  return &g_array_index (priv->groups, GthreeGeometryGroup, index);
}

GthreeGeometryGroup *
gthree_geometry_peek_groups (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  return (GthreeGeometryGroup *)priv->groups->data;
}

int
gthree_geometry_get_draw_range_start (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  return priv->draw_range_start;
}

/* -1 means no max */
int
gthree_geometry_get_draw_range_count (GthreeGeometry  *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  return priv->draw_range_count;
}

/* count -1 means no max */
void
gthree_geometry_set_draw_range (GthreeGeometry  *geometry,
                                int start,
                                int count)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  priv->draw_range_start = start;
  priv->draw_range_count = count;
}


static float
distance_sq (const graphene_vec3_t *p1,
             const graphene_vec3_t *p2)
{
  graphene_vec3_t delta;

  graphene_vec3_subtract (p1, p2, &delta);
  return graphene_vec3_dot (&delta, &delta);
}

static void
expand_box_from_points (graphene_box_t *box,
                        GthreeAttribute *position)
{
  const float *floats, *f;
  int n_points = gthree_attribute_get_count (position);
  int stride = gthree_attribute_get_stride (position);
  int i;

  floats = gthree_attribute_peek_float (position);

  for (f = floats, i = 0; i < n_points; i++, f += stride)
    {
      const graphene_point3d_t *point = (const graphene_point3d_t *)f;
      graphene_vec3_t v;

      graphene_point3d_to_vec3 (point, &v);
      graphene_box_expand_vec3 (box, &v, box);
    }
}

static float
get_max_radius_sq_from_points (graphene_vec3_t *center,
                               GthreeAttribute *position)
{
  const float *floats, *f;
  int n_points = gthree_attribute_get_count (position);
  int stride = gthree_attribute_get_stride (position);
  int i;
  float max_radius_sq = 0.f;

  floats = gthree_attribute_peek_float (position);
  for (f = floats, i = 0; i < n_points; i++, f += stride)
    {
      const graphene_point3d_t *point = (const graphene_point3d_t *)f;
      graphene_vec3_t p;

      graphene_point3d_to_vec3 (point, &p);
      max_radius_sq = fmaxf (max_radius_sq, distance_sq (center, &p));
    }
  return max_radius_sq;
}


const graphene_sphere_t *
gthree_geometry_get_bounding_sphere  (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (!priv->bounding_sphere_set)
    {
      GthreeAttribute *position = gthree_geometry_get_position (geometry);
      GPtrArray *morph_attributes =gthree_geometry_get_morph_attributes (geometry, "position");

      if (position)
        {
          graphene_vec3_t size;
          graphene_vec3_t center;
          graphene_vec3_t min;
          graphene_point3d_t pt;
          graphene_box_t box;
          float max_radius_sq = 0.f;

          graphene_box_init_from_box (&box, graphene_box_empty ());
          expand_box_from_points (&box, position);
          if (morph_attributes)
            {
              for (int i = 0; i < morph_attributes->len; i++)
                {
                  GthreeAttribute *attr = g_ptr_array_index (morph_attributes, i);
                  expand_box_from_points (&box, attr);
                }
            }

          graphene_box_get_size (&box, &size);
          graphene_box_get_min (&box, &pt);
          graphene_point3d_to_vec3 (&pt, &min);

          graphene_vec3_scale (&size, 0.5f, &center);
          graphene_vec3_add (&center, &min, &center);

          max_radius_sq = get_max_radius_sq_from_points (&center, position);
          if (morph_attributes)
            {
              for (int i = 0; i < morph_attributes->len; i++)
                {
                  GthreeAttribute *attr = g_ptr_array_index (morph_attributes, i);
                  max_radius_sq = fmaxf (max_radius_sq, get_max_radius_sq_from_points (&center, attr));
                }
            }

          graphene_sphere_init (&priv->bounding_sphere,
                                graphene_point3d_init_from_vec3 (&pt, &center),
                                sqrtf (max_radius_sq));
        }
      else
        {
          graphene_point3d_t zero = { 0, 0, 0};
          graphene_sphere_init (&priv->bounding_sphere, &zero, 0);
        }

      priv->bounding_sphere_set = TRUE;
    }

  return &priv->bounding_sphere;
}

void
gthree_geometry_set_bounding_sphere  (GthreeGeometry          *geometry,
                                      const graphene_sphere_t *sphere)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  priv->bounding_sphere_set = TRUE;
  priv->bounding_sphere = *sphere;
}

void
gthree_geometry_normalize_normals (GthreeGeometry *geometry)
{
  GthreeAttribute *normal;
  int i, vertex_count;
  graphene_vec3_t n;
  graphene_point3d_t *r;

  normal = gthree_geometry_get_normal (geometry);
  if (normal == NULL)
    return;

  vertex_count = gthree_attribute_get_count (normal);

  // non-indexed elements (unconnected triangle soup)
  for (i = 0; i < vertex_count; i ++)
    {
      graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (normal, i), &n);
      graphene_vec3_normalize (&n, &n);
      r = gthree_attribute_peek_point3d_at (normal, i);
      r->x += graphene_vec3_get_x (&n);
      r->y += graphene_vec3_get_y (&n);
      r->z += graphene_vec3_get_z (&n);
    }
}

void
gthree_geometry_compute_vertex_normals (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeAttribute *position;
  GthreeAttribute *normal;
  int i, vertex_count;
  int vA, vB, vC;
  graphene_vec3_t pA, pB, pC, cb, ab;
  graphene_point3d_t *nA, *nB, *nC;

  position = gthree_geometry_get_position (geometry);
  if (position == NULL)
    return;

  vertex_count = gthree_attribute_get_count (position);

  normal = gthree_geometry_get_normal (geometry);
  if (normal == NULL)
    {
      normal = gthree_attribute_new ("normal", GTHREE_ATTRIBUTE_TYPE_FLOAT, vertex_count, 3, FALSE);
      gthree_geometry_add_attribute (geometry, "normal", normal);
      g_object_unref (normal); // Its owned by geometry anyway
    }
  else
    {
      // reset existing normals to zero
      for (i = 0; i < vertex_count; i++)
        gthree_attribute_set_xyz (normal, i, 0, 0, 0);
    }

  if (priv->index)
    {
      int index_count = gthree_attribute_get_count (priv->index);
      for (i = 0; i < index_count; i += 3 )
        {
          vA = gthree_attribute_get_uint (priv->index, i + 0);
          vB = gthree_attribute_get_uint (priv->index, i + 1);
          vC = gthree_attribute_get_uint (priv->index, i + 2);

          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, vA), &pA);
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, vB), &pB);
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, vC), &pC);

          graphene_vec3_subtract (&pC, &pB, &cb);
          graphene_vec3_subtract (&pA, &pB, &ab);
          graphene_vec3_cross (&cb, &ab, &cb);

          nA = gthree_attribute_peek_point3d_at (normal, vA);
          nA->x += graphene_vec3_get_x (&cb);
          nA->y += graphene_vec3_get_y (&cb);
          nA->z += graphene_vec3_get_z (&cb);

          nB = gthree_attribute_peek_point3d_at (normal, vB);
          nB->x += graphene_vec3_get_x (&cb);
          nB->y += graphene_vec3_get_y (&cb);
          nB->z += graphene_vec3_get_z (&cb);

          nC = gthree_attribute_peek_point3d_at (normal, vC);
          nC->x += graphene_vec3_get_x (&cb);
          nC->y += graphene_vec3_get_y (&cb);
          nC->z += graphene_vec3_get_z (&cb);
        }
    }
  else
    {
      // non-indexed elements (unconnected triangle soup)
      for (i = 0; i < vertex_count; i += 3)
        {
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, i + 0), &pA);
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, i + 1), &pB);
          graphene_point3d_to_vec3 (gthree_attribute_peek_point3d_at (position, i + 2), &pC);

          graphene_vec3_subtract (&pC, &pB, &cb);
          graphene_vec3_subtract (&pA, &pB, &ab);
          graphene_vec3_cross (&cb, &ab, &cb);

          nA = gthree_attribute_peek_point3d_at (normal, i + 0);
          nA->x += graphene_vec3_get_x (&cb);
          nA->y += graphene_vec3_get_y (&cb);
          nA->z += graphene_vec3_get_z (&cb);

          nB = gthree_attribute_peek_point3d_at (normal, i + 1);
          nB->x += graphene_vec3_get_x (&cb);
          nB->y += graphene_vec3_get_y (&cb);
          nB->z += graphene_vec3_get_z (&cb);

          nC = gthree_attribute_peek_point3d_at (normal, i + 2);
          nC->x += graphene_vec3_get_x (&cb);
          nC->y += graphene_vec3_get_y (&cb);
          nC->z += graphene_vec3_get_z (&cb);
        }
    }

  gthree_geometry_normalize_normals (geometry);
  gthree_attribute_set_needs_update (normal);
}

void
gthree_geometry_update (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeAttribute *attribute;
  GHashTableIter iter;

  if (priv->index)
    gthree_attribute_update (priv->index, GL_ELEMENT_ARRAY_BUFFER);
  if (priv->wireframe_index)
    gthree_attribute_update (priv->wireframe_index, GL_ELEMENT_ARRAY_BUFFER);

  g_hash_table_iter_init (&iter, priv->attributes);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&attribute))
    {
      // TODO: Only do this once per frame
      gthree_attribute_update (attribute, GL_ARRAY_BUFFER);
    }

  if (priv->morph_attributes != NULL)
    {
      GHashTableIter iter;
      gpointer key, value;
      int i;

      g_hash_table_iter_init (&iter, priv->morph_attributes);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          GPtrArray *array = value;
          for (i = 0; i < array->len; i++)
            {
              GthreeAttribute *attribute = g_ptr_array_index (array, i);

              // TODO: Only do this once per frame
              gthree_attribute_update (attribute, GL_ARRAY_BUFFER);
            }
        }
    }
}

void
gthree_geometry_fill_render_list (GthreeGeometry   *geometry,
                                  GthreeRenderList *list,
                                  GthreeMaterial   *material,
                                  GthreeObject     *object)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeMaterial *resolved_material;
  int i;

  if (GTHREE_IS_MULTI_MATERIAL (material) && priv->groups->len > 0)
    {
      for (i = 0; i < priv->groups->len; i++)
        {
          GthreeGeometryGroup *group = &g_array_index (priv->groups, GthreeGeometryGroup, i);

          resolved_material = gthree_material_resolve (material, group->material_index);
          if (resolved_material)
            gthree_render_list_push (list, object, geometry, resolved_material, group);
        }
    }
  else
    {
      resolved_material = gthree_material_resolve (material, 0);
      gthree_render_list_push (list, object, geometry, resolved_material, NULL);
    }
}

graphene_point3d_t *
parse_point3 (JsonArray *array,
              graphene_point3d_t *point)
{
  point->x = json_array_get_double_element (array, 0);
  point->y = json_array_get_double_element (array, 1);
  point->z = json_array_get_double_element (array, 2);
  return point;
}

graphene_sphere_t *
parse_sphere (JsonObject *obj,
              graphene_sphere_t *sphere)
{
  double radius = json_object_get_double_member (obj, "radius");
  JsonArray *centerj = json_object_get_array_member (obj, "center");
  graphene_point3d_t center;

  parse_point3 (centerj, &center);

  return graphene_sphere_init (sphere, &center, radius);
}

GthreeGeometry *
gthree_geometry_parse_json (JsonObject *root)
{
  JsonObject *data, *attributes = NULL;
  JsonArray *groups = NULL;
  JsonObject *index_j = NULL;
  g_autoptr(GthreeGeometry) geometry = NULL;
  GthreeGeometryPrivate *priv;

  geometry = gthree_geometry_new ();
  priv = gthree_geometry_get_instance_private (geometry);

  if (json_object_has_member (root, "isInstancedBufferGeometry"))
    g_error ("instanced buffers not supported");

  data = json_object_get_object_member (root, "data");

  if (json_object_has_member (data, "index"))
    index_j = json_object_get_object_member (data, "index");

  if (index_j != NULL)
    {
      g_autoptr(GthreeAttribute) index = gthree_attribute_parse_json (index_j, "index");
      gthree_geometry_set_index (geometry, index);
    }

  if (json_object_has_member (data, "attributes"))
    {
      g_autoptr(GList) members = NULL;
      GList *l;

      attributes = json_object_get_object_member (data, "attributes");

      if (attributes != NULL)
        members = json_object_get_members (attributes);

      for (l = members; l != NULL; l = l->next)
        {
          const char *name = l->data;
          JsonObject *attribute_j = json_object_get_object_member (attributes, name);
          g_autoptr(GthreeAttribute) attribute = gthree_attribute_parse_json (attribute_j, name);
          gthree_geometry_add_attribute (geometry, name, attribute);
        }
    }

  // TODO: Handle json.data.morphAttributes

  if (json_object_has_member (data, "groups"))
    groups = json_object_get_array_member (data, "groups");
  else if (json_object_has_member (data, "drawcalls"))
    groups = json_object_get_array_member (data, "drawcalls");
  else if (json_object_has_member (data, "offsets"))
    groups = json_object_get_array_member (data, "offsets");

  if (groups)
    {
      guint len = json_array_get_length (groups);
      guint i;

      for (i = 0; i < len; i++)
        {
          JsonObject *group = json_array_get_object_element  (groups, i);
          gint64 start = json_object_get_int_member (group, "start");
          gint64 count = json_object_get_int_member (group, "count");
          gint64 material_index = json_object_get_int_member (group, "materialIndex");

          gthree_geometry_add_group (geometry, start, count, material_index);

        }
    }

  if (json_object_has_member (data, "boundingSphere"))
    {
      JsonObject *bs = json_object_get_object_member (data, "boundingSphere");

      parse_sphere (bs, &priv->bounding_sphere);
      priv->bounding_sphere_set = TRUE;
    }

  // TODO parse root.name

  return g_steal_pointer (&geometry);
}
