#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"
#include "gthreemultimaterial.h"
#include "gthreebasicmaterial.h"
#include "gthreegeometrygroupprivate.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;

  GPtrArray *groups; /* GthreeGeometryGroup * */

  guint verticesNeedUpdate : 1;
  guint morphTargetsNeedUpdate : 1;
  guint elementsNeedUpdate : 1;
  guint uvsNeedUpdate : 1;
  guint normalsNeedUpdate : 1;
  guint tangentsNeedUpdate : 1;
  guint colorsNeedUpdate : 1;

} GthreeMeshPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMesh, gthree_mesh, GTHREE_TYPE_OBJECT);

GthreeMesh *
gthree_mesh_new (GthreeGeometry *geometry,
                 GthreeMaterial *material)
{
  GthreeMesh *mesh;
  GthreeMeshPrivate *priv;

  // TODO: properties
  mesh = g_object_new (gthree_mesh_get_type (),
                         NULL);

  priv = gthree_mesh_get_instance_private (mesh);

  priv->geometry = g_object_ref (geometry);
  priv->material = g_object_ref (material);

  return mesh;
}

static void
gthree_mesh_init (GthreeMesh *mesh)
{
  //GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

}

static void
gthree_mesh_finalize (GObject *obj)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_mesh_parent_class)->finalize (obj);
}

static GPtrArray *
make_geometry_groups (GthreeMesh *mesh,
                      GthreeGeometry *geometry,
                      gboolean use_face_material,
                      int max_vertices_in_group)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  guint i, counter, material_index, n_faces;
  guint group_hash;
  GHashTable *hash_map, *geometry_groups;
  GthreeGeometryGroup *group;
  gpointer ptr;
  GPtrArray *groups;

  groups = g_ptr_array_new_with_free_func (g_object_unref);

  hash_map = g_hash_table_new (g_direct_hash, g_direct_equal);
  geometry_groups = g_hash_table_new (g_direct_hash, g_direct_equal);

  n_faces = gthree_geometry_get_n_faces (geometry);
  for (i = 0; i < n_faces; i++)
    {
      material_index = use_face_material ? gthree_geometry_face_get_material_index (geometry, i) : 0;

      counter = 0;
      if (g_hash_table_lookup_extended (hash_map, GINT_TO_POINTER(material_index), NULL, &ptr))
        counter = GPOINTER_TO_INT (ptr);

      group_hash = material_index << 16 | counter;

      if (g_hash_table_lookup_extended (geometry_groups, GINT_TO_POINTER(group_hash), NULL, &ptr))
        group = ptr;
      else
        {
          group = gthree_geometry_group_new (GTHREE_OBJECT (mesh), priv->material, material_index);
          g_hash_table_insert (geometry_groups, GINT_TO_POINTER(group_hash), group);
          g_ptr_array_add (groups, group);
        }

      if (group->n_vertices + 3 > max_vertices_in_group)
        {
          counter += 1;
          g_hash_table_replace (hash_map, GINT_TO_POINTER(material_index), GINT_TO_POINTER (counter));

          group_hash = material_index << 16 | counter;

          if (g_hash_table_lookup_extended (geometry_groups, GINT_TO_POINTER(group_hash), NULL, &ptr))
            group = ptr;
          else
            {
              group = gthree_geometry_group_new (GTHREE_OBJECT (mesh), priv->material, material_index);
              g_hash_table_insert (geometry_groups, GINT_TO_POINTER(group_hash), group);
              g_ptr_array_add (groups, group);
            }
        }

      gthree_geometry_group_add_face (group, i);
    }

  g_hash_table_destroy (hash_map);
  g_hash_table_destroy (geometry_groups);

  return groups;
}

