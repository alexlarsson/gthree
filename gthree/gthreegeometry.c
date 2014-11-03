#include <math.h>
#include <epoxy/gl.h>

#include "gthreegeometry.h"
#include "gthreeprivate.h"

typedef struct
{
  GObject parent;

  int a, b, c;
  graphene_vec3_t normal;
  GdkRGBA color;
  graphene_vec3_t *vertex_normals;
  GdkRGBA *vertex_colors;
  int material_index;
} GthreeFace;

typedef struct {
  GArray *vertices; /* graphene_vec3_t */

  GArray *colors; /* GdkRGBA, per vertex */

  GArray *faces; /* GthreeFace */

  GArray *uv; /* graphene_vec2_t */
  GArray *uv2; /* graphene_vec2_t */

  graphene_box_t bounding_box;
  graphene_sphere_t bounding_sphere;

  guint bounding_box_set;
  guint bounding_sphere_set;

} GthreeGeometryPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeGeometry, gthree_geometry, G_TYPE_OBJECT);

static void
gthree_face_destroy (GthreeFace *face)
{
  g_clear_pointer (&face->vertex_colors, g_free);
  g_clear_pointer (&face->vertex_normals, g_free);
}

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

guint
gthree_geometry_get_n_colors (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->colors->len;
}

guint
gthree_geometry_get_n_faces (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->faces->len;
}

const graphene_vec2_t *
gthree_geometry_get_uvs (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return (graphene_vec2_t *)priv->uv->data;
}

guint
gthree_geometry_get_n_uv (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->uv->len;
}

void
gthree_geometry_add_uv (GthreeGeometry *geometry,
                        graphene_vec2_t *v)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_array_append_val (priv->uv, *v);
}

const graphene_vec2_t *
gthree_geometry_get_uv2s (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return (graphene_vec2_t *)priv->uv2->data;
}

guint
gthree_geometry_get_n_uv2 (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  return priv->uv2->len;
}

void
gthree_geometry_add_uv2 (GthreeGeometry *geometry,
                         graphene_vec2_t *v)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  g_array_append_val (priv->uv2, *v);
}

void
gthree_geometry_set_uv_n (GthreeGeometry  *geometry,
                          int              layer,
                          int              index,
                          graphene_vec2_t *v)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (layer == 0)
    {
      if (priv->uv->len <= index)
        g_array_set_size (priv->uv, index + 1);
      g_array_index (priv->uv, graphene_vec2_t, index) = *v;
    }
  else if (layer == 1)
    {
      if (priv->uv2->len <= index)
        g_array_set_size (priv->uv2, index + 1);
      g_array_index (priv->uv2, graphene_vec2_t, index) = *v;
    }
  else
    g_warning ("only 2 uv layers supported");
}


void
gthree_geometry_set_bounding_sphere (GthreeGeometry *geometry,
                                     const graphene_sphere_t *sphere)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  priv->bounding_sphere_set = TRUE;
  priv->bounding_sphere = *sphere;
}


const graphene_sphere_t *
gthree_geometry_get_bounding_sphere (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  if (!priv->bounding_sphere_set)
    {
      graphene_sphere_init_from_vectors (&priv->bounding_sphere,
                                         priv->vertices->len,
                                         (const graphene_vec3_t *)(priv->vertices->data),
                                         NULL);
      priv->bounding_sphere_set = TRUE;
    }

  return &priv->bounding_sphere;
}

void
gthree_geometry_compute_face_normals (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;
  graphene_vec3_t cb, ab;
  const graphene_vec3_t *vertices;
  const graphene_vec3_t *va, *vb, *vc;
  int i, n_faces;

  n_faces = gthree_geometry_get_n_faces (geometry);
  vertices = gthree_geometry_get_vertices (geometry);
  for (i = 0; i < n_faces; i++)
    {
      face = &g_array_index (priv->faces, GthreeFace, i);

      va = &vertices[face->a];
      vb = &vertices[face->b];
      vc = &vertices[face->c];

      graphene_vec3_subtract (vc, vb, &cb);
      graphene_vec3_subtract (va, vb, &ab);
      graphene_vec3_cross (&cb, &ab, &cb);
      graphene_vec3_normalize (&cb, &cb);

      face->normal = cb;
    }
}

int
gthree_geometry_add_face (GthreeGeometry        *geometry,
			  int                    a,
			  int                    b,
			  int                    c)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  int i = priv->faces->len;

  g_array_set_size (priv->faces, i + 1);
  
  face = &g_array_index (priv->faces, GthreeFace, i);
  face->a = a;
  face->b = b;
  face->c = c;

  return i;
}

int
gthree_geometry_face_get_a (GthreeGeometry *geometry,
			    int index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  return face->a;
}

