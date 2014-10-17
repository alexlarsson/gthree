#ifndef __GTHREE_SHADER_H__
#define __GTHREE_SHADER_H__

#include <gtk/gtk.h>
#include <graphene.h>

#include "gthreeuniforms.h"

G_BEGIN_DECLS

#define GTHREE_TYPE_SHADER      (gthree_shader_get_type ())
#define GTHREE_SHADER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_SHADER, \
                                                             GthreeShader))
#define GTHREE_IS_SHADER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_SHADER))


typedef struct {
  GObject parent;

  GthreeUniforms *uniforms;
  char *vertex_shader;
  char *fragment_shader;
} GthreeShader;

typedef struct {
  GObjectClass parent_class;

} GthreeShaderClass;

GType gthree_shader_get_type (void) G_GNUC_CONST;

GthreeShader *gthree_shader_new   ();
GthreeShader *gthree_get_shader_from_library (const char *name);

G_END_DECLS

#endif /* __GTHREE_SHADER_H__ */
