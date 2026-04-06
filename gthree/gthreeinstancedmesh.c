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
