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

typedef struct _GthreeProgramParameters GthreeProgramParameters;

GTHREE_API
GType gthree_program_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeProgram *gthree_program_new (GthreeShader *shader,
                                   GthreeProgramParameters *parameters,
                                   GthreeRenderer *renderer);

GTHREE_API
void gthree_program_use                                   (GthreeProgram *program);
GTHREE_API
gint gthree_program_lookup_uniform_location               (GthreeProgram *program,
                                                           GQuark         uniform);
GTHREE_API
gint gthree_program_lookup_attribute_location             (GthreeProgram *program,
                                                           GQuark         attribute);
GTHREE_API
GHashTable * gthree_program_get_attribute_locations (GthreeProgram *program);
GTHREE_API
gint gthree_program_lookup_uniform_location_from_string   (GthreeProgram *program,
                                                           const char    *uniform);
GTHREE_API
gint gthree_program_lookup_attribute_location_from_string (GthreeProgram *program,
                                                           const char    *attribute);

typedef struct _GthreeProgramCache GthreeProgramCache;

GTHREE_API
GthreeProgramCache *gthree_program_cache_new  (void);
GTHREE_API
void                gthree_program_cache_free (GthreeProgramCache      *cache);
GTHREE_API
GthreeProgram *     gthree_program_cache_get  (GthreeProgramCache      *cache,
                                               GthreeShader            *shader,
                                               GthreeProgramParameters *parameters,
                                               GthreeRenderer          *renderer);

G_END_DECLS

#endif /* __GTHREE_PROGRAM_H__ */
