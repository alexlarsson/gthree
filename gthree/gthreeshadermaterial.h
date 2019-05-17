#ifndef __GTHREE_SHADER_MATERIAL_H__
#define __GTHREE_SHADER_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_SHADER_MATERIAL      (gthree_shader_material_get_type ())
#define GTHREE_SHADER_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_SHADER_MATERIAL, \
                                                                     GthreeShaderMaterial))
#define GTHREE_IS_SHADER_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_SHADER_MATERIAL))

struct _GthreeShaderMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;

} GthreeShaderMaterialClass;

GType gthree_shader_material_get_type (void) G_GNUC_CONST;

GthreeShaderMaterial *gthree_shader_material_new (GthreeShader *shader);

GthreeShadingType gthree_shader_material_get_shading_type  (GthreeShaderMaterial *shader);
void              gthree_shader_material_set_shading_type  (GthreeShaderMaterial *shader,
                                                            GthreeShadingType     shading_type);
void              gthree_shader_material_set_vertex_colors (GthreeShaderMaterial *shader,
                                                            GthreeColorType       color_type);
GthreeColorType   gthree_shader_material_get_vertex_colors (GthreeShaderMaterial *shader);
void              gthree_shader_material_set_use_lights    (GthreeShaderMaterial *shader,
                                                            gboolean              use_lights);
gboolean          gthree_shader_material_get_use_lights    (GthreeShaderMaterial *shader);

G_END_DECLS

#endif /* __GTHREE_SHADER_MATERIAL_H__ */
