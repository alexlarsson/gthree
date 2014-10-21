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
          GthreeFace *face;
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

          face = gthree_face_new (a + offset, b + offset, d + offset);
          gthree_face_set_normal (face, &normalv);
          // TODO
          //face.vertexNormals.push( normal.clone(), normal.clone(), normal.clone() );
          gthree_face_set_material_index (face, materialIndex);

          /* Take ownership */
          gthree_geometry_add_face (geometry, face);

          gthree_geometry_add_uv (geometry, &uva);
          gthree_geometry_add_uv (geometry, &uvb);
          gthree_geometry_add_uv (geometry, &uvd);

          face = gthree_face_new (b + offset, c + offset, d + offset);
          gthree_face_set_normal (face, &normalv);
          // TODO
          //face.vertexNormals.push( normal.clone(), normal.clone(), normal.clone() );
          gthree_face_set_material_index (face, materialIndex);

          gthree_geometry_add_face (geometry, face);

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
