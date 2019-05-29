#ifndef __GTHREE_MATERIAL_H__
#define __GTHREE_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreetypes.h>
#include <gthree/gthreeprogram.h>
#include <gthree/gthreeshader.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MATERIAL      (gthree_material_get_type ())
#define GTHREE_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_MATERIAL, GthreeMaterial))
#define GTHREE_MATERIAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_MATERIAL, GthreeMaterialClass))
#define GTHREE_IS_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_MATERIAL))
#define GTHREE_IS_MATERIAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_MATERIAL))
#define GTHREE_MATERIAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_MATERIAL, GthreeMaterialClass))

struct _GthreeMaterial {
  GObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMaterial, g_object_unref)

typedef struct {
  GObjectClass parent_class;

  GthreeMaterial * (*resolve) (GthreeMaterial *material,
                               int index);

  GthreeShader * (*get_shader) (GthreeMaterial *material);

  void          (*set_params) (GthreeMaterial *material,
                               GthreeProgramParameters *params);

  void          (*set_uniforms) (GthreeMaterial *material,
                                 GthreeUniforms *uniforms,
                                 GthreeCamera *camera);

  void (*load_default_attribute) (GthreeMaterial       *material,
                                  int                   attribute_location,
                                  GQuark                attribute);

  gboolean           (*needs_view_matrix) (GthreeMaterial *material);
  gboolean           (*needs_camera_pos) (GthreeMaterial *material);
  gboolean           (*needs_lights)  (GthreeMaterial *material);

} GthreeMaterialClass;

GType gthree_material_get_type (void) G_GNUC_CONST;

gboolean          gthree_material_get_is_visible           (GthreeMaterial          *material);
void              gthree_material_set_is_visible           (GthreeMaterial          *material,
                                                            gboolean                 is_visible);
gboolean          gthree_material_get_is_transparent       (GthreeMaterial          *material);
void              gthree_material_set_is_transparent       (GthreeMaterial          *material,
                                                            gboolean                 is_transparent);
float             gthree_material_get_opacity              (GthreeMaterial          *material);
void              gthree_material_set_opacity              (GthreeMaterial          *material,
                                                            float                    opacity);
GthreeBlendMode   gthree_material_get_blend_mode           (GthreeMaterial          *material,
                                                            guint                   *equation,
                                                            guint                   *src_factor,
                                                            guint                   *dst_factor);
void              gthree_material_set_blend_mode           (GthreeMaterial          *material,
                                                            GthreeBlendMode          mode,
                                                            guint                    equation,
                                                            guint                    src_factor,
                                                            guint                    dst_factor);
gboolean          gthree_material_get_polygon_offset       (GthreeMaterial          *material,
                                                            float                   *factor,
                                                            float                   *units);
void              gthree_material_set_polygon_offset       (GthreeMaterial          *material,
                                                            gboolean                 polygon_offset,
                                                            float                    factor,
                                                            float                    units);
gboolean          gthree_material_get_depth_test           (GthreeMaterial          *material);
void              gthree_material_set_depth_test           (GthreeMaterial          *material,
                                                            gboolean                 depth_test);
gboolean          gthree_material_get_depth_write          (GthreeMaterial          *material);
void              gthree_material_set_depth_write          (GthreeMaterial          *material,
                                                            gboolean                 depth_write);
float             gthree_material_get_alpha_test           (GthreeMaterial          *material);
void              gthree_material_set_alpha_test           (GthreeMaterial          *material,
                                                            float                    alpha_test);
GthreeSide        gthree_material_get_side                 (GthreeMaterial          *material);
void              gthree_material_set_side                 (GthreeMaterial          *material,
                                                            GthreeSide               side);
void              gthree_material_set_vertex_colors        (GthreeMaterial          *material,
                                                            gboolean                 vertex_colors);
gboolean          gthree_material_get_vertex_colors        (GthreeMaterial         *material);
GthreeShader *    gthree_material_get_shader               (GthreeMaterial          *material);
GthreeMaterial *  gthree_material_resolve                  (GthreeMaterial          *material,
                                                            int                      index);
void              gthree_material_set_params               (GthreeMaterial          *material,
                                                            GthreeProgramParameters *params);
void              gthree_material_set_uniforms             (GthreeMaterial          *material,
                                                            GthreeUniforms          *uniforms,
                                                            GthreeCamera            *camera);
void              gthree_material_load_default_attribute   (GthreeMaterial          *material,
                                                            int                      attribute_location,
                                                            GQuark                   attribute);
gboolean          gthree_material_get_needs_update         (GthreeMaterial          *material);
void              gthree_material_set_needs_update         (GthreeMaterial          *material,
                                                            gboolean                 needs_update);
gboolean          gthree_material_needs_camera_pos         (GthreeMaterial          *material);
gboolean          gthree_material_needs_view_matrix        (GthreeMaterial          *material);
gboolean          gthree_material_needs_lights             (GthreeMaterial          *material);

G_END_DECLS

#endif /* __GTHREE_MATERIAL_H__ */
