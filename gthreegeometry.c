#include <math.h>
#include <epoxy/gl.h>

#include "gthreegeometry.h"
#include "gthreegeometry.h"

typedef struct {
  GArray *vertices; /* graphene_vec3_t */

  GArray *colors; /* GdkRGBA, per vertex */

  GPtrArray *faces; /* GthreeFace* */

  graphene_point3d_t bounding_box_min;
  graphene_point3d_t bounding_box_max;

} GthreeGeometryPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeGeometry, gthree_geometry, G_TYPE_OBJECT);

GthreeGeometry *
gthree_geometry_new ()
{
  GthreeGeometry *geometry;

  geometry = g_object_new (gthree_geometry_get_type (),
                         NULL);

  return geometry;
}

void
gthree_geometry_add_vertex (GthreeGeometry *geometry,
                            graphene_vec3_t *v)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_array_append_val (priv->vertices,*v);
}

guint
gthree_geometry_get_n_vertices (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->vertices->len;
}

const graphene_vec3_t *
gthree_geometry_get_vertices (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return (graphene_vec3_t *)priv->vertices->data;
}

/* Takes ownership */
void
gthree_geometry_add_face (GthreeGeometry *geometry,
                          GthreeFace *face)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_ptr_array_add (priv->faces, face);
}

GthreeFace *
gthree_geometry_get_face (GthreeGeometry *geometry,
                          int i)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return g_ptr_array_index (priv->faces, i);
}

guint
gthree_geometry_get_n_faces (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->faces->len;
}

static void
gthree_geometry_init (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  priv->vertices = g_array_new (FALSE, FALSE, sizeof (graphene_vec3_t));
  priv->colors = g_array_new (FALSE, FALSE, sizeof (GdkRGBA));
  priv->faces = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
}

static void
gthree_geometry_finalize (GObject *obj)
{
  GthreeGeometry *geometry = GTHREE_GEOMETRY (obj);
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_array_free (priv->vertices, TRUE);
  g_array_free (priv->colors, TRUE);
  g_ptr_array_free (priv->faces, TRUE);

  G_OBJECT_CLASS (gthree_geometry_parent_class)->finalize (obj);
}

static void
gthree_geometry_class_init (GthreeGeometryClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_geometry_finalize;
}
