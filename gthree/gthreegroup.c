#include "gthreegroup.h"


G_DEFINE_TYPE (GthreeGroup, gthree_group, GTHREE_TYPE_OBJECT)


static void
gthree_group_class_init (GthreeGroupClass *klass)
{
}

static void
gthree_group_init (GthreeGroup *group)
{
}

GthreeGroup *
gthree_group_new ()
{
  return g_object_new (gthree_group_get_type (),
                       NULL);
}