static void
create_mesh_buffers (GthreeGeometryGroup *group)
{
  GthreeBuffer *buffer;

  buffer = GTHREE_BUFFER (group);
  glGenBuffers (1, &buffer->vertex_buffer);
  glGenBuffers (1, &buffer->normal_buffer);
  glGenBuffers (1, &buffer->tangent_buffer);
  glGenBuffers (1, &buffer->color_buffer);
  glGenBuffers (1, &buffer->uv_buffer);
  glGenBuffers (1, &buffer->uv2_buffer);

  glGenBuffers (1, &buffer->face_buffer);
  glGenBuffers (1, &buffer->line_buffer);
}

static void
init_mesh_buffers (GthreeMesh *mesh,
                   GthreeGeometryGroup *group)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  GthreeGeometry *geometry = priv->geometry;
  GArray *face_indexes = group->face_indexes;
  guint nvertices = face_indexes->len * 3;
  guint ntris     = face_indexes->len * 1;
  guint nlines    = face_indexes->len * 3;
  GthreeMaterial *material = gthree_buffer_resolve_material (GTHREE_BUFFER(group));
  gboolean uv_type = gthree_material_needs_uv (material);
  gboolean normal_type = gthree_material_needs_normals (material);
  GthreeColorType vertex_color_type = gthree_material_needs_colors (material);

  group->vertex_array = g_new (float, nvertices * 3);

  if (normal_type)
    group->normal_array = g_new (float, nvertices * 3);

  /*
  if (geometry.hasTangents) {
    group->tangentArray = g_new (float,  nvertices * 4 );
  }
  */

  if (vertex_color_type != GTHREE_COLOR_NONE)
    group->color_array = g_new (float, nvertices * 3);

  if (uv_type)
    {
      if (gthree_geometry_get_n_uv (geometry) > 0)
        group->uv_array = g_new (float, nvertices * 2);

      if (gthree_geometry_get_n_uv2 (geometry) > 0)
        group->uv2_array = g_new (float, nvertices * 2);
    }

  /*
  if ( object.geometry.skinWeights.length && object.geometry.skinIndices.length ) {
    group->skinIndexArray = g_new (float,  nvertices * 4 );
    group->skinWeightArray = g_new (float,  nvertices * 4 );
  }
  */

  /* TODO: Handle uint32 extension */
  group->face_array = g_new0 (guint16, ntris * 3);
  group->line_array = g_new0 (guint16, nlines * 2);

  /*
  // custom attributes
  if ( material.attributes ) {
    if ( group->webglCustomAttributesList === undefined ) {
      group->webglCustomAttributesList = [];
    }

    for ( var a in material.attributes ) {
      // Do a shallow copy of the attribute object so different group chunks use different
      // attribute buffers which are correctly indexed in the setMeshBuffers function

      var originalAttribute = material.attributes[ a ];
      var attribute = {};

      for ( var property in originalAttribute ) {
        attribute[ property ] = originalAttribute[ property ];
      }

      if ( ! attribute.__webglInitialized || attribute.createUniqueBuffers ) {
        attribute.__webglInitialized = true;

        var size = 1;   // "f" and "i"

        if ( attribute.type === 'v2' ) size = 2;
        else if ( attribute.type === 'v3' ) size = 3;
        else if ( attribute.type === 'v4' ) size = 4;
        else if ( attribute.type === 'c'  ) size = 3;

        attribute.size = size;

        attribute.array = g_new (float,  nvertices * size );

        attribute.buffer = _gl.createBuffer();
        attribute.buffer.belongsToAttribute = a;

        originalAttribute.needsUpdate = true;
        attribute.__original = originalAttribute;

      }

      group->webglCustomAttributesList.push( attribute );

    }

  }
  */

};

