#ifndef __GTHREE_PHONG_MATERIAL_H__
#define __GTHREE_PHONG_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshbasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_PHONG_MATERIAL      (gthree_mesh_phong_material_get_type ())
#define GTHREE_PHONG_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_PHONG_MATERIAL, \
                                                                     GthreeMeshPhongMaterial))
#define GTHREE_IS_PHONG_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_PHONG_MATERIAL))

struct _GthreeMeshPhongMaterial {
  GthreeMeshBasicMaterial parent;
};

typedef struct {
  GthreeMeshBasicMaterialClass parent_class;

} GthreeMeshPhongMaterialClass;

GthreeMeshPhongMaterial *gthree_mesh_phong_material_new ();
GType gthree_mesh_phong_material_get_type (void) G_GNUC_CONST;

float          gthree_mesh_phong_material_get_shininess      (GthreeMeshPhongMaterial *phong);
void           gthree_mesh_phong_material_set_shininess      (GthreeMeshPhongMaterial *phong,
                                                              float                    shininess);
const GdkRGBA *gthree_mesh_phong_material_get_emissive_color (GthreeMeshPhongMaterial *phong);
void           gthree_mesh_phong_material_set_emissive_color (GthreeMeshPhongMaterial *phong,
                                                              const GdkRGBA           *color);
const GdkRGBA *gthree_mesh_phong_material_get_specular_color (GthreeMeshPhongMaterial *phong);
void           gthree_mesh_phong_material_set_specular_color (GthreeMeshPhongMaterial *phong,
                                                              const GdkRGBA           *color);

G_END_DECLS

#endif /* __GTHREE_PHONGMATERIAL_H__ */
