#ifndef __GTHREE_SPRITE_H__
#define __GTHREE_SPRITE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeobject.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreegeometry.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_SPRITE      (gthree_sprite_get_type ())
#define GTHREE_SPRITE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_SPRITE, \
                                                             GthreeSprite))
#define GTHREE_IS_SPRITE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_SPRITE))

typedef struct {
  GthreeObject parent;
} GthreeSprite;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeSprite, g_object_unref)

typedef struct {
  GthreeObjectClass parent_class;

} GthreeSpriteClass;

GTHREE_API
GType gthree_sprite_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeSprite *gthree_sprite_new (GthreeMaterial *material);

GTHREE_API
GthreeMaterial        *gthree_sprite_get_material (GthreeSprite          *sprite);
GTHREE_API
void                   gthree_sprite_set_material (GthreeSprite          *sprite,
                                                   GthreeMaterial        *material);
GTHREE_API
const graphene_vec2_t *gthree_sprite_get_center   (GthreeSprite          *sprite);
GTHREE_API
void                   gthree_sprite_set_center   (GthreeSprite          *sprite,
                                                   const graphene_vec2_t *center);

G_END_DECLS

#endif /* __GTHREE_SPRITE_H__ */
