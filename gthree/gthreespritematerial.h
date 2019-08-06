#ifndef __GTHREE_SPRITE_MATERIAL_H__
#define __GTHREE_SPRITE_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_SPRITE_MATERIAL      (gthree_sprite_material_get_type ())
#define GTHREE_SPRITE_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_SPRITE_MATERIAL, \
                                                                     GthreeSpriteMaterial))
#define GTHREE_IS_SPRITE_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_SPRITE_MATERIAL))

struct _GthreeSpriteMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;
} GthreeSpriteMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSpriteMaterial, g_object_unref)

GTHREE_API
GType gthree_sprite_material_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeSpriteMaterial *gthree_sprite_material_new (void);

GTHREE_API
const GdkRGBA *   gthree_sprite_material_get_color            (GthreeSpriteMaterial *sprite_material);
GTHREE_API
void              gthree_sprite_material_set_color            (GthreeSpriteMaterial *sprite_material,
                                                               const GdkRGBA           *color);
GTHREE_API
void              gthree_sprite_material_set_map              (GthreeSpriteMaterial *sprite_material,
                                                               GthreeTexture           *texture);
GTHREE_API
GthreeTexture  *  gthree_sprite_material_get_map              (GthreeSpriteMaterial *sprite_material);

GTHREE_API
float             gthree_sprite_material_get_rotation         (GthreeSpriteMaterial *sprite_material);
GTHREE_API
void              gthree_sprite_material_set_rotation         (GthreeSpriteMaterial *sprite_material,
                                                               float                    rotation);
GTHREE_API
gboolean          gthree_sprite_material_get_size_attenuation (GthreeSpriteMaterial *sprite_material);
GTHREE_API
void              gthree_sprite_material_set_size_attenuation (GthreeSpriteMaterial *sprite_material,
                                                               gboolean size_attenuation);


G_END_DECLS

#endif /* __GTHREE_SPRITEMATERIAL_H__ */
