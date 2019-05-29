#ifndef __GTHREE_DEPTH_MATERIAL_H__
#define __GTHREE_DEPTH_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshmaterial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_DEPTH_MATERIAL      (gthree_depth_material_get_type ())
#define GTHREE_DEPTH_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_DEPTH_MATERIAL, \
                                                                     GthreeDepthMaterial))
#define GTHREE_IS_DEPTH_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_DEPTH_MATERIAL))

struct _GthreeDepthMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeDepthMaterialClass;

GthreeDepthMaterial *gthree_depth_material_new ();
GType gthree_depth_material_get_type (void) G_GNUC_CONST;

GthreeShadingType gthree_depth_material_get_shading_type  (GthreeDepthMaterial *depth);
void              gthree_depth_material_set_shading_type  (GthreeDepthMaterial *depth,
                                                           GthreeShadingType    shading_type);


G_END_DECLS

#endif /* __GTHREE_DEPTHMATERIAL_H__ */
