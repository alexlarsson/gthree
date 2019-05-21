#ifndef __GTHREE_PRIVATE_H__
#define __GTHREE_PRIVATE_H__

#include <gthree/gthreeobject.h>
#include <gthree/gthreelight.h>
#include <gthree/gthreebufferprivate.h>

struct _GthreeLightSetup
{
  GdkRGBA ambient;

  int dir_len;
  int dir_count;
  GArray *dir_colors;
  GArray *dir_positions;

  int point_len;
  int point_count;
  GArray *point_colors;
  GArray *point_positions;
  GArray *point_distances;

  int spot_len;
  int spot_count;
  GArray *spot_colors;
  GArray *spot_positions;
  GArray *spot_distances;
  GArray *spot_directions;
  GArray *spot_angles_cos;
  GArray *spot_exponents;

  int hemi_len;
  int hemi_count;
  GArray *hemi_sky_colors;
  GArray *hemi_ground_colors;
  GArray *hemi_positions;
};

guint gthree_renderer_allocate_texture_unit (GthreeRenderer *renderer);

void     gthree_texture_load             (GthreeTexture *texture,
                                          int            slot);
gboolean gthree_texture_get_needs_update (GthreeTexture *texture);
void     gthree_texture_set_needs_update (GthreeTexture *texture,
                                          gboolean       needs_update);
void     gthree_texture_bind             (GthreeTexture *texture,
                                          int            slot,
                                          int            target);
void     gthree_texture_set_parameters (guint texture_type,
                                        GthreeTexture *texture,
                                        gboolean is_image_power_of_two);

GPtrArray *gthree_geometry_create_buffers  (GthreeGeometry *geometry,
                                            GthreeMaterial *material,
                                            GthreeObject   *object);
void gthree_geometry_update                (GthreeGeometry *geometry,
                                            GthreeMaterial *material);
void gthree_geometry_add_buffers_to_object (GthreeGeometry *geometry,
                                            GthreeMaterial *material,
                                            GthreeObject   *object);

void   gthree_light_setup (GthreeLight       *light,
                           GthreeLightSetup *light_setup);

graphene_matrix_t *gthree_camera_get_projection_matrix_for_write (GthreeCamera *camera);

#endif /* __GTHREE_PRIVATE_H__ */
