#include <math.h>
#include <epoxy/gl.h>

#include "gthreeprogram.h"

typedef struct {
  int dummy;
} GthreeProgramPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeProgram, gthree_program, G_TYPE_OBJECT);

GthreeProgram *
gthree_program_new ()
{
  GthreeProgram *program;

  program = g_object_new (gthree_program_get_type (),
                          NULL);

  return program;
}

static void
gthree_program_init (GthreeProgram *program)
{
  //GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

}

static void
gthree_program_finalize (GObject *obj)
{
  //GthreeProgram *program = GTHREE_PROGRAM (obj);
  //GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

  G_OBJECT_CLASS (gthree_program_parent_class)->finalize (obj);
}

static void
gthree_program_class_init (GthreeProgramClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_program_finalize;
}

guint
gthree_program_get_program (GthreeProgram *program)
{
  return 0;
}
