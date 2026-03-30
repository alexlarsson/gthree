#ifndef __GTHREE_MESH_PHYSICAL_MATERIAL_H__
#define __GTHREE_MESH_PHYSICAL_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshstandardmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MESH_PHYSICAL_MATERIAL      (gthree_mesh_physical_material_get_type ())
#define GTHREE_MESH_PHYSICAL_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                             GTHREE_TYPE_MESH_PHYSICAL_MATERIAL, \
                                                                             GthreeMeshPhysicalMaterial))
#define GTHREE_IS_MESH_PHYSICAL_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                             GTHREE_TYPE_MESH_PHYSICAL_MATERIAL))

struct _GthreeMeshPhysicalMaterial {
  GthreeMeshStandardMaterial parent;
};

typedef struct {
  GthreeMeshStandardMaterialClass parent_class;

} GthreeMeshPhysicalMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshPhysicalMaterial, g_object_unref)

GTHREE_API
GthreeMeshPhysicalMaterial *gthree_mesh_physical_material_new ();
GTHREE_API
GType gthree_mesh_physical_material_get_type (void) G_GNUC_CONST;

GTHREE_API
float                  gthree_mesh_physical_material_get_clearcoat                 (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_clearcoat                 (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_clearcoat_map             (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_clearcoat_map             (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_clearcoat_roughness       (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_clearcoat_roughness       (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_clearcoat_roughness_map   (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_clearcoat_roughness_map   (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_clearcoat_normal_map      (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_clearcoat_normal_map      (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
const graphene_vec2_t *gthree_mesh_physical_material_get_clearcoat_normal_scale    (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_clearcoat_normal_scale    (GthreeMeshPhysicalMaterial *physical,
                                                                                    graphene_vec2_t            *scale);
GTHREE_API
float                  gthree_mesh_physical_material_get_ior                       (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_ior                       (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
float                  gthree_mesh_physical_material_get_iridescence               (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_iridescence               (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_iridescence_map           (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_iridescence_map           (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_iridescence_ior           (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_iridescence_ior           (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
float                  gthree_mesh_physical_material_get_iridescence_thickness_min (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_iridescence_thickness_min (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
float                  gthree_mesh_physical_material_get_iridescence_thickness_max (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_iridescence_thickness_max (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_iridescence_thickness_map (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_iridescence_thickness_map (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_sheen                     (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_sheen                     (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
const graphene_vec3_t *gthree_mesh_physical_material_get_sheen_color               (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_sheen_color               (GthreeMeshPhysicalMaterial *physical,
                                                                                    const graphene_vec3_t      *color);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_sheen_color_map           (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_sheen_color_map           (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_sheen_roughness           (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_sheen_roughness           (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_sheen_roughness_map       (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_sheen_roughness_map       (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_transmission              (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_transmission              (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_transmission_map          (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_transmission_map          (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_thickness                 (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_thickness                 (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_thickness_map             (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_thickness_map             (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_attenuation_distance      (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_attenuation_distance      (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
const graphene_vec3_t *gthree_mesh_physical_material_get_attenuation_color         (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_attenuation_color         (GthreeMeshPhysicalMaterial *physical,
                                                                                    const graphene_vec3_t      *color);
GTHREE_API
float                  gthree_mesh_physical_material_get_dispersion                (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_dispersion                (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
float                  gthree_mesh_physical_material_get_specular_intensity        (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_specular_intensity        (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_specular_intensity_map    (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_specular_intensity_map    (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
const graphene_vec3_t *gthree_mesh_physical_material_get_specular_color            (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_specular_color            (GthreeMeshPhysicalMaterial *physical,
                                                                                    const graphene_vec3_t      *color);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_specular_color_map        (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_specular_color_map        (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_physical_material_get_anisotropy                (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_anisotropy                (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
float                  gthree_mesh_physical_material_get_anisotropy_rotation       (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_anisotropy_rotation       (GthreeMeshPhysicalMaterial *physical,
                                                                                    float                       value);
GTHREE_API
GthreeTexture *        gthree_mesh_physical_material_get_anisotropy_map            (GthreeMeshPhysicalMaterial *physical);
GTHREE_API
void                   gthree_mesh_physical_material_set_anisotropy_map            (GthreeMeshPhysicalMaterial *physical,
                                                                                    GthreeTexture              *texture);

G_END_DECLS

#endif /* __GTHREE_MESH_PHYSICAL_MATERIAL_H__ */
