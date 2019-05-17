#ifndef __GTHREE_NORMAL_MATERIAL_H__
#define __GTHREE_NORMAL_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_NORMAL_MATERIAL      (gthree_normal_material_get_type ())
#define GTHREE_NORMAL_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_NORMAL_MATERIAL, \
                                                                     GthreeNormalMaterial))
#define GTHREE_IS_NORMAL_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_NORMAL_MATERIAL))

struct _GthreeNormalMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;

} GthreeNormalMaterialClass;

GType gthree_normal_material_get_type (void) G_GNUC_CONST;

GthreeNormalMaterial *gthree_normal_material_new ();

GthreeShadingType gthree_normal_material_get_shading_type  (GthreeNormalMaterial *normal);
void              gthree_normal_material_set_shading_type  (GthreeNormalMaterial *normal,
                                                           GthreeShadingType    shading_type);

G_END_DECLS

#endif /* __GTHREE_NORMALMATERIAL_H__ */
