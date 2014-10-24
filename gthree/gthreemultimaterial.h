#ifndef __GTHREE_MULTI_MATERIAL_H__
#define __GTHREE_MULTI_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MULTI_MATERIAL      (gthree_multi_material_get_type ())
#define GTHREE_MULTI_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_MULTI_MATERIAL, \
                                                                     GthreeMultiMaterial))
#define GTHREE_IS_MULTI_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_MULTI_MATERIAL))

struct _GthreeMultiMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;

} GthreeMultiMaterialClass;

GthreeMultiMaterial *gthree_multi_material_new ();
GType gthree_multi_material_get_type (void) G_GNUC_CONST;

void gthree_multi_material_set_index (GthreeMultiMaterial *multi_material,
                                      int index,
                                      GthreeMaterial *material);

G_END_DECLS

#endif /* __GTHREE_MULTIMATERIAL_H__ */
