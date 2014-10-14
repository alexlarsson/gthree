#include <math.h>
#include <epoxy/gl.h>

#include "gthreematerial.h"

typedef struct {
  int dummy;
} GthreeMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMaterial, gthree_material, G_TYPE_OBJECT);

GthreeMaterial *
gthree_material_new ()
{
  GthreeMaterial *material;

  material = g_object_new (gthree_material_get_type (),
                         NULL);


  return material;
}

static void
gthree_material_init (GthreeMaterial *material)
{
  //GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

}

static void
gthree_material_finalize (GObject *obj)
{
  //GthreeMaterial *material = GTHREE_MATERIAL (obj);
  //GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  G_OBJECT_CLASS (gthree_material_parent_class)->finalize (obj);
}

static void
gthree_material_class_init (GthreeMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_material_finalize;
}
