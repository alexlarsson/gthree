#ifndef __GTHREE_NORMAL_MATERIAL_H__
#define __GTHREE_NORMAL_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshmaterial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_NORMAL_MATERIAL      (gthree_mesh_normal_material_get_type ())
#define GTHREE_NORMAL_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_NORMAL_MATERIAL, \
                                                                     GthreeMeshNormalMaterial))
#define GTHREE_IS_NORMAL_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_NORMAL_MATERIAL))

struct _GthreeMeshNormalMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshNormalMaterialClass;

GTHREE_API
GType gthree_mesh_normal_material_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeMeshNormalMaterial *gthree_mesh_normal_material_new ();

GTHREE_API
GthreeShadingType gthree_mesh_normal_material_get_shading_type (GthreeMeshNormalMaterial *normal);
GTHREE_API
void              gthree_mesh_normal_material_set_shading_type (GthreeMeshNormalMaterial *normal,
                                                                GthreeShadingType         shading_type);

G_END_DECLS

#endif /* __GTHREE_NORMALMATERIAL_H__ */
