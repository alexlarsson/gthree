#ifndef __GTHREE_PROGRAM_H__
#define __GTHREE_PROGRAM_H__

#include <glib-object.h>

#include <gthree/gthreetypes.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeshader.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_PROGRAM      (gthree_program_get_type ())
#define GTHREE_PROGRAM(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_PROGRAM, \
                                                             GthreeProgram))
#define GTHREE_IS_PROGRAM(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_PROGRAM))

struct _GthreeProgram {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;

} GthreeProgramClass;

typedef struct {
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
  guint shadow_map_type_ : 2;
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

} GthreeProgramParameters;

GType gthree_program_get_type (void) G_GNUC_CONST;

GthreeProgram *gthree_program_new (GthreeShader *shader,
                                   GthreeProgramParameters *parameters,
                                   GthreeRenderer *renderer);

void gthree_program_use                                   (GthreeProgram *program);
gint gthree_program_lookup_uniform_location               (GthreeProgram *program,
                                                           GQuark         uniform);
gint gthree_program_lookup_attribute_location             (GthreeProgram *program,
                                                           GQuark         attribute);
GHashTable * gthree_program_get_attribute_locations (GthreeProgram *program);
gint gthree_program_lookup_uniform_location_from_string   (GthreeProgram *program,
                                                           const char    *uniform);
gint gthree_program_lookup_attribute_location_from_string (GthreeProgram *program,
                                                           const char    *attribute);

typedef struct _GthreeProgramCache GthreeProgramCache;

GthreeProgramCache *gthree_program_cache_new  (void);
void                gthree_program_cache_free (GthreeProgramCache      *cache);
GthreeProgram *     gthree_program_cache_get  (GthreeProgramCache      *cache,
                                               GthreeShader            *shader,
                                               GthreeProgramParameters *parameters,
                                               GthreeRenderer          *renderer);

G_END_DECLS

#endif /* __GTHREE_PROGRAM_H__ */
