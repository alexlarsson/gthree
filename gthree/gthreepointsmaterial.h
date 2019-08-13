#ifndef __GTHREE_POINTS_MATERIAL_H__
#define __GTHREE_POINTS_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_POINTS_MATERIAL      (gthree_points_material_get_type ())
#define GTHREE_POINTS_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_POINTS_MATERIAL, \
                                                                     GthreePointsMaterial))
#define GTHREE_IS_POINTS_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_POINTS_MATERIAL))

struct _GthreePointsMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;
} GthreePointsMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePointsMaterial, g_object_unref)

GTHREE_API
GType gthree_points_material_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreePointsMaterial *gthree_points_material_new (void);

GTHREE_API
const graphene_vec3_t *gthree_points_material_get_color       (GthreePointsMaterial *points_material);
GTHREE_API
void              gthree_points_material_set_color            (GthreePointsMaterial *points_material,
                                                               const graphene_vec3_t   *color);
GTHREE_API
void              gthree_points_material_set_map              (GthreePointsMaterial *points_material,
                                                               GthreeTexture           *texture);
GTHREE_API
GthreeTexture  *  gthree_points_material_get_map              (GthreePointsMaterial *points_material);

GTHREE_API
void              gthree_points_material_set_size              (GthreePointsMaterial *points_material,
                                                                float                 size);
GTHREE_API
float             gthree_points_material_get_size              (GthreePointsMaterial *points_material);

GTHREE_API
gboolean          gthree_points_material_get_size_attenuation (GthreePointsMaterial *points_material);
GTHREE_API
void              gthree_points_material_set_size_attenuation (GthreePointsMaterial *points_material,
                                                               gboolean size_attenuation);


G_END_DECLS

#endif /* __GTHREE_POINTSMATERIAL_H__ */