static void
set_mesh_buffers (GthreeMesh *mesh,
                  GthreeGeometryGroup *group,
                  int hint,
                  gboolean dispose,
                  GthreeMaterial *material)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  GthreeGeometry *geometry = priv->geometry;
  gboolean uv_type = gthree_material_needs_uv (material);
  gboolean normal_type = gthree_material_needs_normals (material);
  GthreeColorType vertex_color_type = gthree_material_needs_colors (material);
  gboolean needs_smooth_normals = normal_type == GTHREE_SHADING_SMOOTH;

  gboolean dirtyVertices = priv->verticesNeedUpdate;
  gboolean dirtyElements = priv->elementsNeedUpdate;
  gboolean dirtyUvs = priv->uvsNeedUpdate;
  gboolean dirtyNormals = priv->normalsNeedUpdate;
  //gboolean dirtyTangents = priv->tangentsNeedUpdate;
  gboolean dirtyColors = priv->colorsNeedUpdate;
  //gboolean dirtyMorphTargets = priv->morphTargetsNeedUpdate;

  guint vertexIndex = 0;
  guint offset = 0;
  guint offset_face = 0;
  guint offset_line = 0;
  guint offset_color = 0;
  guint offset_uv = 0;
  guint offset_uv2 = 0;
  guint offset_normal = 0;
  int i;

  GArray *face_indexes = group->face_indexes;

  const graphene_vec3_t *vertices = gthree_geometry_get_vertices (geometry);

  g_assert (group->vertex_array);

  if (dirtyVertices)
    {
      for (i = 0; i < face_indexes->len; i++)
        {
          int face_index = g_array_index (face_indexes, int, i);
          int a = gthree_geometry_face_get_a (geometry, face_index);
          int b = gthree_geometry_face_get_b (geometry, face_index);
          int c = gthree_geometry_face_get_c (geometry, face_index);

          graphene_vec3_to_float (&vertices[a], &group->vertex_array[offset]);
          graphene_vec3_to_float (&vertices[b], &group->vertex_array[offset + 3]);
          graphene_vec3_to_float (&vertices[c], &group->vertex_array[offset + 6]);

          offset += 9;
        }

      glBindBuffer (GL_ARRAY_BUFFER, GTHREE_BUFFER (group)->vertex_buffer);
      glBufferData (GL_ARRAY_BUFFER, offset * sizeof (float), group->vertex_array, hint);
    }

  if (dirtyColors && vertex_color_type != GTHREE_COLOR_NONE)
    {
      for (i = 0; i < face_indexes->len; i++)
        {
          int face = g_array_index (face_indexes, int, i);
          const GdkRGBA *c1, *c2, *c3;

          if (!gthree_geometry_face_get_vertex_colors (geometry, face, &c1, &c2, &c3) ||
	      vertex_color_type != GTHREE_COLOR_VERTEX)
            {
              c1 = gthree_geometry_face_get_color (geometry, face);
              c2 = c1;
              c3 = c1;
            }

          group->color_array[offset_color]     = c1->red;
          group->color_array[offset_color + 1] = c1->green;
          group->color_array[offset_color + 2] = c1->blue;

          group->color_array[offset_color + 3] = c2->red;
          group->color_array[offset_color + 4] = c2->green;
          group->color_array[offset_color + 5] = c2->blue;

          group->color_array[offset_color + 6] = c3->red;
          group->color_array[offset_color + 7] = c3->green;
          group->color_array[offset_color + 8] = c3->blue;

          offset_color += 9;
        }

      if (offset_color > 0)
        {
          glBindBuffer (GL_ARRAY_BUFFER, GTHREE_BUFFER (group)->color_buffer);
          glBufferData (GL_ARRAY_BUFFER, offset_color * sizeof (float), group->color_array, hint);
        }
    }

  if (dirtyNormals && normal_type != GTHREE_SHADING_NONE)
    {
      for (i = 0; i < face_indexes->len; i++)
        {
          int face_index = g_array_index (face_indexes, int, i);
	  int j;
	  const graphene_vec3_t *vns[3];

          if (gthree_geometry_face_get_vertex_normals (geometry, face_index, &vns[0], &vns[1], &vns[2]) && needs_smooth_normals)
            {
              for (j = 0; j < 3; j++)
                {
                  const graphene_vec3_t *vn = vns[j];

                  group->normal_array[offset_normal    ] = graphene_vec3_get_x (vn);
                  group->normal_array[offset_normal + 1] = graphene_vec3_get_y (vn);
                  group->normal_array[offset_normal + 2] = graphene_vec3_get_z (vn);

                  offset_normal += 3;
                }
            }
          else
            {
              for (j = 0; j < 3; j ++)
                {
                  const graphene_vec3_t *vn = gthree_geometry_face_get_normal (geometry, face_index);

                  group->normal_array[offset_normal    ] = graphene_vec3_get_x (vn);
                  group->normal_array[offset_normal + 1] = graphene_vec3_get_y (vn);
                  group->normal_array[offset_normal + 2] = graphene_vec3_get_z (vn);

                  offset_normal += 3;
                }
            }
        }

      if (offset_normal > 0)
        {
          glBindBuffer (GL_ARRAY_BUFFER, GTHREE_BUFFER (group)->normal_buffer);
          glBufferData (GL_ARRAY_BUFFER, offset_normal * sizeof (float), group->normal_array, hint);
        }
    }

  if (dirtyUvs && gthree_geometry_get_n_uv (geometry) > 0 && uv_type)
    {
      int n_uv = gthree_geometry_get_n_uv (geometry);
      const graphene_vec2_t *uvs = gthree_geometry_get_uvs (geometry);

      for (i = 0; i < face_indexes->len; i++)
        {
          int face_index = g_array_index (face_indexes, int, i);
          int j;

          for (j = 0; j < 3; j ++)
            {
              int uvi = face_index * 3 + j;
              const graphene_vec2_t *v;

              if (uvi >= n_uv)
                continue;

              v = &uvs[uvi];

              group->uv_array[offset_uv] = graphene_vec2_get_x (v);
              group->uv_array[offset_uv + 1] = graphene_vec2_get_y (v);
              offset_uv += 2;
            }
        }

      if (offset_uv > 0)
        {
          glBindBuffer (GL_ARRAY_BUFFER, GTHREE_BUFFER (group)->uv_buffer);
          glBufferData (GL_ARRAY_BUFFER, offset_uv * sizeof (float), group->uv_array, hint);
        }
    }

  if (dirtyUvs && gthree_geometry_get_n_uv2 (geometry) > 0 && uv_type)
    {
      int n_uv2 = gthree_geometry_get_n_uv2 (geometry);
      const graphene_vec2_t *uvs2 = gthree_geometry_get_uv2s (geometry);

      for (i = 0; i < face_indexes->len; i++)
        {
          int face_index = g_array_index (face_indexes, int, i);
          int j;

          for (j = 0; j < 3; j ++)
            {
              int uvi = face_index * 3 + j;
              const graphene_vec2_t *v;

              if (uvi >= n_uv2)
                continue;

              v = &uvs2[uvi];

              group->uv2_array[offset_uv2] = graphene_vec2_get_x (v);
              group->uv2_array[offset_uv2 + 1] = graphene_vec2_get_y (v);
              offset_uv2 += 2;
            }
        }

      if (offset_uv2 > 0)
        {
          glBindBuffer (GL_ARRAY_BUFFER, GTHREE_BUFFER (group)->uv2_buffer);
          glBufferData (GL_ARRAY_BUFFER, offset_uv2 * sizeof (float), group->uv2_array, hint);
        }
    }

