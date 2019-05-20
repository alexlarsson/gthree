#include <math.h>
#include <epoxy/gl.h>

#include "gthreeprimitives.h"

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
  normal[w] = depth >= 0 ? 1 : -1;
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
gthree_geometry_new_plane (float width, float height,
                           int widthSegments, int heightSegments)
{
  GthreeGeometry *geometry;

  geometry = g_object_new (gthree_geometry_get_type (),
                           NULL);

  build_plane (geometry, AXIS_X, AXIS_Y,  1, -1, width, height, 0, 4, widthSegments, heightSegments, 1); // pz

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
  graphene_sphere_t bound;
  graphene_point3d_t center;

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

  graphene_sphere_init (&bound, graphene_point3d_init (&center, 0, 0, 0), radius);
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

GthreeGeometry *
gthree_geometry_new_cylinder_full (float    radiusTop,
                                   float    radiusBottom,
                                   float    height,
                                   int      radialSegments,
                                   int      heightSegments,
                                   gboolean openEnded,
                                   float    thetaStart,
                                   float    thetaLength)
{
  GthreeGeometry *geometry;
  int x, y;
  int center;
  graphene_vec3_t vertex;
  graphene_vec3_t n;
  graphene_vec3_t *normals;
  float tanTheta;

  geometry = g_object_new (gthree_geometry_get_type (), NULL);

  radialSegments = MAX(radialSegments, 3);
  heightSegments = MAX(heightSegments, 1);

  normals = g_newa (graphene_vec3_t, radialSegments + 1);
  tanTheta = (radiusBottom - radiusTop) / height;

  for (y = 0; y <= heightSegments; y++)
    {
      float v = y * 1.0 / heightSegments;
      float radius = v * (radiusBottom - radiusTop) + radiusTop;
      float vy = (0.5 - v) * height;

      for (x = 0; x <= radialSegments; x++)
        {
          float u = x * 1.0 / radialSegments;
          float angle = u * thetaLength + thetaStart;
          float vx, vz;
          float nx, ny, nz;

          vx = radius * sin (angle);
          vz = radius * cos (angle);

          gthree_geometry_add_vertex (geometry, graphene_vec3_init (&vertex, vx, vy, vz));
          if (y == 0)
            {
              nx = vx;
              nz = vz;
              ny = sqrt (nx * nx + nz * nz) * tanTheta;
              graphene_vec3_init (&normals[x], nx, ny, nz);
              graphene_vec3_normalize (&normals[x], &normals[x]);
            }
        }
    }

  for (y = 0; y < heightSegments; y++)
    {
      float v1 = y * 1.0 / heightSegments;
      float v2 = (y + 1) * 1.0 / heightSegments;

      for (x = 0; x < radialSegments; x++)
        {
          float u1 = x * 1.0 / radialSegments;
          float u2 = (x + 1) * 1.0 / radialSegments;

          int i1 = x + y * (radialSegments + 1);
          int i2 = x + (y + 1) * (radialSegments + 1);
          int i3 = x + 1 + (y + 1) * (radialSegments + 1);
          int i4 = x + 1 + y * (radialSegments + 1);
          int face;
          graphene_vec3_t *n1, *n2, *n3, *n4;
          graphene_vec2_t uv1, uv2, uv3, uv4;

          n1 = &normals[x];
          n2 = &normals[x];
          n3 = &normals[x + 1];
          n4 = &normals[x + 1];

          graphene_vec2_init (&uv1, u1, v1);
          graphene_vec2_init (&uv2, u1, v2);
          graphene_vec2_init (&uv3, u2, v2);
          graphene_vec2_init (&uv4, u2, v1);

          face = gthree_geometry_add_face (geometry, i1, i2, i4);
          gthree_geometry_face_set_vertex_normals (geometry, face, n1, n2, n4);

          gthree_geometry_add_uv (geometry, &uv1);
          gthree_geometry_add_uv (geometry, &uv2);
          gthree_geometry_add_uv (geometry, &uv4);

          face = gthree_geometry_add_face (geometry, i2, i3, i4);
          gthree_geometry_face_set_vertex_normals (geometry, face, n2, n3, n4);

          gthree_geometry_add_uv (geometry, &uv2);
          gthree_geometry_add_uv (geometry, &uv3);
          gthree_geometry_add_uv (geometry, &uv4);
        }
    }

  if (!openEnded && radiusTop > 0)
    {
      graphene_vec2_t uv1, uv2, uv3;

      center = gthree_geometry_get_n_vertices (geometry);

      gthree_geometry_add_vertex (geometry, graphene_vec3_init (&vertex, 0, 0.5 * height, 0));

      graphene_vec2_init (&uv1, 1, 1);

      graphene_vec3_normalize (&vertex, &n);

      for (x = 0; x < radialSegments; x++)
        {
          int i1 = center;
          int i2 = x;
          int i3 = x + 1;
          int face;

          graphene_vec2_init (&uv2, x * 1.0 / radialSegments, 0);
          graphene_vec2_init (&uv3, (x + 1) * 1.0 / radialSegments, 0);

          face = gthree_geometry_add_face (geometry, i1, i2, i3);
          gthree_geometry_face_set_vertex_normals (geometry, face, &n, &n, &n);

          gthree_geometry_add_uv (geometry, &uv1);
          gthree_geometry_add_uv (geometry, &uv2);
          gthree_geometry_add_uv (geometry, &uv3);
        }
    }

  if (!openEnded && radiusBottom > 0)
    {
      graphene_vec2_t uv1, uv2, uv3;

      center = gthree_geometry_get_n_vertices (geometry);

      gthree_geometry_add_vertex (geometry, graphene_vec3_init (&vertex, 0, -0.5 * height, 0));

      graphene_vec2_init (&uv1, 0, 0);

      graphene_vec3_normalize (&vertex, &n);

      for (x = 0; x < radialSegments; x++)
        {
          int i1 = center;
          int i2 = x + heightSegments * (radialSegments + 1);
          int i3 = x + 1 + heightSegments * (radialSegments + 1);
          int face;

          graphene_vec2_init (&uv2, (x + 1) * 1.0 / radialSegments, 1);
          graphene_vec2_init (&uv3, x * 1.0 / radialSegments, 1);

          face = gthree_geometry_add_face (geometry, i1, i3, i2);
          gthree_geometry_face_set_vertex_normals (geometry, face, &n, &n, &n);

          gthree_geometry_add_uv (geometry, &uv1);
          gthree_geometry_add_uv (geometry, &uv2);
          gthree_geometry_add_uv (geometry, &uv3);
        }
    }

  gthree_geometry_compute_face_normals (geometry);

  return geometry;
};

GthreeGeometry *
gthree_geometry_new_cylinder (float radius,
                              float height)
{
  return gthree_geometry_new_cylinder_full (radius, radius, height, 8, 1, FALSE, 0, 2 * G_PI);
}

GthreeGeometry *
gthree_geometry_new_torus_full (float radius,
                                float tube,
                                int   radialSegments,
                                int   tubularSegments,
                                float arc)
{
  GthreeGeometry *geometry;
  const graphene_vec3_t *vertices;
  int i, j;

  geometry = g_object_new (gthree_geometry_get_type (), NULL);

  for (j = 0; j <= radialSegments; j++)
    {
      for (i = 0; i <= tubularSegments; i++)
        {
          float u = i * arc / tubularSegments;
          float v = j * 2 * G_PI / radialSegments;
          graphene_vec3_t vertex;

          gthree_geometry_add_vertex (geometry,
                                      graphene_vec3_init (&vertex,
                                                          (radius + tube * cos (v)) * cos (u),
                                                          (radius + tube * cos (v)) * sin (u),
                                                          tube * sin (v)));
        }
    }

  vertices = gthree_geometry_get_vertices (geometry);

  for (j = 1; j <= radialSegments; j++)
    {
      for (i = 1; i <= tubularSegments; i++)
        {
          int a = (tubularSegments + 1) * j + i - 1;
          int b = (tubularSegments + 1) * (j - 1) + i - 1;
          int c = (tubularSegments + 1) * (j - 1) + i;
          int d = (tubularSegments + 1) * j + i;
          int face1, face2;

          float u1 = (i - 1) * 1.0 / tubularSegments * arc;
          float u2 = i * 1.0 / tubularSegments * arc;

          graphene_vec3_t na, nb, nc, nd, center;
          graphene_vec2_t uv;

          face1 = gthree_geometry_add_face (geometry, a, b, d);
          face2 = gthree_geometry_add_face (geometry, b, c, d);

          graphene_vec3_init (&center, radius * cos (u1), radius * sin (u1), 0);

          graphene_vec3_subtract (&vertices[a], &center, &na);
          graphene_vec3_normalize (&na, &na);

          graphene_vec3_subtract (&vertices[b], &center, &nb);
          graphene_vec3_normalize (&nb, &nb);

          graphene_vec3_init (&center, radius * cos (u2), radius * sin (u2), 0);

          graphene_vec3_subtract (&vertices[c], &center, &nc);
          graphene_vec3_normalize (&nc, &nc);

          graphene_vec3_subtract (&vertices[d], &center, &nd);
          graphene_vec3_normalize (&nd, &nd);

          gthree_geometry_face_set_vertex_normals (geometry, face1, &na, &nb, &nd);
          gthree_geometry_face_set_vertex_normals (geometry, face2, &nb, &nc, &nd);

          gthree_geometry_add_uv (geometry, graphene_vec2_init (&uv, (i - 1) * 1.0 / tubularSegments, j * 1.0 / radialSegments));
          gthree_geometry_add_uv (geometry, graphene_vec2_init (&uv, (i - 1) * 1.0 / tubularSegments, (j - 1) * 1.0 / radialSegments));
          gthree_geometry_add_uv (geometry, graphene_vec2_init (&uv, i * 1.0 / tubularSegments, j * 1.0 / radialSegments));

          gthree_geometry_add_uv (geometry, graphene_vec2_init (&uv, (i - 1) * 1.0 / tubularSegments, (j - 1) * 1.0 / radialSegments));
          gthree_geometry_add_uv (geometry, graphene_vec2_init (&uv, i * 1.0 / tubularSegments, (j - 1) * 1.0 / radialSegments));
          gthree_geometry_add_uv (geometry, graphene_vec2_init (&uv, i * 1.0 / tubularSegments, j * 1.0 / radialSegments));
        }
    }

  gthree_geometry_compute_face_normals (geometry);

  return geometry;
};

GthreeGeometry *
gthree_geometry_new_torus (float radius,
                           float tube)
{
  return gthree_geometry_new_torus_full (radius, tube, 8, 6, 2 * G_PI);
}
