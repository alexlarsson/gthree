#include <epoxy/gl.h>

#include "gthreeinstancedmesh.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"
#include "gthreeattribute.h"

typedef struct {
  int count;
  int allocated_count;
  GthreeAttribute *instance_matrix;
  GthreeAttribute *instance_color;

  float *morph_texture_data;
  guint morph_texture;
  int morph_texture_width;
  int morph_texture_height;
  gboolean morph_texture_needs_update;
} GthreeInstancedMeshPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeInstancedMesh, gthree_instanced_mesh, GTHREE_TYPE_MESH)

GthreeInstancedMesh *
gthree_instanced_mesh_new (GthreeGeometry *geometry,
                           GthreeMaterial *material,
                           int             count)
{
  GthreeInstancedMesh *mesh;
  GthreeInstancedMeshPrivate *priv;
  g_autoptr(GPtrArray) materials = g_ptr_array_new_with_free_func (g_object_unref);
  float identity[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  if (material)
    g_ptr_array_add (materials, g_object_ref (material));

  mesh = g_object_new (gthree_instanced_mesh_get_type (),
                       "geometry", geometry,
                       "materials", materials,
                       NULL);

  gthree_mesh_update_morph_targets (GTHREE_MESH (mesh));

  priv = gthree_instanced_mesh_get_instance_private (mesh);
  priv->count = count;
  priv->allocated_count = count;

  priv->instance_matrix = gthree_attribute_new ("instanceMatrix",
                                                GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                                count, 16, FALSE);
  gthree_attribute_set_dynamic (priv->instance_matrix, TRUE);

  for (int i = 0; i < count; i++)
    {
      GthreeAttributeArray *array = gthree_attribute_get_array (priv->instance_matrix);
      gthree_attribute_array_set_elements_from_float (array, i, 0, identity, 16);
    }

  return mesh;
}

static void
gthree_instanced_mesh_init (GthreeInstancedMesh *mesh)
{
}

static void
gthree_instanced_mesh_finalize (GObject *obj)
{
  GthreeInstancedMesh *mesh = GTHREE_INSTANCED_MESH (obj);
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  g_clear_object (&priv->instance_matrix);
  g_clear_object (&priv->instance_color);
  g_clear_pointer (&priv->morph_texture_data, g_free);
  if (priv->morph_texture != 0)
    {
      glDeleteTextures (1, &priv->morph_texture);
      priv->morph_texture = 0;
    }

  G_OBJECT_CLASS (gthree_instanced_mesh_parent_class)->finalize (obj);
}

static void
gthree_instanced_mesh_update (GthreeObject   *object,
                              GthreeRenderer *renderer)
{
  GthreeInstancedMesh *mesh = GTHREE_INSTANCED_MESH (object);
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  GTHREE_OBJECT_CLASS (gthree_instanced_mesh_parent_class)->update (object, renderer);

  gthree_attribute_update (priv->instance_matrix, renderer, GL_ARRAY_BUFFER);
  if (priv->instance_color)
    gthree_attribute_update (priv->instance_color, renderer, GL_ARRAY_BUFFER);
}

static void
gthree_instanced_mesh_class_init (GthreeInstancedMeshClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_instanced_mesh_finalize;
  object_class->update = gthree_instanced_mesh_update;
}

int
gthree_instanced_mesh_get_count (GthreeInstancedMesh *mesh)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);
  return priv->count;
}

void
gthree_instanced_mesh_set_count (GthreeInstancedMesh *mesh,
                                 int                  count)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  g_return_if_fail (count >= 0 && count <= priv->allocated_count);
  priv->count = count;
}

void
gthree_instanced_mesh_set_matrix_at (GthreeInstancedMesh    *mesh,
                                     int                     index,
                                     const graphene_matrix_t *matrix)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);
  GthreeAttributeArray *array;
  float floats[16];

  g_return_if_fail (index >= 0 && index < priv->allocated_count);

  graphene_matrix_to_float (matrix, floats);
  array = gthree_attribute_get_array (priv->instance_matrix);
  gthree_attribute_array_set_elements_from_float (array, index, 0, floats, 16);
  gthree_attribute_set_needs_update (priv->instance_matrix);
}

