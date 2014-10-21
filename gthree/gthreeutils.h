#ifndef __GTHREE_UTILS_H__
#define __GTHREE_UTILS_H__

#include <glib.h>

#include <graphene.h>

G_BEGIN_DECLS

typedef struct {
  graphene_vec3_t normal;
  float constant;
} GthreePlane;

typedef struct {
  graphene_vec3_t center;
  float radius;
} GthreeSphere;

typedef struct {
  graphene_vec3_t min;
  graphene_vec3_t max;
} GthreeBox3;

typedef struct {
  GthreePlane planes[6];
} GthreeFrustum;

void  gthree_plane_init              (GthreePlane           *plane,
                                      const graphene_vec3_t *normal,
                                      float                  constant);
void  gthree_plane_init_vec4         (GthreePlane           *plane,
                                      const graphene_vec4_t *v);
void  gthree_plane_init_components   (GthreePlane           *plane,
                                      float                  x,
                                      float                  y,
                                      float                  z,
                                      float                  w);
void  gthree_plane_normalize         (GthreePlane           *plane);
void  gthree_plane_negate            (GthreePlane           *plane);
float gthree_plane_distance_to_point (const GthreePlane     *plane,
                                      const graphene_vec3_t *point);


void gthree_box3_init_empty       (GthreeBox3            *box);
void gthree_box3_init_from_points (GthreeBox3            *box,
                                   const graphene_vec3_t *points,
                                   int                    num_points);
void gthree_box3_expand_by_point  (GthreeBox3            *box,
                                   const graphene_vec3_t *point);

void gthree_sphere_init_from_points (GthreeSphere            *sphere,
                                     const graphene_vec3_t   *points,
                                     int                      num_points,
                                     const graphene_vec3_t   *optional_center);
void gthree_sphere_transform        (GthreeSphere            *sphere,
                                     const graphene_matrix_t *matrix);



void     gthree_frustum_init_from_matrix  (GthreeFrustum           *frustum,
                                           const graphene_matrix_t *matrix);
gboolean gthree_frustum_intersects_sphere (GthreeFrustum           *frustum,
                                           const GthreeSphere      *sphere);

G_END_DECLS

#endif /* __GTHREE_UTILS_H__ */
