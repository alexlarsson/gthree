#ifndef __GTHREE_PRIVATE_H__
#define __GTHREE_PRIVATE_H__

#include <gthree/gthreeobject.h>
#include <gthree/gthreelight.h>
#include <gthree/gthreegeometry.h>

/* Each hash maps to a specific program (e.g. one with some set of lights), not a particular set of uniform values (like positions/colors/etc) */
typedef struct {
  guint8 num_directional;
  guint8 num_point;
} GthreeLightSetupHash;

struct _GthreeLightSetup
{
  GdkRGBA ambient;

  /* Uniforms */
  GPtrArray *directional;
  GPtrArray *point;

  GthreeLightSetupHash hash;
};

/* Keep track of what state the material is wired up for */
struct _GthreeMaterialProperties
{
  GthreeProgram *program;
  GthreeLightSetupHash light_hash;
} ;

GthreeRenderList *gthree_render_list_new ();
void gthree_render_list_free (GthreeRenderList *list);
void gthree_render_list_init (GthreeRenderList *list);
void gthree_render_list_push (GthreeRenderList *list,
                              GthreeObject *object,
                              GthreeGeometry *geometry,
                              GthreeMaterial *material,
                              GthreeGeometryGroup *group);
void gthree_render_list_sort (GthreeRenderList *list);


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

void gthree_geometry_update           (GthreeGeometry   *geometry);
void gthree_geometry_fill_render_list (GthreeGeometry   *geometry,
                                       GthreeRenderList *list,
                                       GthreeMaterial   *material,
                                       GthreeObject     *object);

gboolean gthree_light_setup_hash_equal (GthreeLightSetupHash *a,
                                        GthreeLightSetupHash *b);

void gthree_light_setup  (GthreeLight   *light,
                          GthreeCamera  *camera,
                          GthreeLightSetup *setup);

GthreeMaterialProperties *gthree_material_get_properties (GthreeMaterial  *material);

graphene_matrix_t *gthree_camera_get_projection_matrix_for_write (GthreeCamera *camera);

#endif /* __GTHREE_PRIVATE_H__ */
