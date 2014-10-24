#ifndef __GTHREE_PHONG_MATERIAL_H__
#define __GTHREE_PHONG_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_PHONG_MATERIAL      (gthree_phong_material_get_type ())
#define GTHREE_PHONG_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_PHONG_MATERIAL, \
                                                                     GthreePhongMaterial))
#define GTHREE_IS_PHONG_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_PHONG_MATERIAL))

struct _GthreePhongMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;

} GthreePhongMaterialClass;

GthreePhongMaterial *gthree_phong_material_new ();
GType gthree_phong_material_get_type (void) G_GNUC_CONST;

const GdkRGBA * gthree_phong_material_get_color          (GthreePhongMaterial *phong);
void            gthree_phong_material_set_color          (GthreePhongMaterial *phong,
							  const GdkRGBA       *color);
const GdkRGBA * gthree_phong_material_get_ambient_color  (GthreePhongMaterial *phong);
void            gthree_phong_material_set_ambient_color  (GthreePhongMaterial *phong,
							  const GdkRGBA       *color);
const GdkRGBA * gthree_phong_material_get_emissive_color (GthreePhongMaterial *phong);
void            gthree_phong_material_set_emissive_color (GthreePhongMaterial *phong,
							  const GdkRGBA       *color);
const GdkRGBA * gthree_phong_material_get_specular_color (GthreePhongMaterial *phong);
void            gthree_phong_material_set_specular_color (GthreePhongMaterial *phong,
							  const GdkRGBA       *color);
void            gthree_phong_material_set_vertex_colors  (GthreePhongMaterial *phong,
							  GthreeColorType      color_type);
GthreeColorType gthree_phong_material_get_vertex_colors  (GthreePhongMaterial *phong);
void            gthree_phong_material_set_shading_type   (GthreePhongMaterial *phong,
							  GthreeShadingType    shading_type);
void            gthree_phong_material_set_map            (GthreePhongMaterial *phong,
							  GthreeTexture       *texture);
GthreeTexture  *gthree_phong_material_get_map            (GthreePhongMaterial *phong);


G_END_DECLS

#endif /* __GTHREE_PHONGMATERIAL_H__ */
