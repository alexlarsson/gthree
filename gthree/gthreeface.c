#include <math.h>
#include <epoxy/gl.h>

#include "gthreeface.h"
#include "gthreeprivate.h"


G_DEFINE_TYPE (GthreeFace, gthree_face, G_TYPE_OBJECT);

GthreeFace *
gthree_face_new (int a, int b, int c)
{
  GthreeFace *face;

  face = g_object_new (gthree_face_get_type (),
                         NULL);

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

int
gthree_face_get_material_index (GthreeFace *face)
{
  return face->material_index;
}

static void
gthree_face_init (GthreeFace *face)
{
}

static void
gthree_face_finalize (GObject *obj)
{
  //GthreeFace *face = GTHREE_FACE (obj);

  G_OBJECT_CLASS (gthree_face_parent_class)->finalize (obj);
}

static void
gthree_face_class_init (GthreeFaceClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_face_finalize;
}
