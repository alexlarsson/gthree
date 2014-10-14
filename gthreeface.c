#include <math.h>
#include <epoxy/gl.h>

#include "gthreeface.h"
#include "gthreeprivate.h"

struct _GthreeFace {
  int a, b, c;
  graphene_vec3_t normal;
  GdkRGBA color;
  graphene_vec3_t *vertex_normals;
  GdkRGBA *vertex_colors;
  int material_index;
};

GthreeFace *
gthree_face_new (int a, int b, int c)
{
  GthreeFace *face;

  face = g_slice_new0 (GthreeFace);
  face->a = a;
  face->b = b;
  face->c = c;

  return face;
}

void
gthree_face_set_normal (GthreeFace *face,
                        graphene_vec3_t *normal)
{
  face->normal = *normal;
}

void
gthree_face_set_color (GthreeFace *face,
                       GdkRGBA *color)
{
  face->color = *color;
}

void
gthree_face_set_material_index (GthreeFace *face,
                                int material_index)
{
  face->material_index = material_index;
}

void
gthree_face_destroy (GthreeFace *face)
{
  g_slice_free (GthreeFace, face);
}
