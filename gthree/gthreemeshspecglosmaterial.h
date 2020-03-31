#ifndef __GTHREE_SPECGLOS_MATERIAL_H__
#define __GTHREE_SPECGLOS_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshbasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MESH_SPECGLOS_MATERIAL      (gthree_mesh_specglos_material_get_type ())
#define GTHREE_MESH_SPECGLOS_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                             GTHREE_TYPE_MESH_SPECGLOS_MATERIAL, \
                                                                             GthreeMeshSpecglosMaterial))
#define GTHREE_IS_MESH_SPECGLOS_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                             GTHREE_TYPE_MESH_SPECGLOS_MATERIAL))

struct _GthreeMeshSpecglosMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshSpecglosMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshSpecglosMaterial, g_object_unref)

GTHREE_API
GthreeMeshSpecglosMaterial *gthree_mesh_specglos_material_new ();
GTHREE_API
GType gthree_mesh_specglos_material_get_type (void) G_GNUC_CONST;

GTHREE_API
const graphene_vec3_t *gthree_mesh_specglos_material_get_color               (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_color               (GthreeMeshSpecglosMaterial *specglos,
                                                                              const graphene_vec3_t      *color);
GTHREE_API
const graphene_vec3_t *gthree_mesh_specglos_material_get_emissive_color      (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_emissive_color      (GthreeMeshSpecglosMaterial *specglos,
                                                                              const graphene_vec3_t      *color);
GTHREE_API
GthreeTexture *        gthree_mesh_specglos_material_get_emissive_map        (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_emissive_map        (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_specglos_material_get_emissive_intensity  (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_emissive_intensity  (GthreeMeshSpecglosMaterial *specglos,
                                                                             float                       value);
GTHREE_API
void                   gthree_mesh_specglos_material_set_map                 (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_specglos_material_get_map                 (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_env_map             (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_specglos_material_get_env_map             (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
float                  gthree_mesh_specglos_material_get_env_map_intensity   (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_env_map_intensity   (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       value);
GTHREE_API
float                  gthree_mesh_specglos_material_get_refraction_ratio    (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_refraction_ratio    (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       ratio);
GTHREE_API
float                  gthree_mesh_specglos_material_get_glossiness           (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_glossiness           (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       ratio);
GTHREE_API
const graphene_vec3_t *gthree_mesh_specglos_material_get_specular_factor     (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_specular_factor     (GthreeMeshSpecglosMaterial *specglos,
                                                                              const graphene_vec3_t      *factor);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_light_map           (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_light_map           (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_specglos_material_get_light_map_intensity (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_light_map_intensity (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_ao_map              (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_ao_map              (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_specglos_material_get_ao_map_intensity    (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_ao_map_intensity    (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_bump_map            (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_bump_map            (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_specglos_material_get_bump_scale          (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_bump_scale          (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_normal_map          (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_normal_map          (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeNormalMapType    gthree_mesh_specglos_material_get_normal_map_type     (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_normal_map_type     (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeNormalMapType         type);
GTHREE_API
const graphene_vec2_t *gthree_mesh_specglos_material_get_normal_map_scale    (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_normal_map_scale    (GthreeMeshSpecglosMaterial *specglos,
                                                                              graphene_vec2_t            *scale);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_displacement_map    (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_displacement_map    (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
float                  gthree_mesh_specglos_material_get_displacement_scale  (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_displacement_scale  (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       value);
GTHREE_API
float                  gthree_mesh_specglos_material_get_displacement_bias   (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_displacement_bias   (GthreeMeshSpecglosMaterial *specglos,
                                                                              float                       value);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_glossiness_map       (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_glossiness_map       (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_specular_map       (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_specular_map       (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);
GTHREE_API
GthreeTexture*         gthree_mesh_specglos_material_get_alpha_map           (GthreeMeshSpecglosMaterial *specglos);
GTHREE_API
void                   gthree_mesh_specglos_material_set_alpha_map           (GthreeMeshSpecglosMaterial *specglos,
                                                                              GthreeTexture              *texture);


G_END_DECLS

#endif /* __GTHREE_SPECGLOSMATERIAL_H__ */
