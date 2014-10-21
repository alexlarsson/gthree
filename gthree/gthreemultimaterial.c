#include <math.h>
#include <epoxy/gl.h>

#include "gthreemultimaterial.h"

typedef struct {
  GHashTable *hash;
} GthreeMultiMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMultiMaterial, gthree_multi_material, GTHREE_TYPE_MATERIAL);

GthreeMultiMaterial *
gthree_multi_material_new ()
{
  GthreeMultiMaterial *material;

  material = g_object_new (gthree_multi_material_get_type (),
                        NULL);

  return material;
}

static void
gthree_multi_material_init (GthreeMultiMaterial *multi)
{
  GthreeMultiMaterialPrivate *priv = gthree_multi_material_get_instance_private (multi);

  priv->hash = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                      NULL, g_object_unref);
}

static void
gthree_multi_material_finalize (GObject *obj)
{
  GthreeMultiMaterial *multi = GTHREE_MULTI_MATERIAL (obj);
  GthreeMultiMaterialPrivate *priv = gthree_multi_material_get_instance_private (multi);

  g_hash_table_destroy (priv->hash);

  G_OBJECT_CLASS (gthree_multi_material_parent_class)->finalize (obj);
}

void
gthree_multi_material_set_index (GthreeMultiMaterial *multi,
                                 int index,
                                 GthreeMaterial *material)
{
  GthreeMultiMaterialPrivate *priv = gthree_multi_material_get_instance_private (multi);

  g_hash_table_replace (priv->hash, GINT_TO_POINTER (index), g_object_ref (material));
}

static GthreeMaterial *
gthree_multi_material_resolve (GthreeMaterial *material,
                               int index)
{
  GthreeMultiMaterial *multi = GTHREE_MULTI_MATERIAL (material);
  GthreeMultiMaterialPrivate *priv = gthree_multi_material_get_instance_private (multi);

  return g_hash_table_lookup (priv->hash, GINT_TO_POINTER (index));
}

static void
gthree_multi_material_class_init (GthreeMultiMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_multi_material_finalize;

  GTHREE_MATERIAL_CLASS(klass)->resolve = gthree_multi_material_resolve;
}

