#ifndef __GTHREE_PRIVATE_H__
#define __GTHREE_PRIVATE_H__

#include <gthree/gthreeobject.h>
#include <gthree/gthreelight.h>
#include <gthree/gthreegeometry.h>
#include <gthree/gthreeinterpolant.h>
#include <gthree/gthreekeyframetrack.h>
#include <gthree/gthreerendertarget.h>
#include <gthree/gthreemesh.h>
#include <gthree/gthreesprite.h>
#include <gthree/gthreelightshadow.h>
#include <gthree/gthreedirectionallightshadow.h>
#include <gthree/gthreespotlightshadow.h>
#include <json-glib/json-glib.h>

//#define DEBUG_LABELS
//#define DEBUG_GROUPS

/* Each hash maps to a specific program (e.g. one with some set of lights), not a particular set of uniform values (like positions/colors/etc) */
typedef struct {
  guint8 num_directional;
  guint8 num_point;
  guint8 num_spot;
  guint8 num_shadow;
  guint8 num_hemi;
  guint8 obj_receive_shadow;
} GthreeLightSetupHash;

struct _GthreeLightSetup
{
  graphene_vec3_t ambient;

  /* Uniforms */
  GPtrArray *directional;
  GPtrArray *directional_shadow_map;
  GArray *directional_shadow_map_matrix;
  GPtrArray *point;
  GPtrArray *point_shadow_map;
  GArray *point_shadow_map_matrix;
  GPtrArray *spot;
  GPtrArray *spot_shadow_map;
  GArray *spot_shadow_map_matrix;
  GPtrArray *shadow;
  GPtrArray *hemi;

  GthreeLightSetupHash hash;
};

/* Keep track of what state the material is wired up for */
struct _GthreeMaterialProperties
{
  GthreeProgram *program; /* Not owned, only use while valid for the owning renderer */
  GthreeLightSetupHash light_hash;
  guint num_clipping_planes;
  guint num_intersection;
};

struct  _GthreeProgramParameters {
  guint precision : 2; /* GthreePrecision */
  guint supports_vertex_textures : 1;
  guint16 output_encoding : 3;
  guint map : 1;
  guint map_encoding : 3;
  guint matcap : 1;
  guint matcap_encoding : 3;
  guint env_map : 1;
  guint env_map_mode : 3;
  guint env_map_encoding : 3;
  guint light_map : 1;
  guint ao_map : 1;
  guint emissive_map : 1;
  guint emissive_map_encoding : 3;
  guint bump_map : 1;
  guint normal_map : 1;
  guint object_space_normal_map : 1;
  guint displacement_map : 1;
  guint specular_map : 1;
  guint roughness_map : 1;
  guint glossiness_map : 1;
  guint metalness_map : 1;
  guint gradient_map : 1;
  guint alpha_map : 1;
  guint combine : 1;
  guint vertex_colors : 1;
  guint vertex_tangents : 1;
  guint fog : 1;
  guint use_fog : 1;
  guint fog_exp : 1;
  guint flat_shading : 1;
  guint size_attenuation : 1;
  guint logarithmic_depth_buffer : 1;
  guint skinning : 1;
  guint use_vertex_texture : 1;
  guint morph_targets : 1;
  guint morph_normals : 1;
  guint premultiplied_alpha : 1;
  guint shadow_map_enabled : 1;
  guint shadow_map_type : 2;
  guint tone_mapping : 1;
  guint physically_correct_lights : 1;
  guint double_sided : 1;
  guint flip_sided : 1;
  guint depth_packing : 2;
  guint dithering : 1;

  guint8 alpha_test;
  guint16 max_bones;
  guint16 max_morph_targets;

  guint16 num_dir_lights;
  guint16 num_point_lights;
  guint16 num_spot_lights;
  guint16 num_hemi_lights;
  guint16 num_rect_area_lights;

  guint16 num_clipping_planes;
  guint16 num_clip_intersection;
};


gboolean gthree_uniform_is_array (GthreeUniform *uniform);
GthreeUniform *gthree_uniform_newq (GQuark name, GthreeUniformType type);

GthreeRenderList *gthree_render_list_new ();
void gthree_render_list_free (GthreeRenderList *list);
void gthree_render_list_init (GthreeRenderList *list);
void gthree_render_list_push (GthreeRenderList *list,
                              GthreeObject *object,
                              GthreeGeometry *geometry,
                              GthreeMaterial *material,
                              GthreeGeometryGroup *group);
void gthree_render_list_sort (GthreeRenderList *list);

guint32 gthree_renderer_get_resource_id (GthreeRenderer *renderer);
void gthree_renderer_mark_realized (GthreeRenderer *renderer,
                                    GthreeResource *resource);
void gthree_renderer_mark_unrealized (GthreeRenderer *renderer,
                                      GthreeResource *resource);
void gthree_resource_mark_dirty (GthreeResource *resource);
gboolean gthree_resource_get_dirty_for (GthreeResource  *resource,
                                        GthreeRenderer   *renderer);
void gthree_resource_mark_clean_for (GthreeResource *resource,
                                     GthreeRenderer *renderer);

guint gthree_renderer_allocate_texture_unit (GthreeRenderer *renderer);

int gthree_texture_get_internal_gl_format (guint gl_format,
                                           guint gl_type);
