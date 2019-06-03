#include "gthreebone.h"


G_DEFINE_TYPE (GthreeBone, gthree_bone, GTHREE_TYPE_OBJECT)


static void
gthree_bone_class_init (GthreeBoneClass *klass)
{
}

static void
gthree_bone_init (GthreeBone *bone)
{
}

GthreeBone *
gthree_bone_new ()
{
  return g_object_new (gthree_bone_get_type (),
                       NULL);
}
