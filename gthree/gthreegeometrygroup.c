#include <math.h>
#include <epoxy/gl.h>

#include "gthreegeometrygroupprivate.h"

G_DEFINE_TYPE (GthreeGeometryGroup, gthree_geometry_group, GTHREE_TYPE_BUFFER);

GthreeGeometryGroup *
gthree_geometry_group_new (GthreeObject *object,
                           GthreeMaterial *material,
                           guint32 material_index)
{
  GthreeGeometryGroup *group;

  group = g_object_new (gthree_geometry_group_get_type (),
                        NULL);

  group->parent.object = object; /* Weak ref */
  group->parent.material = g_object_ref (material);
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

void
gthree_geometry_group_add_face (GthreeGeometryGroup *group,
                                int face_index)
{
  g_array_append_val (group->face_indexes, face_index);
  group->n_vertices += 3;
}
