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
  GPtrArray *attributes;
  GArray *groups;

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
  gthree_resource_unuse (GTHREE_RESOURCE (attribute));
  g_object_unref (attribute);
}

static void
gthree_geometry_init (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  priv->attributes = g_ptr_array_new_with_free_func ((GDestroyNotify)drop_attribute);
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
  g_ptr_array_unref (priv->attributes);
  g_array_unref (priv->groups);

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
                               GthreeAttribute *attribute)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeAttributeName name;
  GthreeAttribute *old_attribute;

  name = gthree_attribute_get_name (attribute);
  if (name == GTHREE_ATTRIBUTE_NAME_INDEX)
    {
      g_warning ("Use gthree_geometry_set_index to add an index");
      gthree_geometry_set_index (geometry, attribute);
      return attribute;
    }

  if (priv->attributes->len <= name)
    g_ptr_array_set_size (priv->attributes, name + 1);

  old_attribute = g_ptr_array_index (priv->attributes, name);
  if (old_attribute)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (old_attribute));
      g_object_unref (old_attribute);
    }
  gthree_resource_use (GTHREE_RESOURCE (attribute));
  g_ptr_array_index (priv->attributes, name) = g_object_ref (attribute);

  return attribute;
}

void
gthree_geometry_remove_attribute (GthreeGeometry  *geometry,
                                  GthreeAttributeName name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeAttribute *old_attribute;

  if (priv->attributes->len <= name)
    return;

  old_attribute = g_ptr_array_index (priv->attributes, name);
  if (old_attribute)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (old_attribute));
      g_object_unref (old_attribute);
    }
  g_ptr_array_index (priv->attributes, name) = NULL;
}

GthreeAttribute *
gthree_geometry_get_attribute (GthreeGeometry  *geometry,
                               GthreeAttributeName name)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (priv->attributes->len <= name)
    return NULL;

  return g_ptr_array_index (priv->attributes, name);
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
  return gthree_geometry_get_attribute (geometry, GTHREE_ATTRIBUTE_NAME_POSITION);
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
  return gthree_geometry_get_attribute (geometry, GTHREE_ATTRIBUTE_NAME_NORMAL);
}

GthreeAttribute *
gthree_geometry_get_color (GthreeGeometry  *geometry)
{
  return gthree_geometry_get_attribute (geometry, GTHREE_ATTRIBUTE_NAME_COLOR);
}

GthreeAttribute *
gthree_geometry_get_uv (GthreeGeometry  *geometry)
{
  return gthree_geometry_get_attribute (geometry, GTHREE_ATTRIBUTE_NAME_UV);
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

const graphene_sphere_t *
gthree_geometry_get_bounding_sphere  (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (!priv->bounding_sphere_set)
    {
      GthreeAttribute *position = gthree_geometry_get_position (geometry);

      if (position)
        {
          GthreeAttributeArray *array = gthree_attribute_get_array (position);

          /* TODO: This assumes the positions are not interleaved... */
          graphene_sphere_init_from_points (&priv->bounding_sphere,
                                            gthree_attribute_array_get_count (array),
                                            gthree_attribute_array_peek_point3d (array),
                                            NULL);

          /* TODO: The three.js code does a lot of special handling for morphing here too */
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
gthree_geometry_compute_vertex_normals (GthreeGeometry *geometry,
                                        gboolean area_weighted)
{
  g_warning ("gthree_geometry_compute_vertex_normals");
#if 0
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;
  const graphene_vec3_t *vertices;
  graphene_vec3_t *vertex_normals;
  int i, n_faces, n_vertices;

  n_faces = gthree_geometry_get_n_faces (geometry);
  n_vertices = gthree_geometry_get_n_vertices (geometry);
  vertices = gthree_geometry_get_vertices (geometry);
  vertex_normals = g_new0 (graphene_vec3_t, n_vertices);

  if (area_weighted)
    {
      for (i = 0; i < n_faces; i++)
        {
          face = &g_array_index (priv->faces, GthreeFace, i);
          const graphene_vec3_t *va, *vb, *vc;
          graphene_vec3_t cb, ab;

          // vertex normals weighted by triangle areas
          // http://www.iquilezles.org/www/articles/normals/normals.htm

          va = &vertices[face->a];
          vb = &vertices[face->b];
          vc = &vertices[face->c];

          graphene_vec3_subtract (vc, vb, &cb);
          graphene_vec3_subtract (va, vb, &ab);
          graphene_vec3_cross (&cb, &ab, &cb);

          graphene_vec3_add (&vertex_normals[face->a], &cb, &vertex_normals[face->a]);
          graphene_vec3_add (&vertex_normals[face->b], &cb, &vertex_normals[face->b]);
          graphene_vec3_add (&vertex_normals[face->c], &cb, &vertex_normals[face->c]);
        }
    }
  else
    {
      for (i = 0; i < n_faces; i++)
        {
          face = &g_array_index (priv->faces, GthreeFace, i);

          graphene_vec3_add (&vertex_normals[face->a], &face->normal, &vertex_normals[face->a]);
          graphene_vec3_add (&vertex_normals[face->b], &face->normal, &vertex_normals[face->b]);
          graphene_vec3_add (&vertex_normals[face->c], &face->normal, &vertex_normals[face->c]);
        }
    }

  for (i = 0; i < n_vertices; i++)
    graphene_vec3_normalize (&vertex_normals[i], &vertex_normals[i]);

  for (i = 0; i < n_faces; i++)
    {
      face = &g_array_index (priv->faces, GthreeFace, i);

      if (face->vertex_normals == NULL)
        face->vertex_normals = g_new (graphene_vec3_t, 3);

      face->vertex_normals[0] = vertex_normals[face->a];
      face->vertex_normals[1] = vertex_normals[face->b];
      face->vertex_normals[2] = vertex_normals[face->c];
    }

  g_free (vertex_normals);
#endif
}

void
gthree_geometry_update (GthreeGeometry *geometry,
                        GthreeMaterial *material)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  int i;

  if (priv->index)
    gthree_attribute_update (priv->index, GL_ELEMENT_ARRAY_BUFFER);
  if (priv->wireframe_index)
    gthree_attribute_update (priv->wireframe_index, GL_ELEMENT_ARRAY_BUFFER);

  for (i = 0; i < priv->attributes->len; i++)
    {
      GthreeAttribute *attribute = g_ptr_array_index (priv->attributes, i);

      if (attribute == NULL)
        continue;

      // TODO: Only do this once per frame
      gthree_attribute_update (attribute, GL_ARRAY_BUFFER);
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
