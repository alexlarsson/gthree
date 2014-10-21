#ifndef __GTHREE_MATERIAL_H__
#define __GTHREE_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gtk/gtk.h>

#include "gthreeobject.h"
#include "gthreetypes.h"
#include "gthreeprogram.h"
#include "gthreeshader.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_MATERIAL      (gthree_material_get_type ())
#define GTHREE_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_MATERIAL, GthreeMaterial))
#define GTHREE_MATERIAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_MATERIAL, GthreeMaterialClass))
#define GTHREE_IS_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_MATERIAL))
#define GTHREE_IS_MATERIAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_MATERIAL))
#define GTHREE_MATERIAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_MATERIAL, GthreeMaterialClass))

struct _GthreeMaterial {
  GObject parent;

  // TODO: hide
  gboolean needs_update;

  GthreeProgram *program;
  GthreeShader *shader;
};

typedef struct {
  GObjectClass parent_class;

  GthreeMaterial * (*resolve) (GthreeMaterial *material,
                               int index);

  void          (*set_params) (GthreeMaterial *material,
                               GthreeProgramParameters *params);

  void          (*set_uniforms) (GthreeMaterial *material,
                                 GthreeUniforms *uniforms);

} GthreeMaterialClass;

GthreeMaterial *gthree_material_new ();
GType gthree_material_get_type (void) G_GNUC_CONST;

gboolean        gthree_material_get_is_visible           (GthreeMaterial       *material);
void            gthree_material_set_is_visible           (GthreeMaterial       *material,
                                                          gboolean              is_visible);
gboolean        gthree_material_get_is_transparent       (GthreeMaterial       *material);
void            gthree_material_set_is_transparent       (GthreeMaterial       *material,
                                                          gboolean              is_transparent);
gboolean        gthree_material_get_is_wireframe         (GthreeMaterial       *material);
void            gthree_material_set_is_wireframe         (GthreeMaterial       *material,
                                                          gboolean              is_wireframe);
float           gthree_material_get_wireframe_line_width (GthreeMaterial       *material);
void            gthree_material_set_wireframe_line_width (GthreeMaterial       *material,
                                                          float                 line_width);
float           gthree_material_get_opacity              (GthreeMaterial       *material);
void            gthree_material_set_opacity              (GthreeMaterial       *material,
                                                          float                 opacity);
GthreeBlendMode gthree_material_get_blend_mode           (GthreeMaterial       *material,
                                                          guint                *equation,
                                                          guint                *src_factor,
                                                          guint                *dst_factor);
void            gthree_material_set_blend_mode           (GthreeMaterial       *material,
                                                          GthreeBlendMode       mode,
                                                          guint                 equation,
                                                          guint                 src_factor,
                                                          guint                 dst_factor);
gboolean        gthree_material_get_polygon_offset       (GthreeMaterial       *material,
                                                          float                *factor,
                                                          float                *units);
void            gthree_material_set_polygon_offset       (GthreeMaterial       *material,
                                                          gboolean              polygon_offset,
                                                          float                 factor,
                                                          float                 units);
gboolean        gthree_material_get_depth_test           (GthreeMaterial       *material);
void            gthree_material_set_depth_test           (GthreeMaterial       *material,
                                                          gboolean              depth_test);
gboolean        gthree_material_get_depth_write          (GthreeMaterial       *material);
void            gthree_material_set_depth_write          (GthreeMaterial       *material,
                                                          gboolean              depth_write);
float           gthree_material_get_alpha_test           (GthreeMaterial       *material);
void            gthree_material_set_alpha_test           (GthreeMaterial       *material,
                                                          float                 alpha_test);
GthreeSide      gthree_material_get_side                 (GthreeMaterial       *material);
void            gthree_material_set_side                 (GthreeMaterial       *material,
                                                          GthreeSide            side);
GthreeShader *  gthree_material_get_shader               (GthreeMaterial       *material);
GthreeMaterial *gthree_material_resolve                  (GthreeMaterial       *material,
                                                          int                   index);
void            gthree_material_set_params               (GthreeMaterial       *material,
                                                          GthreeProgramParameters *params);
void            gthree_material_set_uniforms             (GthreeMaterial *material,
                                                          GthreeUniforms *uniforms);



G_END_DECLS

#endif /* __GTHREE_MATERIAL_H__ */
