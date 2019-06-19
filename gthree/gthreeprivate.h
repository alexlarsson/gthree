#ifndef __GTHREE_PRIVATE_H__
#define __GTHREE_PRIVATE_H__

#include <gthree/gthreeobject.h>
#include <gthree/gthreelight.h>
#include <gthree/gthreegeometry.h>
#include <gthree/gthreeinterpolant.h>
#include <gthree/gthreekeyframetrack.h>

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

void gthree_texture_set_max_mip_level (GthreeTexture *texture,
                                       int level);
int gthree_texture_get_max_mip_level (GthreeTexture *texture);
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

void gthree_skeleton_update  (GthreeSkeleton *skeleton);
float *gthree_skeleton_get_bone_matrices (GthreeSkeleton *skeleton);

void gthree_light_setup  (GthreeLight   *light,
                          GthreeCamera  *camera,
                          GthreeLightSetup *setup);

GthreeMaterialProperties *gthree_material_get_properties (GthreeMaterial  *material);

graphene_matrix_t *gthree_camera_get_projection_matrix_for_write (GthreeCamera *camera);

void gthree_object_print_tree (GthreeObject *object, int depth);

GthreeInterpolant *gthree_interpolant_create (GType type,
                                              GthreeAttributeArray *parameter_positions,
                                              GthreeAttributeArray *sample_values);

void gthree_interpolant_copy_sample_value (GthreeInterpolant *interpolant, int index);

GthreeKeyframeTrack * gthree_keyframe_track_create  (GType type,
                                                     const char *name,
                                                     GthreeAttributeArray *times,
                                                     GthreeAttributeArray *values);

typedef struct {
  int cache_index;
  int by_clip_cache_index;
} GthreeAnimationActionMixerData;

void  _gthree_animation_action_update            (GthreeAnimationAction *action,
                                                  float                  time,
                                                  float                  delta_time,
                                                  float                  time_direction,
                                                  int                    accu_index);
float _gthree_animation_action_update_weight     (GthreeAnimationAction *action,
                                                  float                  time);
float _gthree_animation_action_update_time_scale (GthreeAnimationAction *action,
                                                  float                  time_scale);
float _gthree_animation_action_update_time        (GthreeAnimationAction *action,
                                                  float                  delta_time);
void  _gthree_animation_action_set_endings       (GthreeAnimationAction *action,
                                                  gboolean               at_start,
                                                  gboolean               at_end,
                                                  gboolean               ping_pong);
void  _gthree_animation_action_schedule_fading   (GthreeAnimationAction *action,
                                                  float                  duration,
                                                  float                  weight_now,
                                                  float                  weight_then);
GPtrArray * _gthree_animation_action_get_property_bindings (GthreeAnimationAction *action);
GPtrArray * _gthree_animation_action_get_interpolants (GthreeAnimationAction *action);

GthreeAnimationActionMixerData *_gthree_animation_action_get_mixer_data (GthreeAnimationAction *action);


void _gthree_animation_mixer_init_action_data (GthreeAnimationActionMixerData *data);

void               _gthree_animation_mixer_activate_action               (GthreeAnimationMixer  *mixer,
                                                                          GthreeAnimationAction *action);
void               _gthree_animation_mixer_deactivate_action             (GthreeAnimationMixer  *mixer,
                                                                          GthreeAnimationAction *action);
gboolean           _gthree_animation_mixer_is_active_action              (GthreeAnimationMixer  *mixer,
                                                                          GthreeAnimationAction *action);
void               _gthree_animation_mixer_take_back_control_interpolant (GthreeAnimationMixer  *mixer,
                                                                          GthreeInterpolant     *interpolant);
GthreeInterpolant *_gthree_animation_mixer_lend_control_interpolant      (GthreeAnimationMixer  *mixer);
void               _gthree_animation_mixer_displatch_event               (GthreeAnimationMixer  *mixer,
                                                                          const char            *type,
                                                                          ...);


int _gthree_interpolant_get_cache_index (GthreeInterpolant *interpolant);
void _gthree_interpolant_set_cache_index (GthreeInterpolant *interpolant,
                                          int cache_index);

char * ghtree_property_sanitize_name (const char *name);

#endif /* __GTHREE_PRIVATE_H__ */
