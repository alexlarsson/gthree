#include <math.h>
#include <epoxy/gl.h>

#include "gthreegeometry.h"

enum {
  AXIS_X,
  AXIS_Y,
  AXIS_Z,
};

static void
build_plane (GthreeGeometry *geometry,
             int u, int v, int udir, int vdir, float width, float height, float depth, int materialIndex,
             int widthSegments, int heightSegments, int depthSegments)
{
  int w;
  int ix, iy;
  float gridX = widthSegments;
  float gridY = heightSegments;
  float gridX1, gridY1, segment_width, segment_height;
  float width_half = width / 2;
  float height_half = height / 2;
  int offset = gthree_geometry_get_n_vertices (geometry);
  float normal[3];
  graphene_vec3_t normalv;

  if ((u == AXIS_X && v == AXIS_Y) || (u == AXIS_Y && v == AXIS_X))
    {
      w = AXIS_Z;
    }
  else if ((u == AXIS_X && v == AXIS_Z) || (u == AXIS_Z && v == AXIS_X))
    {
      w = AXIS_Y;
      gridY = depthSegments;
    }
  else /* ((u == AXIS_Z && v == AXIS_Y) || (u == AXIS_Y && v == AXIS_Z)) */
    {
      w = AXIS_X;
      gridX = depthSegments;
    }

  gridX1 = gridX + 1;
  gridY1 = gridY + 1;

  segment_width = width / gridX;
  segment_height = height / gridY;

  normal[u] = 0;
  normal[v] = 0;
  normal[w] = depth > 0 ? 1 : -1;
  graphene_vec3_init_from_float (&normalv, normal);

  for (iy = 0; iy < gridY1; iy++)
    {
      for (ix = 0; ix < gridX1; ix++)
        {
          float vector[3];
          graphene_vec3_t vectorv;

          vector[u] = ( ix * segment_width - width_half ) * udir;
          vector[v] = ( iy * segment_height - height_half ) * vdir;
          vector[w] = depth;

          graphene_vec3_init_from_float (&vectorv, vector);
          gthree_geometry_add_vertex (geometry, &vectorv);
        }
    }

  for (iy = 0; iy < gridY; iy++)
    {
      for (ix = 0; ix < gridX; ix++)
        {
          int face;
          graphene_vec2_t uva, uvb, uvc, uvd;
          int a = ix + gridX1 * iy;
          int b = ix + gridX1 * ( iy + 1 );
          int c = ( ix + 1 ) + gridX1 * ( iy + 1 );
          int d = ( ix + 1 ) + gridX1 * iy;

          graphene_vec2_init (&uva,
                              ix / (float)gridX, 1 - iy / (float)gridY);
          graphene_vec2_init (&uvb,
                              ix / (float)gridX, 1 - ( iy + 1 ) / (float)gridY);
          graphene_vec2_init (&uvc,
                              (ix + 1) / (float)gridX, 1 - (iy + 1) / (float)gridY);
          graphene_vec2_init (&uvd,
                              (ix + 1) / (float)gridX, 1 - iy / (float)gridY);

          face = gthree_geometry_add_face (geometry, a + offset, b + offset, d + offset);
          gthree_geometry_face_set_normal (geometry, face, &normalv);
          gthree_geometry_face_set_vertex_normals (geometry, face, &normalv, &normalv, &normalv);
          gthree_geometry_face_set_material_index (geometry, face, materialIndex);

          gthree_geometry_add_uv (geometry, &uva);
          gthree_geometry_add_uv (geometry, &uvb);
          gthree_geometry_add_uv (geometry, &uvd);

          face = gthree_geometry_add_face (geometry, b + offset, c + offset, d + offset);
          gthree_geometry_face_set_normal (geometry, face, &normalv);
          gthree_geometry_face_set_vertex_normals (geometry, face, &normalv, &normalv, &normalv);
          gthree_geometry_face_set_material_index (geometry, face, materialIndex);

          gthree_geometry_add_uv (geometry, &uvb);
          gthree_geometry_add_uv (geometry, &uvc);
          gthree_geometry_add_uv (geometry, &uvd);
        }
    }
}

GthreeGeometry *
gthree_geometry_new_box (float width, float height, float depth,
                         int widthSegments, int heightSegments, int depthSegments)
{
  GthreeGeometry *geometry;
  float width_half = width / 2;
  float height_half = height / 2;
  float depth_half = depth / 2;

  geometry = g_object_new (gthree_geometry_get_type (),
                           NULL);

  build_plane (geometry, AXIS_Z, AXIS_Y, -1, -1, depth, height, width_half, 0, widthSegments, heightSegments, depthSegments); // px
  build_plane (geometry, AXIS_Z, AXIS_Y,  1, -1, depth, height, - width_half, 1, widthSegments, heightSegments, depthSegments); // nx
  build_plane (geometry, AXIS_X, AXIS_Z,  1,  1, width, depth, height_half, 2, widthSegments, heightSegments, depthSegments ); // py
  build_plane (geometry, AXIS_X, AXIS_Z,  1, -1, width, depth, - height_half, 3, widthSegments, heightSegments, depthSegments ); // ny
  build_plane (geometry, AXIS_X, AXIS_Y,  1, -1, width, height, depth_half, 4, widthSegments, heightSegments, depthSegments ); // pz
  build_plane (geometry, AXIS_X, AXIS_Y, -1, -1, width, height, - depth_half, 5, widthSegments, heightSegments, depthSegments ); // nz

  // TODO
  //this.mergeVertices();

  return geometry;
}


