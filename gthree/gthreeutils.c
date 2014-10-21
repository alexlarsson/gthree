#include "gthreeutils.h"

void
gthree_plane_init (GthreePlane       *plane,
                   const graphene_vec3_t   *normal,
                   float              constant)
{
  plane->normal = *normal;
  plane->constant = constant;
}

void
gthree_plane_init_components (GthreePlane       *plane,
                              float              x,
                              float              y,
                              float              z,
                              float              w)
{
  graphene_vec3_init (&plane->normal, x, y, z);
  plane->constant = w;
}

void
gthree_plane_init_vec4 (GthreePlane       *plane,
                        const graphene_vec4_t   *v)
{
  graphene_vec4_get_xyz (v, &plane->normal);
  plane->constant = graphene_vec4_get_w (v);
}

void
gthree_plane_normalize (GthreePlane *plane)
{
  float normal_length = graphene_vec3_length (&plane->normal);
  graphene_vec3_normalize (&plane->normal, &plane->normal);
  plane->constant /= normal_length;
}

float
gthree_plane_distance_to_point (const GthreePlane *plane,
                                const graphene_vec3_t *point)
{
  return graphene_vec3_dot (&plane->normal, point) + plane->constant;
}

void
gthree_box3_init_empty (GthreeBox3 *box)
{
  graphene_vec3_init (&box->min, INFINITY, INFINITY, INFINITY);
  graphene_vec3_init (&box->max, -INFINITY, -INFINITY, -INFINITY);
}

void
gthree_box3_expand_by_point (GthreeBox3 *box,
                             const graphene_vec3_t *point)
{
  graphene_vec3_min (&box->min, point, &box->min);
  graphene_vec3_max (&box->max, point, &box->max);
}

// TODO: Use scalar multiply when available 
static graphene_vec3_t *
my_vec3_scale (const graphene_vec3_t *src, float s, graphene_vec3_t *dst)
{
  float x, y, z;
  x = graphene_vec3_get_x (src) * s;
  y = graphene_vec3_get_y (src) * s;
  z = graphene_vec3_get_z (src) * s;

  return graphene_vec3_init (dst, x, y, z);
}

graphene_vec3_t *
gthree_box3_get_center (GthreeBox3 *box,
                        graphene_vec3_t *center)
{
  graphene_vec3_t sum;

  graphene_vec3_add (&box->min, &box->max, &sum);

  my_vec3_scale (&sum, 0.5, center);

  return center;
}

void
gthree_box3_init_from_points (GthreeBox3 *box,
                              const graphene_vec3_t *points,
                              int num_points)
{
  int i;

  gthree_box3_init_empty (box);

  for ( i = 0; i < num_points; i++)
    gthree_box3_expand_by_point (box, &points[i]);
}

static float
distance_squared (const graphene_vec3_t *a,
                  const graphene_vec3_t *b)
{
  graphene_vec3_t v;

  graphene_vec3_subtract (a, b, &v);
  return graphene_vec3_dot (&v, &v);
}


void
gthree_sphere_init_from_points (GthreeSphere *sphere,
                                const graphene_vec3_t *points,
                                int num_points,
                                const graphene_vec3_t *optional_center)
{
  GthreeBox3 box;
  float max_radius_sq;
  int i;

  if (optional_center)
    sphere->center = *optional_center;
  else
    {
      gthree_box3_init_from_points (&box, points, num_points);
      gthree_box3_get_center (&box, &sphere->center);
    }

  max_radius_sq = 0;
  for (i = 0; i < num_points; i++)
    max_radius_sq = fmaxf (max_radius_sq, distance_squared (&sphere->center, &points[i]));

  sphere->radius = sqrtf (max_radius_sq);
}

void
gthree_sphere_transform (GthreeSphere *sphere,
                         const graphene_matrix_t *matrix)
{
  float max_scale;
  graphene_vec4_t v;

  /* TODO: Would be nice if graphene exposed this as a transform vec3 + pos */
  graphene_vec4_init_from_vec3 (&v, &sphere->center, 1.0);
  graphene_matrix_transform_vec4 (matrix, &v, &v);
  graphene_vec4_get_xyz (&v, &sphere->center);

  max_scale = graphene_matrix_get_x_scale (matrix);
  max_scale = fmaxf (max_scale, graphene_matrix_get_y_scale (matrix));
  max_scale = fmaxf (max_scale, graphene_matrix_get_z_scale (matrix));

  sphere->radius *= max_scale;
}

void
gthree_frustum_init_from_matrix (GthreeFrustum *frustum,
                                 const graphene_matrix_t *matrix)
{
  graphene_vec4_t r1, r2, r3, r4, t;
  graphene_matrix_t m;

  graphene_matrix_transpose (matrix, &m);

  graphene_matrix_get_row (&m, 0, &r1);
  graphene_matrix_get_row (&m, 1, &r2);
  graphene_matrix_get_row (&m, 2, &r3);
  graphene_matrix_get_row (&m, 3, &r4);

  graphene_vec4_subtract (&r4, &r1, &t);
  gthree_plane_init_vec4 (&frustum->planes[0], &t);
  gthree_plane_normalize (&frustum->planes[0]);

  graphene_vec4_add (&r4, &r1, &t);
  gthree_plane_init_vec4 (&frustum->planes[1], &t);
  gthree_plane_normalize (&frustum->planes[1]);

  graphene_vec4_subtract (&r4, &r2, &t);
  gthree_plane_init_vec4 (&frustum->planes[2], &t);
  gthree_plane_normalize (&frustum->planes[2]);

  graphene_vec4_add (&r4, &r2, &t);
  gthree_plane_init_vec4 (&frustum->planes[3], &t);
  gthree_plane_normalize (&frustum->planes[3]);

  graphene_vec4_subtract (&r4, &r3, &t);
  gthree_plane_init_vec4 (&frustum->planes[4], &t);
  gthree_plane_normalize (&frustum->planes[4]);

  graphene_vec4_add (&r4, &r3, &t);
  gthree_plane_init_vec4 (&frustum->planes[5], &t);
  gthree_plane_normalize (&frustum->planes[5]);
}

gboolean
gthree_frustum_intersects_sphere (GthreeFrustum           *frustum,
                                  const GthreeSphere      *sphere)
{
  int i;

  for ( i = 0; i < 6; i ++)
    {
      float distance = gthree_plane_distance_to_point (&frustum->planes[i], &sphere->center);
      if (distance < -sphere->radius )
        return FALSE;
    }

  return TRUE;
}
