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

GthreeMeshLambertMaterial *gthree_mesh_lambert_material_new ();
GType gthree_mesh_lambert_material_get_type (void) G_GNUC_CONST;

const GdkRGBA * gthree_mesh_lambert_material_get_emissive_color   (GthreeMeshLambertMaterial *lambert);
void            gthree_mesh_lambert_material_set_emissive_color   (GthreeMeshLambertMaterial *lambert,
                                                                   const GdkRGBA             *color);
const GdkRGBA * gthree_mesh_lambert_material_get_color            (GthreeMeshLambertMaterial *lambert);
void            gthree_mesh_lambert_material_set_color            (GthreeMeshLambertMaterial *lambert,
                                                                   const GdkRGBA             *color);
float           gthree_mesh_lambert_material_get_refraction_ratio (GthreeMeshLambertMaterial *lambert);
void            gthree_mesh_lambert_material_set_refraction_ratio (GthreeMeshLambertMaterial *lambert,
                                                                   float                      ratio);
float           gthree_mesh_lambert_material_get_reflectivity     (GthreeMeshLambertMaterial *lambert);
void            gthree_mesh_lambert_material_set_reflectivity     (GthreeMeshLambertMaterial *lambert,
                                                                   float                      reflectivity);
void            gthree_mesh_lambert_material_set_map              (GthreeMeshLambertMaterial *lambert,
                                                                   GthreeTexture             *texture);
GthreeTexture * gthree_mesh_lambert_material_get_map              (GthreeMeshLambertMaterial *lambert);
void            gthree_mesh_lambert_material_set_env_map          (GthreeMeshLambertMaterial *lambert,
                                                                   GthreeTexture             *texture);
GthreeTexture * gthree_mesh_lambert_material_get_env_map          (GthreeMeshLambertMaterial *lambert);
void            gthree_mesh_lambert_material_set_combine          (GthreeMeshLambertMaterial *lambert,
                                                                   GthreeOperation            combine);
GthreeOperation gthree_mesh_lambert_material_get_combine          (GthreeMeshLambertMaterial *lambert);

G_END_DECLS

#endif /* __GTHREE_LAMBERTMATERIAL_H__ */
