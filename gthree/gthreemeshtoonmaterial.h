#ifndef __GTHREE_MESH_TOON_MATERIAL_H__
#define __GTHREE_MESH_TOON_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshbasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MESH_TOON_MATERIAL      (gthree_mesh_toon_material_get_type ())
#define GTHREE_MESH_TOON_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                          GTHREE_TYPE_MESH_TOON_MATERIAL, \
                                                                          GthreeMeshToonMaterial))
#define GTHREE_IS_MESH_TOON_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                          GTHREE_TYPE_MESH_TOON_MATERIAL))

struct _GthreeMeshToonMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshToonMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshToonMaterial, g_object_unref)

GTHREE_API
GthreeMeshToonMaterial *gthree_mesh_toon_material_new ();
GTHREE_API
GType gthree_mesh_toon_material_get_type (void) G_GNUC_CONST;

GTHREE_API
const graphene_vec3_t *gthree_mesh_toon_material_get_color               (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_color               (GthreeMeshToonMaterial *toon,
                                                                          const graphene_vec3_t  *color);
GTHREE_API
void                   gthree_mesh_toon_material_set_map                 (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_toon_material_get_map                 (GthreeMeshToonMaterial *toon);
GTHREE_API
GthreeTexture *        gthree_mesh_toon_material_get_gradient_map        (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_gradient_map        (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
GthreeTexture *        gthree_mesh_toon_material_get_light_map           (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_light_map           (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
float                  gthree_mesh_toon_material_get_light_map_intensity (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_light_map_intensity (GthreeMeshToonMaterial *toon,
                                                                          float                   value);
GTHREE_API
GthreeTexture*         gthree_mesh_toon_material_get_ao_map              (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_ao_map              (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
float                  gthree_mesh_toon_material_get_ao_map_intensity    (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_ao_map_intensity    (GthreeMeshToonMaterial *toon,
                                                                          float                   value);
GTHREE_API
const graphene_vec3_t *gthree_mesh_toon_material_get_emissive_color      (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_emissive_color      (GthreeMeshToonMaterial *toon,
                                                                          const graphene_vec3_t  *color);
GTHREE_API
GthreeTexture *        gthree_mesh_toon_material_get_emissive_map        (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_emissive_map        (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
float                  gthree_mesh_toon_material_get_emissive_intensity  (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_emissive_intensity  (GthreeMeshToonMaterial *toon,
                                                                          float                   value);
GTHREE_API
GthreeTexture*         gthree_mesh_toon_material_get_bump_map            (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_bump_map            (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
float                  gthree_mesh_toon_material_get_bump_scale          (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_bump_scale          (GthreeMeshToonMaterial *toon,
                                                                          float                   value);
GTHREE_API
GthreeTexture*         gthree_mesh_toon_material_get_normal_map          (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_normal_map          (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
GthreeNormalMapType    gthree_mesh_toon_material_get_normal_map_type     (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_normal_map_type     (GthreeMeshToonMaterial *toon,
                                                                          GthreeNormalMapType     type);
GTHREE_API
const graphene_vec2_t *gthree_mesh_toon_material_get_normal_map_scale    (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_normal_map_scale    (GthreeMeshToonMaterial *toon,
                                                                          graphene_vec2_t        *scale);
GTHREE_API
GthreeTexture*         gthree_mesh_toon_material_get_displacement_map    (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_displacement_map    (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);
GTHREE_API
float                  gthree_mesh_toon_material_get_displacement_scale  (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_displacement_scale  (GthreeMeshToonMaterial *toon,
                                                                          float                   value);
GTHREE_API
float                  gthree_mesh_toon_material_get_displacement_bias   (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_displacement_bias   (GthreeMeshToonMaterial *toon,
                                                                          float                   value);
GTHREE_API
GthreeTexture*         gthree_mesh_toon_material_get_alpha_map           (GthreeMeshToonMaterial *toon);
GTHREE_API
void                   gthree_mesh_toon_material_set_alpha_map           (GthreeMeshToonMaterial *toon,
                                                                          GthreeTexture          *texture);

G_END_DECLS

#endif /* __GTHREE_MESH_TOON_MATERIAL_H__ */
