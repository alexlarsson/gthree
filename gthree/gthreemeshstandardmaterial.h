#ifndef __GTHREE_STANDARD_MATERIAL_H__
#define __GTHREE_STANDARD_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshbasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MESH_STANDARD_MATERIAL      (gthree_mesh_standard_material_get_type ())
#define GTHREE_MESH_STANDARD_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                             GTHREE_TYPE_MESH_STANDARD_MATERIAL, \
                                                                             GthreeMeshStandardMaterial))
#define GTHREE_IS_MESH_STANDARD_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                             GTHREE_TYPE_MESH_STANDARD_MATERIAL))

struct _GthreeMeshStandardMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshStandardMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshStandardMaterial, g_object_unref)

GTHREE_API
GthreeMeshStandardMaterial *gthree_mesh_standard_material_new ();
GTHREE_API
GType gthree_mesh_standard_material_get_type (void) G_GNUC_CONST;

GTHREE_API
const GdkRGBA *        gthree_mesh_standard_material_get_color               (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_color               (GthreeMeshStandardMaterial *standard,
                                                                              const GdkRGBA              *color);
GTHREE_API
const GdkRGBA *        gthree_mesh_standard_material_get_emissive_color      (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_emissive_color      (GthreeMeshStandardMaterial *standard,
                                                                              const GdkRGBA              *color);
GTHREE_API
GthreeTexture *        gthree_mesh_standard_material_get_emissive_map        (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_emissive_map        (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_standard_material_get_emissive_intensity  (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_emissive_intensity  (GthreeMeshStandardMaterial *standard,
                                                                             float                       value);
GTHREE_API
void                   gthree_mesh_standard_material_set_map                 (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_standard_material_get_map                 (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_env_map             (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_standard_material_get_env_map             (GthreeMeshStandardMaterial *standard);
GTHREE_API
float                  gthree_mesh_standard_material_get_env_map_intensity   (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_env_map_intensity   (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
float                  gthree_mesh_standard_material_get_refraction_ratio    (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_refraction_ratio    (GthreeMeshStandardMaterial *standard,
                                                                              float                       ratio);
GTHREE_API
float                  gthree_mesh_standard_material_get_roughness           (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_roughness           (GthreeMeshStandardMaterial *standard,
                                                                              float                       ratio);
GTHREE_API
float                  gthree_mesh_standard_material_get_metalness           (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_metalness           (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_light_map           (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_light_map           (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_standard_material_get_light_map_intensity (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_light_map_intensity (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_ao_map              (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_ao_map              (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_standard_material_get_ao_map_intensity    (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_ao_map_intensity    (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_bump_map            (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_bump_map            (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_standard_material_get_bump_scale          (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_bump_scale          (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_normal_map          (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_normal_map          (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeNormalMapType    gthree_mesh_standard_material_get_normal_map_type     (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_normal_map_type     (GthreeMeshStandardMaterial *standard,
                                                                              GthreeNormalMapType         type);
GTHREE_API
const graphene_vec2_t *gthree_mesh_standard_material_get_normal_map_scale    (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_normal_map_scale    (GthreeMeshStandardMaterial *standard,
                                                                              graphene_vec2_t            *scale);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_displacement_map    (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_displacement_map    (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_standard_material_get_displacement_scale  (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_displacement_scale  (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
float                  gthree_mesh_standard_material_get_displacement_bias   (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_displacement_bias   (GthreeMeshStandardMaterial *standard,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_roughness_map       (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_roughness_map       (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_metalness_map       (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_metalness_map       (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture*         gthree_mesh_standard_material_get_alpha_map           (GthreeMeshStandardMaterial *standard);
GTHREE_API
void                   gthree_mesh_standard_material_set_alpha_map           (GthreeMeshStandardMaterial *standard,
                                                                              GthreeTexture              *texture);


G_END_DECLS

#endif /* __GTHREE_STANDARDMATERIAL_H__ */
