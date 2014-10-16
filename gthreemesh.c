#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"
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
make_geometry_groups (GthreeGeometry *geometry,
                      gboolean use_face_material,
                      int max_vertices_in_group)
{
  guint i, counter, material_index, n_faces;
  guint group_hash;
  GthreeFace*face;
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
      face = gthree_geometry_get_face (geometry, i);
      material_index = use_face_material ? gthree_face_get_material_index (face) : 0;

      counter = 0;
      if (g_hash_table_lookup_extended (hash_map, GINT_TO_POINTER(material_index), NULL, &ptr))
        counter = GPOINTER_TO_INT (ptr);

      group_hash = material_index << 16 | counter;

      if (g_hash_table_lookup_extended (geometry_groups, GINT_TO_POINTER(group_hash), NULL, &ptr))
        group = ptr;
      else
        {
          group = gthree_geometry_group_new (material_index);
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
              group = gthree_geometry_group_new (material_index);
              g_hash_table_insert (geometry_groups, GINT_TO_POINTER(group_hash), group);
              g_ptr_array_add (groups, group);
            }
        }

      gthree_geometry_group_add_face (group, face);
    }

  g_hash_table_destroy (hash_map);
  g_hash_table_destroy (geometry_groups);

  return groups;
}

static GthreeMaterial *
get_buffer_material (GthreeMesh *mesh,
                     GthreeGeometryGroup *group)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  // TODO: handle MeshFaceMaterial
  return priv->material;
}

static gboolean
buffer_guess_uv_type (GthreeMaterial *material)
{
  return FALSE;
}

static GthreeShadingType
buffer_guess_normal_type (GthreeMaterial *material)
{
  return GTHREE_SHADING_NONE;
}

static GthreeColorType
buffer_guess_vertex_color_type (GthreeMaterial *material)
{
  return GTHREE_COLOR_NONE;
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
  //GthreeGeometry *geometry = priv->geometry;
  GPtrArray *faces = group->faces;
  guint nvertices = faces->len * 3;
  guint ntris     = faces->len * 1;
  guint nlines    = faces->len * 3;
  //GthreeMaterial *material = get_buffer_material (mesh, group);
  //gboolean uv_type = buffer_guess_uv_type (priv->material);
  gboolean normal_type = buffer_guess_normal_type (priv->material);
  //GthreeColorType vertex_color_type = buffer_guess_vertex_color_type (material);

  group->vertex_array = g_new (float, nvertices * 3);

  if (normal_type)
    group->normal_array = g_new (float, nvertices * 3);

  /*
  if (geometry.hasTangents) {
    group->tangentArray = g_new (float,  nvertices * 4 );
  }

  if ( vertex_color_type ) {
    group->colorArray = g_new (float,  nvertices * 3 );
  }
  if ( uv_type ) {
    if ( geometry.faceVertexUvs.length > 0 ) {
      group->uvArray = g_new (float,  nvertices * 2 );
    }

    if ( geometry.faceVertexUvs.length > 1 ) {
      group->uv2Array = g_new (float,  nvertices * 2 );
    }

  }

  if ( object.geometry.skinWeights.length && object.geometry.skinIndices.length ) {
    group->skinIndexArray = g_new (float,  nvertices * 4 );
    group->skinWeightArray = g_new (float,  nvertices * 4 );
  }
  */

  /* TODO: Handle uint32 extension */
  group->face_array = g_new0 (guint16, ntris * 3);
  group->line_array = g_new0 (guint16, nlines * 2);

  group->face_count = ntris * 3;
  group->line_count = nlines * 2;

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
  //gboolean uv_type = buffer_guess_uv_type (priv->material);
  GthreeShadingType normal_type = buffer_guess_normal_type (priv->material);
  GthreeColorType vertex_color_type = buffer_guess_vertex_color_type (material);
  gboolean needsSmoothNormals = normal_type == GTHREE_SHADING_SMOOTH;

  gboolean dirtyVertices = priv->verticesNeedUpdate;
  gboolean dirtyElements = priv->elementsNeedUpdate;
  //gboolean dirtyUvs = priv->uvsNeedUpdate;
  //gboolean dirtyNormals = priv->normalsNeedUpdate;
  //gboolean dirtyTangents = priv->tangentsNeedUpdate;
  gboolean dirtyColors = priv->colorsNeedUpdate;
  //gboolean dirtyMorphTargets = priv->morphTargetsNeedUpdate;

  guint vertexIndex = 0;
  guint offset = 0;
  guint offset_face = 0;
  guint offset_line = 0;
  int i;

  GPtrArray *faces = group->faces;

  const graphene_vec3_t *vertices = gthree_geometry_get_vertices (priv->geometry);

  g_assert (group->vertex_array);

  if (dirtyVertices)
    {
      for (i = 0; i < faces->len; i++)
        {
          GthreeFace *face = g_ptr_array_index (faces, i);

          graphene_vec3_to_float (&vertices[face->a], &group->vertex_array[offset]);
          graphene_vec3_to_float (&vertices[face->b], &group->vertex_array[offset + 4]);
          graphene_vec3_to_float (&vertices[face->c], &group->vertex_array[offset + 6]);

          offset += 9;
        }

      glBindBuffer (GL_ARRAY_BUFFER, GTHREE_BUFFER (group)->vertex_buffer);
      glBufferData (GL_ARRAY_BUFFER, faces->len * 3 * sizeof (float), group->vertex_array, hint);
    }

  if (dirtyColors && vertex_color_type != GTHREE_COLOR_NONE)
    {
      /* TODO
      for ( f = 0, fl = chunk_faces3.length; f < fl; f ++ )
        {
          face = obj_faces[ chunk_faces3[ f ] ];
          vertexColors = face.vertexColors;
          faceColor = face.color;
          if ( vertexColors.length === 3 && vertexColorType === THREE.VertexColors )
            {
              c1 = vertexColors[ 0 ];
              c2 = vertexColors[ 1 ];
              c3 = vertexColors[ 2 ];
            }
          else
            {
              c1 = faceColor;
              c2 = faceColor;
              c3 = faceColor;
            }

          colorArray[ offset_color ]     = c1.r;
          colorArray[ offset_color + 1 ] = c1.g;
          colorArray[ offset_color + 2 ] = c1.b;

          colorArray[ offset_color + 3 ] = c2.r;
          colorArray[ offset_color + 4 ] = c2.g;
          colorArray[ offset_color + 5 ] = c2.b;

          colorArray[ offset_color + 6 ] = c3.r;
          colorArray[ offset_color + 7 ] = c3.g;
          colorArray[ offset_color + 8 ] = c3.b;

          offset_color += 9;
        }

      if ( offset_color > 0 )
        {
          _gl.bindBuffer( _gl.ARRAY_BUFFER, geometryGroup.__webglColorBuffer );
          _gl.bufferData( _gl.ARRAY_BUFFER, colorArray, hint );
        }
      */
    }

  if (dirtyElements)
    {
      for (i = 0; i < faces->len; i++)
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

      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, GTHREE_BUFFER (group)->line_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, offset_line * sizeof (guint16), group->line_array, hint);
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

      material = get_buffer_material (mesh, group);

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
        make_geometry_groups (priv->geometry,
                              FALSE /* TODO material instanceof THREE.MeshFaceMaterial */,
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

static void
gthree_mesh_class_init (GthreeMeshClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_finalize;

  GTHREE_OBJECT_CLASS (klass)->update = gthree_mesh_update;
  GTHREE_OBJECT_CLASS (klass)->realize = gthree_mesh_realize;
  GTHREE_OBJECT_CLASS (klass)->unrealize = gthree_mesh_unrealize;
}