int gthree_texture_format_to_gl (GthreeTextureFormat format);
int gthree_texture_data_type_to_gl (GthreeDataType type);

void gthree_texture_setup_framebuffer (GthreeTexture *texture,
                                       GthreeRenderer *renderer,
                                       int width,
                                       int height,
                                       guint framebuffer,
                                       int attachement,
                                       int texture_target);

void gthree_texture_set_max_mip_level (GthreeTexture *texture,
                                       int level);
int gthree_texture_get_max_mip_level (GthreeTexture *texture);
void     gthree_texture_load             (GthreeTexture *texture,
                                          GthreeRenderer *renderer,
                                          int            slot);
gboolean gthree_texture_get_needs_update (GthreeTexture *texture,
                                          GthreeRenderer *renderer);
void     gthree_texture_realize          (GthreeTexture *texture,
                                          GthreeRenderer *renderer);
void     gthree_texture_bind             (GthreeTexture *texture,
                                          GthreeRenderer *renderer,
                                          int            slot,
                                          int            target);
void     gthree_texture_set_parameters (guint texture_type,
                                        GthreeTexture *texture,
                                        gboolean is_image_power_of_two);

guint gthree_render_target_get_gl_framebuffer (GthreeRenderTarget *target,
                                               GthreeRenderer *renderer);
void gthree_render_target_realize (GthreeRenderTarget *target,
                                   GthreeRenderer *renderer);
const graphene_rect_t * gthree_render_target_get_viewport (GthreeRenderTarget *target);


GthreeGeometry *gthree_geometry_parse_json (JsonObject *object);
void gthree_geometry_update           (GthreeGeometry   *geometry,
                                       GthreeRenderer *renderer);
void gthree_geometry_fill_render_list (GthreeGeometry   *geometry,
                                       GthreeRenderList *list,
                                       GthreeMaterial   *material,
                                       GPtrArray        *materials,
                                       GthreeObject     *object);

gboolean gthree_light_setup_hash_equal (GthreeLightSetupHash *a,
                                        GthreeLightSetupHash *b);
void gthree_light_set_shadow (GthreeLight   *light,
                              GthreeLightShadow *shadow);

void gthree_skeleton_update  (GthreeSkeleton *skeleton);
float *gthree_skeleton_get_bone_matrices (GthreeSkeleton *skeleton);

void gthree_light_setup  (GthreeLight   *light,
                          GthreeCamera  *camera,
                          GthreeLightSetup *setup);

GthreeLightShadow *gthree_light_shadow_new (GthreeCamera *camera);
void gthree_light_shadow_set_camera (GthreeLightShadow *shadow,
                                     GthreeCamera *camera);
GthreeRenderTarget * gthree_light_shadow_get_map (GthreeLightShadow *shadow);
void gthree_light_shadow_set_map (GthreeLightShadow *shadow,
                                  GthreeRenderTarget *map);
graphene_matrix_t * gthree_light_shadow_get_matrix (GthreeLightShadow *shadow);

GthreeDirectionalLightShadow *gthree_directional_light_shadow_new (void);

GthreeSpotLightShadow *gthree_spot_light_shadow_new (void);
void gthree_spot_light_shadow_update (GthreeSpotLightShadow *shadow,
                                      GthreeSpotLight *light);

GthreeMaterialProperties *gthree_material_get_properties   (GthreeMaterial *material);
void                      gthree_material_mark_valid_for   (GthreeMaterial *material,
                                                            guint32         renderer_id);
gboolean                  gthree_material_is_valid_for     (GthreeMaterial *material,
                                                            guint32         renderer_id);

graphene_matrix_t *gthree_camera_get_projection_matrix_for_write (GthreeCamera *camera);

void gthree_object_print_tree (GthreeObject *object, int depth);

GthreeAttribute *gthree_attribute_parse_json                 (JsonObject           *root,
                                                              const char           *name);

/* These are valid when realized */
int gthree_attribute_get_gl_buffer            (GthreeAttribute *attribute,
                                               GthreeRenderer *renderer);
int gthree_attribute_get_gl_type              (GthreeAttribute *attribute);
int gthree_attribute_get_gl_bytes_per_element (GthreeAttribute *attribute);

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

void gthree_mesh_material_set_num_supported_morph_targets (GthreeMeshMaterial *material,
                                                           int num_supported);
void gthree_mesh_material_set_num_supported_morph_normals (GthreeMeshMaterial *material,
                                                           int num_supported);

void            gthree_renderer_push_current (GthreeRenderer *renderer);
void            gthree_renderer_pop_current  (GthreeRenderer *renderer);
GthreeRenderer *gthree_renderer_get_current  (void);

typedef enum {
  GTHREE_RESOURCE_KIND_TEXTURE,
  GTHREE_RESOURCE_KIND_BUFFER,
  GTHREE_RESOURCE_KIND_FRAMEBUFFER,
  GTHREE_RESOURCE_KIND_RENDERBUFFER,
} GthreeResourceKind;

void gthree_renderer_lazy_delete (GthreeRenderer *renderer,
                                  GthreeResourceKind kind,
                                  guint             id);

GthreeGeometry *gthree_sprite_get_geometry (GthreeSprite *sprite);

#endif /* __GTHREE_PRIVATE_H__ */
