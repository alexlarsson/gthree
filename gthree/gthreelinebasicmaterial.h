#ifndef __GTHREE_LINE_BASIC_MATERIAL_H__
#define __GTHREE_LINE_BASIC_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_LINE_BASIC_MATERIAL      (gthree_line_basic_material_get_type ())
#define GTHREE_LINE_BASIC_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_LINE_BASIC_MATERIAL, \
                                                                     GthreeLineBasicMaterial))
#define GTHREE_IS_LINE_BASIC_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_LINE_BASIC_MATERIAL))

typedef struct _GthreeLineBasicMaterial {
  GthreeMaterial parent;
} GthreeLineBasicMaterial;

typedef struct {
  GthreeMaterialClass parent_class;

} GthreeLineBasicMaterialClass;

GthreeLineBasicMaterial *gthree_line_basic_material_new ();
GType gthree_line_basic_material_get_type (void) G_GNUC_CONST;

const GdkRGBA *   gthree_line_basic_material_get_color         (GthreeLineBasicMaterial *line_basic);
void              gthree_line_basic_material_set_color         (GthreeLineBasicMaterial *line_basic,
                                                                const GdkRGBA       *color);

G_END_DECLS

#endif /* __GTHREE_LINE_BASICMATERIAL_H__ */