#ifdef TODO
  // dirtyTangents
  // dirtyMorphTargets
  // obj_skinWeights.length
  // custom attributes
#endif

  if (dirtyElements)
    {
      for (i = 0; i < face_indexes->len; i++)
        {
          group->face_array[offset_face]   = vertexIndex;
          group->face_array[offset_face + 1] = vertexIndex + 1;
          group->face_array[offset_face + 2] = vertexIndex + 2;

          offset_face += 3;

          group->line_array[offset_line]     = vertexIndex;
          group->line_array[offset_line + 1] = vertexIndex + 1;

          group->line_array[offset_line + 2] = vertexIndex;
          group->line_array[offset_line + 3] = vertexIndex + 2;

          group->line_array[offset_line + 4] = vertexIndex + 1;
          group->line_array[offset_line + 5] = vertexIndex + 2;

          offset_line += 6;

          vertexIndex += 3;
        }

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GTHREE_BUFFER (group)->face_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, offset_face * sizeof (guint16), group->face_array, hint);
      GTHREE_BUFFER(group)->face_count = offset_face;

      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, GTHREE_BUFFER (group)->line_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, offset_line * sizeof (guint16), group->line_array, hint);
      GTHREE_BUFFER(group)->line_count = offset_line;
    }

  if (dispose)
    gthree_geometry_group_dispose (group);
}