GthreeGeometry *
gthree_geometry_new_sphere_full (float radius,
                                 int widthSegments, int heightSegments,
                                 float phiStart, float phiLength,
                                 float thetaStart, float thetaLength)
{
  GthreeGeometry *geometry;
  int x, y, vertex_count, vertices_w, vertices_h;
  int *vertices;
  graphene_vec2_t *uvs;
  const graphene_vec3_t *final_vertices;
  GthreeSphere bound;

  geometry = g_object_new (gthree_geometry_get_type (),
                           NULL);

  widthSegments = MAX(3, widthSegments);
  heightSegments = MAX(2, heightSegments);

  vertices_w = widthSegments + 1;
  vertices_h = heightSegments + 1;
  vertices = g_new (int, vertices_w * vertices_h);
  uvs = g_new (graphene_vec2_t, vertices_w * vertices_h);

  vertex_count = 0;
  for (y = 0; y <= heightSegments; y++)
    {
      for (x = 0; x <= widthSegments; x++)
        {
          float u = (float)x / widthSegments;
          float v = (float)y / heightSegments;
          float vx, vy, vz;
          graphene_vec3_t vertex;
          graphene_vec2_t uv;

          vx = - radius * cos (phiStart + u * phiLength) * sin (thetaStart + v * thetaLength);
          vy = radius * cos (thetaStart + v * thetaLength);
          vz = radius * sin (phiStart + u * phiLength) * sin (thetaStart + v * thetaLength);

          graphene_vec3_init (&vertex, vx, vy, vz);
          gthree_geometry_add_vertex (geometry, &vertex);

          vertices[x + y * vertices_w] = vertex_count++;

          graphene_vec2_init (&uv, u, 1 - v);
          uvs[x + y * vertices_w] = uv;
        }
    }

  final_vertices = gthree_geometry_get_vertices (geometry);
  for (y = 0; y < heightSegments; y++)
    {
      for (x = 0; x < widthSegments; x++)
        {
          int vi1 = vertices[(y    ) * vertices_w + (x + 1)];
          int vi2 = vertices[(y    ) * vertices_w + (x    )];
          int vi3 = vertices[(y + 1) * vertices_w + (x    )];
          int vi4 = vertices[(y + 1) * vertices_w + (x + 1)];
          graphene_vec2_t uv1 = uvs[(y    ) * vertices_w + (x + 1)];
          graphene_vec2_t uv2 = uvs[(y    ) * vertices_w + (x    )];
          graphene_vec2_t uv3 = uvs[(y + 1) * vertices_w + (x    )];
          graphene_vec2_t uv4 = uvs[(y + 1) * vertices_w + (x + 1)];
          const graphene_vec3_t *v1 = &final_vertices[vi1];
          const graphene_vec3_t *v2 = &final_vertices[vi2];
          const graphene_vec3_t *v3 = &final_vertices[vi3];
          const graphene_vec3_t *v4 = &final_vertices[vi4];
          graphene_vec3_t n1, n2, n3, n4;
          int face;

          graphene_vec3_normalize (v1, &n1);
          graphene_vec3_normalize (v2, &n2);
          graphene_vec3_normalize (v3, &n3);
          graphene_vec3_normalize (v4, &n4);

          if (fabs (graphene_vec3_get_y (v1)) == radius)
            {
              graphene_vec2_init (&uv1,
                                  (graphene_vec2_get_x (&uv1) + graphene_vec2_get_x (&uv2)) / 2,
                                  graphene_vec2_get_y (&uv1));

              face = gthree_geometry_add_face (geometry, vi1, vi3, vi4);
              gthree_geometry_face_set_vertex_normals (geometry, face,
						       &n1, &n3, &n4);

              gthree_geometry_add_uv (geometry, &uv1);
              gthree_geometry_add_uv (geometry, &uv3);
              gthree_geometry_add_uv (geometry, &uv4);
            }
          else if (fabs (graphene_vec3_get_y (v3)) == radius)
            {
              graphene_vec2_init (&uv3,
                                  (graphene_vec2_get_x (&uv3) + graphene_vec2_get_x (&uv4)) / 2,
                                  graphene_vec2_get_y (&uv3));

              face = gthree_geometry_add_face (geometry, vi1, vi2, vi3);
              gthree_geometry_face_set_vertex_normals (geometry, face,
						       &n1, &n2, &n3);

              gthree_geometry_add_uv (geometry, &uv1);
              gthree_geometry_add_uv (geometry, &uv2);
              gthree_geometry_add_uv (geometry, &uv3);
            }
          else
            {
              face = gthree_geometry_add_face (geometry, vi1, vi2, vi4);
              gthree_geometry_face_set_vertex_normals (geometry, face,
						       &n1, &n2, &n4);

              gthree_geometry_add_uv (geometry, &uv1);
              gthree_geometry_add_uv (geometry, &uv2);
              gthree_geometry_add_uv (geometry, &uv4);

              face = gthree_geometry_add_face (geometry, vi2, vi3, vi4);
              gthree_geometry_face_set_vertex_normals (geometry, face,
						       &n2, &n3, &n4);

              gthree_geometry_add_uv (geometry, &uv2);
              gthree_geometry_add_uv (geometry, &uv3);
              gthree_geometry_add_uv (geometry, &uv4);
            }
        }
    }

  g_free (vertices);
  g_free (uvs);

  gthree_geometry_compute_face_normals (geometry);

  graphene_vec3_init (&bound.center, 0, 0, 0);
  bound.radius = radius;
  gthree_geometry_set_bounding_sphere  (geometry, &bound);

  return geometry;
}

GthreeGeometry *
gthree_geometry_new_sphere (float radius,
                            int   widthSegments,
                            int   heightSegments)
{
  return gthree_geometry_new_sphere_full (radius, widthSegments, heightSegments,
                                          0, 2 * G_PI, 0, G_PI);
}
