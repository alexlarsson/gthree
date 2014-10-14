#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;

} GthreeMeshPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMesh, gthree_mesh, GTHREE_TYPE_OBJECT);

GthreeMesh *
gthree_mesh_new (GthreeGeometry *geometry,
                 GthreeMaterial *material)
{
  GthreeMesh *mesh;
  GthreeMeshPrivate *priv;

  // TODO: properties
  mesh = g_object_new (gthree_mesh_get_type (),
                         NULL);

  priv = gthree_mesh_get_instance_private (mesh);

  priv->geometry = g_object_ref (geometry);
  priv->material = g_object_ref (material);

  return mesh;
}

static void
gthree_mesh_init (GthreeMesh *mesh)
{
  //GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

}

static void
gthree_mesh_finalize (GObject *obj)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_mesh_parent_class)->finalize (obj);
}

static void
gthree_mesh_class_init (GthreeMeshClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_finalize;
}
