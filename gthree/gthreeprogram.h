#ifndef __GTHREE_PROGRAM_H__
#define __GTHREE_PROGRAM_H__

#include <gtk/gtk.h>

#include "gthreetypes.h"
#include "gthreeenums.h"

G_BEGIN_DECLS

#define GTHREE_TYPE_PROGRAM      (gthree_program_get_type ())
#define GTHREE_PROGRAM(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_PROGRAM, \
                                                             GthreeProgram))
#define GTHREE_IS_PROGRAM(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_PROGRAM))

struct _GthreeProgram {
  GObject parent;

  // TODO: Switch these from string to quarks
  GHashTable *uniform_locations;
  GHashTable *attribute_locations;

  int usedTimes;
  gpointer code;
  GLuint gl_program;
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
  guint vertex_colors : 2; /* GthreeColorType  */
  guint fog : 1;
  guint use_fog : 1;
  guint fog_exp : 1;
  guint metal : 1;
  guint wrap_around : 1;

  guint16 max_dir_lights;
  guint16 max_point_lights;
  guint16 max_spot_lights;
  guint16 max_hemi_lights;
  guint16 max_shadows;
  guint16 max_bones;

  guint double_sided : 1;
  guint flip_sided : 1;
} GthreeProgramParameters;

GType gthree_program_get_type (void) G_GNUC_CONST;

GthreeProgram *gthree_program_new (gpointer code, GthreeMaterial *material, GthreeProgramParameters *parameters);

guint gthree_program_get_program (GthreeProgram *program);

// TODO: Convert to quark
gint gthree_program_lookup_uniform_location (GthreeProgram *program,
                                             const char *uniform);
gint gthree_program_lookup_attribute_location (GthreeProgram *program,
                                               const char *attribute);

G_END_DECLS

#endif /* __GTHREE_PROGRAM_H__ */