static void
gthree_mesh_update (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  GthreeGeometryGroup *group;
  GthreeMaterial *material;
  int i;

  //geometryGroup, customAttributesDirty, material;

  for (i = 0; i < priv->groups->len; i++)
    {
      group = g_ptr_array_index (priv->groups, i);

      material = gthree_buffer_resolve_material (GTHREE_BUFFER(group));

      //customAttributesDirty = material.attributes && areCustomAttributesDirty( material );
      if ( priv->verticesNeedUpdate || priv->morphTargetsNeedUpdate || priv->elementsNeedUpdate ||
           priv->uvsNeedUpdate || priv->normalsNeedUpdate ||
           priv->colorsNeedUpdate || priv->tangentsNeedUpdate /* || customAttributesDirty*/ )
        set_mesh_buffers (mesh, group, GL_DYNAMIC_DRAW, TRUE /*! geometry.dynamic*/, material);
    }

  priv->verticesNeedUpdate = FALSE;
  priv->morphTargetsNeedUpdate = FALSE;
  priv->elementsNeedUpdate = FALSE;
  priv->uvsNeedUpdate = FALSE;
  priv->normalsNeedUpdate = FALSE;
  priv->tangentsNeedUpdate = FALSE;
  priv->colorsNeedUpdate = FALSE;

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_mesh_realize (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  GthreeGeometryGroup *group;
  int i;
  gboolean realized;

  if (!priv->groups)
    {
      gthree_object_remove_buffers (object);

      priv->groups =
        make_geometry_groups (mesh, priv->geometry,
                              GTHREE_IS_MULTI_MATERIAL(priv->material),
                              65535 /* TODO_glExtensionElementIndexUint ? 4294967296 : 65535 */);
    }

  realized = FALSE;
  for (i = 0; i < priv->groups->len; i++)
    {
      group = g_ptr_array_index (priv->groups, i);

      if (!group->realized)
        {
          group->realized = TRUE;
          realized = TRUE;

          create_mesh_buffers (group);
          init_mesh_buffers (mesh, group);

          gthree_object_add_buffer (object, GTHREE_BUFFER(group));
        }
    }

  if (realized)
    {
      priv->verticesNeedUpdate = TRUE;
      priv->morphTargetsNeedUpdate = TRUE;
      priv->elementsNeedUpdate = TRUE;
      priv->uvsNeedUpdate = TRUE;
      priv->normalsNeedUpdate = TRUE;
      priv->tangentsNeedUpdate = TRUE;
      priv->colorsNeedUpdate = TRUE;
    }
}

static void
gthree_mesh_unrealize (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  if (priv->groups)
    g_ptr_array_free (priv->groups, TRUE);
}

static gboolean
gthree_mesh_in_frustum (GthreeObject *object,
                        GthreeFrustum *frustum)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  GthreeSphere sphere;

  sphere = *gthree_geometry_get_bounding_sphere (priv->geometry);

  gthree_sphere_transform (&sphere, gthree_object_get_world_matrix (object));

  return gthree_frustum_intersects_sphere (frustum, &sphere);
}

static void
gthree_mesh_class_init (GthreeMeshClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_finalize;

  GTHREE_OBJECT_CLASS (klass)->in_frustum = gthree_mesh_in_frustum;
  GTHREE_OBJECT_CLASS (klass)->update = gthree_mesh_update;
  GTHREE_OBJECT_CLASS (klass)->realize = gthree_mesh_realize;
  GTHREE_OBJECT_CLASS (klass)->unrealize = gthree_mesh_unrealize;
}
