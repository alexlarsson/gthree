#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"
#include "gthreemultimaterial.h"
#include "gthreebasicmaterial.h"
#include "gthreegeometrygroupprivate.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

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
gthree_mesh_update (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry, priv->material);

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_mesh_realize (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  gthree_geometry_realize (priv->geometry, priv->material);
  gthree_geometry_add_buffers_to_object (priv->geometry, priv->material, object);
}

static void
gthree_mesh_unrealize (GthreeObject *object)
{
  /* TODO: unrealize the geometry? */
}

static gboolean
gthree_mesh_in_frustum (GthreeObject *object,
                        const graphene_frustum_t *frustum)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  graphene_sphere_t sphere;

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static gboolean
gthree_mesh_real_has_attribute_data (GthreeObject                *object,
                                     const char                  *attribute)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  if (strcmp (attribute, "color") == 0)
    return gthree_geometry_get_n_colors (priv->geometry) > 0 || gthree_geometry_get_n_faces (priv->geometry);
  else if (strcmp (attribute, "uv") == 0)
    return gthree_geometry_get_n_uv (priv->geometry) > 0;
  else if (strcmp (attribute, "uv2") == 0)
    return gthree_geometry_get_n_uv2 (priv->geometry) > 0;

  return FALSE;
}


static void
gthree_mesh_class_init (GthreeMeshClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_mesh_finalize;

  GTHREE_OBJECT_CLASS (klass)->in_frustum = gthree_mesh_in_frustum;
  GTHREE_OBJECT_CLASS (klass)->has_attribute_data = gthree_mesh_real_has_attribute_data;
  GTHREE_OBJECT_CLASS (klass)->update = gthree_mesh_update;
  GTHREE_OBJECT_CLASS (klass)->realize = gthree_mesh_realize;
  GTHREE_OBJECT_CLASS (klass)->unrealize = gthree_mesh_unrealize;
}
