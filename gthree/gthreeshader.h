#ifndef __GTHREE_SHADER_H__
#define __GTHREE_SHADER_H__

#include <glib-object.h>
#include <graphene.h>

#include <gthree/gthreeuniforms.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_SHADER      (gthree_shader_get_type ())
#define GTHREE_SHADER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_SHADER, \
                                                             GthreeShader))
#define GTHREE_IS_SHADER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_SHADER))


typedef struct {
  GObject parent;
} GthreeShader;

typedef struct {
  GObjectClass parent_class;

} GthreeShaderClass;

GType gthree_shader_get_type (void) G_GNUC_CONST;

GthreeShader *  gthree_shader_new   (GPtrArray *defines,
                                     GthreeUniforms *uniforms,
                                     const char *vertex_shader_text,
                                     const char *fragment_shader_text);

GthreeShader *  gthree_shader_clone                                (GthreeShader  *shader);
GPtrArray      *gthree_shader_get_defines                          (GthreeShader  *shader);
GthreeUniforms *gthree_shader_get_uniforms                         (GthreeShader  *shader);
const char *    gthree_shader_get_vertex_shader_text               (GthreeShader  *shader);
const char *    gthree_shader_get_fragment_shader_text             (GthreeShader  *shader);
void            gthree_shader_update_uniform_locations_for_program (GthreeShader  *shader,
                                                                    GthreeProgram *program);


GthreeShader *gthree_get_shader_from_library   (const char *name);
GthreeShader *gthree_clone_shader_from_library (const char *name);


G_END_DECLS

#endif /* __GTHREE_SHADER_H__ */
