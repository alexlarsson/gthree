#include <math.h>
#include <epoxy/gl.h>

#include "gthreegeometrygroupprivate.h"
#include "gthreegeometry.h"

G_DEFINE_TYPE (GthreeGeometryGroup, gthree_geometry_group, GTHREE_TYPE_BUFFER);

GthreeGeometryGroup *
gthree_geometry_group_new (GthreeGeometry *geometry, guint32 material_index)
{
  GthreeGeometryGroup *group;

  group = g_object_new (gthree_geometry_group_get_type (),
                        NULL);

  group->geometry = geometry;
  group->parent.material_index = material_index;

  return group;
}

static void
gthree_geometry_group_init (GthreeGeometryGroup *group)
{
  group->face_indexes = g_array_new (FALSE, FALSE, sizeof (int));
}

void
gthree_geometry_group_dispose (GthreeGeometryGroup *group)
{
  g_clear_pointer (&group->vertex_array, g_free);
  g_clear_pointer (&group->normal_array, g_free);
  g_clear_pointer (&group->tangent_array, g_free);
  g_clear_pointer (&group->color_array, g_free);
  g_clear_pointer (&group->uv_array, g_free);
  g_clear_pointer (&group->uv2_array, g_free);
  g_clear_pointer (&group->face_array, g_free);
  g_clear_pointer (&group->line_array, g_free);
}

static void
gthree_geometry_group_finalize (GObject *obj)
{
  GthreeGeometryGroup *group = GTHREE_GEOMETRY_GROUP (obj);

  g_array_free (group->face_indexes, TRUE);

  gthree_geometry_group_dispose (group);

  G_OBJECT_CLASS (gthree_geometry_group_parent_class)->finalize (obj);
}

static void
gthree_geometry_group_class_init (GthreeGeometryGroupClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_geometry_group_finalize;

}

static void
create_buffers (GthreeGeometryGroup *group)
{
  GthreeBuffer *buffer;

  buffer = GTHREE_BUFFER (group);
  if (buffer->vertex_buffer == 0)
    glGenBuffers (1, &buffer->vertex_buffer);
  if (buffer->face_buffer == 0)
    glGenBuffers (1, &buffer->face_buffer);
  if (buffer->line_buffer == 0)
    glGenBuffers (1, &buffer->line_buffer);
  if (buffer->normal_buffer == 0)
    glGenBuffers (1, &buffer->normal_buffer);
  if (buffer->tangent_buffer == 0)
    glGenBuffers (1, &buffer->tangent_buffer);
  if (buffer->color_buffer == 0)
    glGenBuffers (1, &buffer->color_buffer);
  if (buffer->uv_buffer == 0)
    glGenBuffers (1, &buffer->uv_buffer);
  if (buffer->uv2_buffer == 0)
    glGenBuffers (1, &buffer->uv2_buffer);
}

void
init_buffers (GthreeGeometryGroup *group,
              GthreeMaterial *group_material)
{
  GArray *face_indexes = group->face_indexes;
  guint nvertices = face_indexes->len * 3;
  guint ntris     = face_indexes->len * 1;
  guint nlines    = face_indexes->len * 3;
  gboolean uv_type = gthree_material_needs_uv (group_material);
  gboolean normal_type = gthree_material_needs_normals (group_material);
  GthreeColorType vertex_color_type = gthree_material_needs_colors (group_material);

  if (group->vertex_array == NULL)
    {
      group->vertex_array = g_new (float, nvertices * 3);
      /* TODO: Handle uint32 extension */
      group->face_array = g_new0 (guint16, ntris * 3);
      group->line_array = g_new0 (guint16, nlines * 2);

      group->vertices_need_update = TRUE;
      group->elements_need_update = TRUE;
    }

  if (group->normal_array == NULL &&
      normal_type)
    {
      group->normal_array = g_new (float, nvertices * 3);
      group->normals_need_update = TRUE;
    }

  /*
  if (geometry.hasTangents) {
    group->tangentArray = g_new (float,  nvertices * 4 );
    group->tangents_need_update = TRUE;
  }
  */

  if (group->color_array == NULL &&
      vertex_color_type != GTHREE_COLOR_NONE)
    {
      group->color_array = g_new (float, nvertices * 3);
      group->colors_need_update = TRUE;
    }


  if (uv_type)
    {
      if (group->uv_array == NULL &&
          gthree_geometry_get_n_uv (group->geometry) > 0)
        {
          group->uv_array = g_new (float, nvertices * 2);
          group->uvs_need_update = TRUE;
        }

      if (group->uv2_array == NULL &&
          gthree_geometry_get_n_uv2 (group->geometry) > 0)
        {
          group->uv2_array = g_new (float, nvertices * 2);
          group->uvs_need_update = TRUE;
        }
    }

  /*
  if ( object.geometry.skinWeights.length && object.geometry.skinIndices.length ) {
    group->skinIndexArray = g_new (float,  nvertices * 4 );
    group->skinWeightArray = g_new (float,  nvertices * 4 );
    group->morph_targets_need_update = TRUE;
  }
  */


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

void
gthree_geometry_group_realize (GthreeGeometryGroup *group,
                               GthreeMaterial *group_material)
{
  create_buffers (group);
  init_buffers (group, group_material);

}

void
gthree_geometry_group_update (GthreeGeometryGroup *group,
                              GthreeMaterial *material,
                              gboolean dispose)
{
  GthreeGeometry *geometry = group->geometry;
  gboolean uv_type = gthree_material_needs_uv (material);
  gboolean normal_type = gthree_material_needs_normals (material);
  GthreeColorType vertex_color_type = gthree_material_needs_colors (material);
  gboolean needs_smooth_normals = normal_type == GTHREE_SHADING_SMOOTH;

  gboolean dirtyVertices = group->vertices_need_update;
  gboolean dirtyElements = group->elements_need_update;
  gboolean dirtyUvs = group->uvs_need_update;
  gboolean dirtyNormals = group->normals_need_update;
  //gboolean dirtyTangents = group->tangents_need_update;
  gboolean dirtyColors = group->colors_need_update;
  //gboolean dirtyMorphTargets = group->morph_targets_need_update;

  guint vertexIndex = 0;
  guint offset = 0;
  guint offset_face = 0;
  guint offset_line = 0;
  guint offset_color = 0;
  guint offset_uv = 0;
  guint offset_uv2 = 0;
  guint offset_normal = 0;
  guint hint = GL_DYNAMIC_DRAW;
  int i;

  GArray *face_indexes = group->face_indexes;

  const graphene_vec3_t *vertices = gthree_geometry_get_vertices (geometry);

  if (dirtyVertices)
    {
      g_assert (group->vertex_array);

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

      group->vertices_need_update = FALSE;
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
      group->colors_need_update = FALSE;
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
      group->normals_need_update = FALSE;
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
      group->uvs_need_update = FALSE;
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
      group->uvs_need_update = FALSE;
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

      group->elements_need_update = FALSE;
    }

  /*
  group->morph_targets_need_update = FALSE;
  group->tangents_need_update = FALSE;
  */

  /*  if (dispose)
      gthree_geometry_group_dispose (group); */
}

void
gthree_geometry_group_add_face (GthreeGeometryGroup *group,
                                int face_index)
{
  g_array_append_val (group->face_indexes, face_index);
  group->n_vertices += 3;
}
