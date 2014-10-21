#ifndef __GTHREE_PRIVATE_H__
#define __GTHREE_PRIVATE_H__

#include "gthreeobject.h"
#include "gthreeface.h"
#include "gthreebufferprivate.h"

struct _GthreeFace
{
  GObject parent;

  int a, b, c;
  graphene_vec3_t normal;
  GdkRGBA color;
  graphene_vec3_t *vertex_normals;
  GdkRGBA *vertex_colors;
  int material_index;
};

guint gthree_renderer_allocate_texture_unit (GthreeRenderer *renderer);
void gthree_texture_load (GthreeTexture *texture, int slot);

#endif /* __GTHREE_PRIVATE_H__ */
