#ifndef __GTHREE_STANDARD_MATERIAL_H__
#define __GTHREE_STANDARD_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshbasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_STANDARD_MATERIAL      (gthree_mesh_standard_material_get_type ())
#define GTHREE_STANDARD_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_STANDARD_MATERIAL, \
                                                                     GthreeMeshStandardMaterial))
#define GTHREE_IS_STANDARD_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_STANDARD_MATERIAL))

struct _GthreeMeshStandardMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshStandardMaterialClass;

GthreeMeshStandardMaterial *gthree_mesh_standard_material_new ();
GType gthree_mesh_standard_material_get_type (void) G_GNUC_CONST;

const GdkRGBA *        gthree_mesh_standard_material_get_color               (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_color               (GthreeMeshStandardMaterial *standard,
                                                                              const GdkRGBA              *color);
const GdkRGBA *        gthree_mesh_standard_material_get_emissive_color      (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_emissive_color      (GthreeMeshStandardMaterial *standard,
                                                                              const GdkRGBA              *color);
GthreeTexture *        gthree_mesh_standard_material_get_emissive_map        (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_emissive_map        (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
float                  gthree_mesh_standard_material_get_emissive_intensity  (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_emissive_intensity  (GthreeMeshStandardMaterial *standard,
                                                                             float                       value);
void                   gthree_mesh_standard_material_set_map                 (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GthreeTexture *        gthree_mesh_standard_material_get_map                 (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_env_map             (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GthreeTexture *        gthree_mesh_standard_material_get_env_map             (GthreeMeshStandardMaterial *standard);
float                  gthree_mesh_standard_material_get_env_map_intensity   (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_env_map_intensity   (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
float                  gthree_mesh_standard_material_get_refraction_ratio    (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_refraction_ratio    (GthreeMeshStandardMaterial *standard,
                                                                              float                       ratio);
float                  gthree_mesh_standard_material_get_roughness           (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_roughness           (GthreeMeshStandardMaterial *standard,
                                                                              float                       ratio);
float                  gthree_mesh_standard_material_get_metalness           (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_metalness           (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GthreeTexture*         gthree_mesh_standard_material_get_light_map           (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_light_map           (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
float                  gthree_mesh_standard_material_get_light_map_intensity (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_light_map_intensity (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GthreeTexture*         gthree_mesh_standard_material_get_ao_map              (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_ao_map              (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
float                  gthree_mesh_standard_material_get_ao_map_intensity    (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_ao_map_intensity    (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GthreeTexture*         gthree_mesh_standard_material_get_bump_map            (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_bump_map            (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
float                  gthree_mesh_standard_material_get_bump_scale          (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_bump_scale          (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GthreeTexture*         gthree_mesh_standard_material_get_normal_map          (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_normal_map          (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GthreeNormalMapType    gthree_mesh_standard_material_get_normal_map_type     (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_normal_map_type     (GthreeMeshStandardMaterial *standard,
                                                                              GthreeNormalMapType         type);
const graphene_vec2_t *gthree_mesh_standard_material_get_normal_map_scale    (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_normal_map_scale    (GthreeMeshStandardMaterial *standard,
                                                                              graphene_vec2_t            *scale);
GthreeTexture*         gthree_mesh_standard_material_get_displacement_map    (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_displacement_map    (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
float                  gthree_mesh_standard_material_get_displacement_scale  (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_displacement_scale  (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
float                  gthree_mesh_standard_material_get_displacement_bias   (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_displacement_bias   (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GthreeTexture*         gthree_mesh_standard_material_get_roughness_map       (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_roughness_map       (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GthreeTexture*         gthree_mesh_standard_material_get_metalness_map       (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_metalness_map       (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GthreeTexture*         gthree_mesh_standard_material_get_alpha_map           (GthreeMeshStandardMaterial *standard);
void                   gthree_mesh_standard_material_set_alpha_map           (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);


G_END_DECLS

#endif /* __GTHREE_STANDARDMATERIAL_H__ */
