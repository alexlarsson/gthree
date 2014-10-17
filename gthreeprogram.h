#ifndef __GTHREE_PROGRAM_H__
#define __GTHREE_PROGRAM_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_PROGRAM      (gthree_program_get_type ())
#define GTHREE_PROGRAM(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_PROGRAM, \
                                                             GthreeProgram))
#define GTHREE_IS_PROGRAM(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_PROGRAM))

typedef struct {
  GObject parent;
} GthreeProgram;

typedef struct {
  GObjectClass parent_class;

} GthreeProgramClass;

GType gthree_program_get_type (void) G_GNUC_CONST;

GthreeProgram *gthree_program_new ();

guint gthree_program_get_program (GthreeProgram *program);

G_END_DECLS

#endif /* __GTHREE_PROGRAM_H__ */
