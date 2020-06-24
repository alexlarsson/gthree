#ifndef __GTHREE_FOG_H__
#define __GTHREE_FOG_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <graphene.h>
#include <glib-object.h>
#include <gthree/gthreetypes.h>

G_BEGIN_DECLS

typedef enum {
  GTHREE_FOG_STYLE_LINEAR,
  GTHREE_FOG_STYLE_EXP2,
} GthreeFogStyle;

#define GTHREE_TYPE_FOG      (gthree_fog_get_type ())
#define GTHREE_FOG(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                          GTHREE_TYPE_FOG, \
                                                          GthreeFog))
#define GTHREE_IS_FOG(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),       \
                                                          GTHREE_TYPE_FOG))

struct _GthreeFog {
  GObject parent;
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeFog, g_object_unref)

typedef struct {
  GObjectClass parent_class;
} GthreeFogClass;

GTHREE_API
GType gthree_fog_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeFog *gthree_fog_new_linear (const graphene_vec3_t *color,
                                  float near,
                                  float far);
GTHREE_API
GthreeFog *gthree_fog_new_exp2 (const graphene_vec3_t *color,
                                float density);

GTHREE_API
GthreeFogStyle         gthree_fog_get_style   (GthreeFog             *fog);
GTHREE_API
void                   gthree_fog_set_style   (GthreeFog             *fog,
                                               GthreeFogStyle         style);
GTHREE_API
const graphene_vec3_t *gthree_fog_get_color   (GthreeFog             *fog);
GTHREE_API
void                   gthree_fog_set_color   (GthreeFog             *fog,
                                               const graphene_vec3_t *color);
GTHREE_API
float                  gthree_fog_get_near    (GthreeFog             *fog);
GTHREE_API
void                   gthree_fog_set_near    (GthreeFog             *fog,
                                               float                  near);
GTHREE_API
float                  gthree_fog_get_far     (GthreeFog             *fog);
GTHREE_API
void                   gthree_fog_set_far     (GthreeFog             *fog,
                                               float                  far);
GTHREE_API
float                  gthree_fog_get_density (GthreeFog             *fog);
GTHREE_API
void                   gthree_fog_set_density (GthreeFog             *fog,
                                               float                  density);


G_END_DECLS

#endif /* __GTHREE_FOG_H__ */