void
gthree_instanced_mesh_get_matrix_at (GthreeInstancedMesh *mesh,
                                     int                  index,
                                     graphene_matrix_t   *matrix)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  g_return_if_fail (index >= 0 && index < priv->allocated_count);

  gthree_attribute_get_matrix (priv->instance_matrix, index, matrix);
}

void
gthree_instanced_mesh_set_color_at (GthreeInstancedMesh   *mesh,
                                    int                    index,
                                    const graphene_vec3_t *color)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  g_return_if_fail (index >= 0 && index < priv->allocated_count);

  if (priv->instance_color == NULL)
    {
      priv->instance_color = gthree_attribute_new ("instanceColor",
                                                   GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                                   priv->allocated_count, 3, FALSE);
      gthree_attribute_set_dynamic (priv->instance_color, TRUE);

      for (int i = 0; i < priv->allocated_count; i++)
        {
          graphene_vec3_t white;
          graphene_vec3_init (&white, 1, 1, 1);
          gthree_attribute_set_vec3 (priv->instance_color, i, &white);
        }
    }

  gthree_attribute_set_vec3 (priv->instance_color, index, color);
  gthree_attribute_set_needs_update (priv->instance_color);
}

void
gthree_instanced_mesh_get_color_at (GthreeInstancedMesh *mesh,
                                    int                  index,
                                    graphene_vec3_t     *color)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  g_return_if_fail (priv->instance_color != NULL);
  g_return_if_fail (index >= 0 && index < priv->allocated_count);

  gthree_attribute_get_vec3 (priv->instance_color, index, color);
}

GthreeAttribute *
gthree_instanced_mesh_get_instance_matrix (GthreeInstancedMesh *mesh)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);
  return priv->instance_matrix;
}

GthreeAttribute *
gthree_instanced_mesh_get_instance_color (GthreeInstancedMesh *mesh)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);
  return priv->instance_color;
}

void
gthree_instanced_mesh_set_morph_at (GthreeInstancedMesh *mesh,
                                    int                  index,
                                    GthreeMesh          *source)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);
  GArray *influences = gthree_mesh_get_morph_targets (source);

  g_return_if_fail (index >= 0 && index < priv->allocated_count);
  g_return_if_fail (influences != NULL && influences->len > 0);

  int len = influences->len + 1;

  if (priv->morph_texture_data == NULL)
    {
      priv->morph_texture_width = len;
      priv->morph_texture_height = priv->allocated_count;
      priv->morph_texture_data = g_new0 (float, len * priv->allocated_count);
    }

  float morph_influences_sum = 0;
  for (int i = 0; i < (int)influences->len; i++)
    morph_influences_sum += g_array_index (influences, float, i);

  float base_influence = 1.0f - morph_influences_sum;
  if (base_influence < 0.0f)
    base_influence = 0.0f;

  int data_index = len * index;
  priv->morph_texture_data[data_index] = base_influence;
  for (int i = 0; i < (int)influences->len; i++)
    priv->morph_texture_data[data_index + 1 + i] = g_array_index (influences, float, i);

  priv->morph_texture_needs_update = TRUE;
}

guint
gthree_instanced_mesh_get_morph_texture (GthreeInstancedMesh *mesh)
{
  GthreeInstancedMeshPrivate *priv = gthree_instanced_mesh_get_instance_private (mesh);

  if (priv->morph_texture_data == NULL)
    return 0;

  if (priv->morph_texture == 0)
    {
      glGenTextures (1, &priv->morph_texture);
      glBindTexture (GL_TEXTURE_2D, priv->morph_texture);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      priv->morph_texture_needs_update = TRUE;
    }

  if (priv->morph_texture_needs_update)
    {
      glBindTexture (GL_TEXTURE_2D, priv->morph_texture);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_R32F,
                    priv->morph_texture_width, priv->morph_texture_height,
                    0, GL_RED, GL_FLOAT, priv->morph_texture_data);
      glBindTexture (GL_TEXTURE_2D, 0);
      priv->morph_texture_needs_update = FALSE;
    }

  return priv->morph_texture;
}
