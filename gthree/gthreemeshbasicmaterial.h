#ifndef __GTHREE_BASIC_MATERIAL_H__
#define __GTHREE_BASIC_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_BASIC_MATERIAL      (gthree_mesh_basic_material_get_type ())
#define GTHREE_BASIC_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_BASIC_MATERIAL, \
                                                                     GthreeMeshBasicMaterial))
#define GTHREE_IS_BASIC_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_BASIC_MATERIAL))

struct _GthreeMeshBasicMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshBasicMaterialClass;

GthreeMeshBasicMaterial *gthree_mesh_basic_material_new ();
GType gthree_mesh_basic_material_get_type (void) G_GNUC_CONST;

const GdkRGBA *   gthree_mesh_basic_material_get_color            (GthreeMeshBasicMaterial *basic);
void              gthree_mesh_basic_material_set_color            (GthreeMeshBasicMaterial *basic,
                                                                   const GdkRGBA           *color);
void              gthree_mesh_basic_material_set_map              (GthreeMeshBasicMaterial *basic,
                                                                   GthreeTexture           *texture);
GthreeTexture  *  gthree_mesh_basic_material_get_map              (GthreeMeshBasicMaterial *basic);
void              gthree_mesh_basic_material_set_env_map          (GthreeMeshBasicMaterial *basic,
                                                                   GthreeTexture           *texture);
GthreeTexture  *  gthree_mesh_basic_material_get_env_map          (GthreeMeshBasicMaterial *basic);
float             gthree_mesh_basic_material_get_refraction_ratio (GthreeMeshBasicMaterial *basic);
void              gthree_mesh_basic_material_set_refraction_ratio (GthreeMeshBasicMaterial *basic,
                                                                   float                    ratio);
float             gthree_mesh_basic_material_get_reflectivity     (GthreeMeshBasicMaterial *basic);
void              gthree_mesh_basic_material_set_reflectivity     (GthreeMeshBasicMaterial *basic,
                                                                   float                    reflectivity);
GthreeOperation   gthree_mesh_basic_material_get_combine          (GthreeMeshBasicMaterial *basic);
void              gthree_mesh_basic_material_set_combine          (GthreeMeshBasicMaterial *basic,
                                                                   GthreeOperation          combine);

G_END_DECLS

#endif /* __GTHREE_BASICMATERIAL_H__ */
