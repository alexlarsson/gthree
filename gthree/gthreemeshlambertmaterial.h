#ifndef __GTHREE_LAMBERT_MATERIAL_H__
#define __GTHREE_LAMBERT_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshbasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_LAMBERT_MATERIAL      (gthree_mesh_lambert_material_get_type ())
#define GTHREE_LAMBERT_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_LAMBERT_MATERIAL, \
                                                                     GthreeMeshLambertMaterial))
#define GTHREE_IS_LAMBERT_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_LAMBERT_MATERIAL))

struct _GthreeMeshLambertMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshLambertMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshLambertMaterial, g_object_unref)

GTHREE_API
GthreeMeshLambertMaterial *gthree_mesh_lambert_material_new ();
GTHREE_API
GType gthree_mesh_lambert_material_get_type (void) G_GNUC_CONST;

GTHREE_API
const graphene_vec3_t * gthree_mesh_lambert_material_get_emissive_color (GthreeMeshLambertMaterial *lambert);
GTHREE_API
void            gthree_mesh_lambert_material_set_emissive_color   (GthreeMeshLambertMaterial *lambert,
                                                                   const graphene_vec3_t             *color);
GTHREE_API
const graphene_vec3_t * gthree_mesh_lambert_material_get_color    (GthreeMeshLambertMaterial *lambert);
GTHREE_API
void            gthree_mesh_lambert_material_set_color            (GthreeMeshLambertMaterial *lambert,
                                                                   const graphene_vec3_t             *color);
GTHREE_API
float           gthree_mesh_lambert_material_get_refraction_ratio (GthreeMeshLambertMaterial *lambert);
GTHREE_API
void            gthree_mesh_lambert_material_set_refraction_ratio (GthreeMeshLambertMaterial *lambert,
                                                                   float                      ratio);
GTHREE_API
float           gthree_mesh_lambert_material_get_reflectivity     (GthreeMeshLambertMaterial *lambert);
GTHREE_API
void            gthree_mesh_lambert_material_set_reflectivity     (GthreeMeshLambertMaterial *lambert,
                                                                   float                      reflectivity);
GTHREE_API
void            gthree_mesh_lambert_material_set_map              (GthreeMeshLambertMaterial *lambert,
                                                                   GthreeTexture             *texture);
GTHREE_API
GthreeTexture * gthree_mesh_lambert_material_get_map              (GthreeMeshLambertMaterial *lambert);
GTHREE_API
void            gthree_mesh_lambert_material_set_env_map          (GthreeMeshLambertMaterial *lambert,
                                                                   GthreeTexture             *texture);
GTHREE_API
GthreeTexture * gthree_mesh_lambert_material_get_env_map          (GthreeMeshLambertMaterial *lambert);
GTHREE_API
void            gthree_mesh_lambert_material_set_combine          (GthreeMeshLambertMaterial *lambert,
                                                                   GthreeOperation            combine);
GTHREE_API
GthreeOperation gthree_mesh_lambert_material_get_combine          (GthreeMeshLambertMaterial *lambert);

G_END_DECLS

#endif /* __GTHREE_LAMBERTMATERIAL_H__ */