int
gthree_geometry_face_get_b (GthreeGeometry *geometry,
			    int index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  return face->b;
}

int
gthree_geometry_face_get_c (GthreeGeometry *geometry,
			    int index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  
  return face->c;
}

void
gthree_geometry_face_set_normal (GthreeGeometry        *geometry,
				 int                    index,
				 const graphene_vec3_t *normal)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  face->normal = *normal;
}

const graphene_vec3_t *
gthree_geometry_face_get_normal (GthreeGeometry        *geometry,
				 int                    index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  return &face->normal;
}

void
gthree_geometry_face_set_vertex_normals (GthreeGeometry        *geometry,
					 int                    index,
					 const graphene_vec3_t *normal_a,
					 const graphene_vec3_t *normal_b,
					 const graphene_vec3_t *normal_c)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);

    if (face->vertex_normals == NULL)
    face->vertex_normals = g_new (graphene_vec3_t, 3);

  face->vertex_normals[0] = *normal_a;
  face->vertex_normals[1] = *normal_b;
  face->vertex_normals[2] = *normal_c;
}

gboolean
gthree_geometry_face_get_vertex_normals  (GthreeGeometry         *geometry,
					 int                     index,
					 const graphene_vec3_t **normal_a,
					 const graphene_vec3_t **normal_b,
					 const graphene_vec3_t **normal_c)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  if (face->vertex_normals == NULL)
    {
      *normal_a = NULL;
      *normal_b = NULL;
      *normal_c = NULL;
      return FALSE;
    }
  else
    {
      *normal_a = &face->vertex_normals[0];
      *normal_b = &face->vertex_normals[1];
      *normal_c = &face->vertex_normals[2];
      return TRUE;
    }
}

void
gthree_geometry_face_set_color (GthreeGeometry        *geometry,
				int                    index,
				const GdkRGBA         *color)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  face->color = *color;
}

const GdkRGBA *
gthree_geometry_face_get_color (GthreeGeometry         *geometry,
				int                     index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  return &face->color;
}

void
gthree_geometry_face_set_vertex_colors (GthreeGeometry        *geometry,
					int                    index,
					const GdkRGBA         *a,
					const GdkRGBA         *b,
					const GdkRGBA         *c)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  if (face->vertex_colors == NULL)
    face->vertex_colors = g_new (GdkRGBA, 3);

  face->vertex_colors[0] = *a;
  face->vertex_colors[1] = *b;
  face->vertex_colors[2] = *c;
}

gboolean
gthree_geometry_face_get_vertex_colors  (GthreeGeometry         *geometry,
					 int                     index,
					 const GdkRGBA         **a,
					 const GdkRGBA         **b,
					 const GdkRGBA         **c)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  if (face->vertex_colors == NULL)
    {
      *a = NULL;
      *b = NULL;
      *c = NULL;
      return FALSE;
    }
  else
    {
      *a = &face->vertex_colors[0];
      *b = &face->vertex_colors[1];
      *c = &face->vertex_colors[2];
      return TRUE;
    }
}

void
gthree_geometry_face_set_material_index (GthreeGeometry        *geometry,
					 int                    index,
					 int                    material_index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  face->material_index = material_index;
}

int
gthree_geometry_face_get_material_index (GthreeGeometry        *geometry,
					 int                    index)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  GthreeFace *face;

  face = &g_array_index (priv->faces, GthreeFace, index);
  return face->material_index;
}

static void
gthree_geometry_init (GthreeGeometry *geometry)
{
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);

  priv->vertices = g_array_new (FALSE, FALSE, sizeof (graphene_vec3_t));
  priv->colors = g_array_new (FALSE, FALSE, sizeof (GdkRGBA));
  priv->faces = g_array_new (FALSE, TRUE, sizeof (GthreeFace));
  priv->uv = g_array_new (FALSE, TRUE, sizeof (graphene_vec2_t));
  priv->uv2 = g_array_new (FALSE, TRUE, sizeof (graphene_vec2_t));
}

static void
gthree_geometry_finalize (GObject *obj)
{
  GthreeGeometry *geometry = GTHREE_GEOMETRY (obj);
  GthreeGeometryPrivate *priv = gthree_geometry_get_instance_private (geometry);
  int i;

  g_array_free (priv->vertices, TRUE);
  g_array_free (priv->colors, TRUE);
  for (i = 0; i < priv->faces->len; i++)
    gthree_face_destroy (&g_array_index (priv->faces, GthreeFace, i));
  g_array_free (priv->faces, TRUE);
  g_array_free (priv->uv, TRUE);
  g_array_free (priv->uv2, TRUE);

  G_OBJECT_CLASS (gthree_geometry_parent_class)->finalize (obj);
}

static void
gthree_geometry_class_init (GthreeGeometryClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_geometry_finalize;
}
