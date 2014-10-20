#ifndef __GTHREE_MATERIAL_H__
#define __GTHREE_MATERIAL_H__

#include <gtk/gtk.h>

#include "gthreeobject.h"
#include "gthreetypes.h"
#include "gthreeprogram.h"
#include "gthreeshader.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_MATERIAL      (gthree_material_get_type ())
#define GTHREE_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_MATERIAL, \
                                                             GthreeMaterial))
#define GTHREE_IS_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_MATERIAL))

struct _GthreeMaterial {
  GObject parent;

  // TODO: hide
  gboolean needs_update;

  GthreeProgram *program;
  GthreeShader *shader;
};

typedef struct {
  GObjectClass parent_class;

} GthreeMaterialClass;

GthreeMaterial *gthree_material_new ();
GType gthree_material_get_type (void) G_GNUC_CONST;

gboolean gthree_material_get_is_visible (GthreeMaterial *material);
gboolean gthree_material_get_is_transparent (GthreeMaterial *material);
gboolean gthree_material_get_is_wireframe (GthreeMaterial *material);
float gthree_material_get_wireframe_line_width (GthreeMaterial *material);

GthreeBlendMode gthree_material_get_blend_mode (GthreeMaterial *material,
                                                GthreeBlendEquation *equation,
                                                GthreeBlendSrcFactor *src_factor,
                                                GthreeBlendDstFactor *dst_factor);

gboolean gthree_material_get_polygon_offset (GthreeMaterial *material,
                                             float *factor, float *units);

gboolean gthree_material_get_depth_test (GthreeMaterial *material);
gboolean gthree_material_get_depth_write (GthreeMaterial *material);

GthreeSide gthree_material_get_side (GthreeMaterial *material);

GthreeShader *gthree_material_get_shader (GthreeMaterial *material);


G_END_DECLS

#endif /* __GTHREE_MATERIAL_H__ */
