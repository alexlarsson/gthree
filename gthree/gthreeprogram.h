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
  guint use_vertex_texture : 1;
  guint map : 1;
  guint env_map : 1;
  guint light_map : 1;
  guint normal_map : 1;
  guint specular_map : 1;
  guint alpha_map : 1;
  guint bump_map : 1;
  guint vertex_colors : 1;
  guint fog : 1;
  guint use_fog : 1;
  guint fog_exp : 1;
  guint metal : 1;
  guint wrap_around : 1;
  guint double_sided : 1;
  guint flip_sided : 1;
  guint flat_shading : 1;

  guint unused : 12;

  guint16 max_dir_lights;
  guint16 max_point_lights;
  guint16 max_spot_lights;
  guint16 max_hemi_lights;
  guint16 max_shadows;
  guint16 max_bones;
  float   alpha_test;

  guint16 unused2[4];
} GthreeProgramParameters;

GType gthree_program_get_type (void) G_GNUC_CONST;

GthreeProgram *gthree_program_new (GthreeShader *shader, GthreeProgramParameters *parameters);

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
                                               GthreeProgramParameters *parameters);

G_END_DECLS

#endif /* __GTHREE_PROGRAM_H__ */
