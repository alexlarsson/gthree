#ifndef __GTHREE_BASIC_MATERIAL_H__
#define __GTHREE_BASIC_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include "gthreematerial.h"
#include "gthreetexture.h"

G_BEGIN_DECLS


#define GTHREE_TYPE_BASIC_MATERIAL      (gthree_basic_material_get_type ())
#define GTHREE_BASIC_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_BASIC_MATERIAL, \
                                                                     GthreeBasicMaterial))
#define GTHREE_IS_BASIC_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_BASIC_MATERIAL))

struct _GthreeBasicMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;

} GthreeBasicMaterialClass;

GthreeBasicMaterial *gthree_basic_material_new ();
GType gthree_basic_material_get_type (void) G_GNUC_CONST;

const GdkRGBA * gthree_basic_material_get_color         (GthreeBasicMaterial *basic);
void            gthree_basic_material_set_color         (GthreeBasicMaterial *basic,
                                                         const GdkRGBA       *color);
void            gthree_basic_material_set_vertex_colors (GthreeBasicMaterial *basic,
                                                         GthreeColorType      color_type);
GthreeColorType gthree_basic_material_get_vertex_colors (GthreeBasicMaterial *basic);
void            gthree_basic_material_set_map           (GthreeBasicMaterial *basic,
                                                         GthreeTexture       *texture);
GthreeTexture  *gthree_basic_material_get_map           (GthreeBasicMaterial *basic);

G_END_DECLS

#endif /* __GTHREE_BASICMATERIAL_H__ */
