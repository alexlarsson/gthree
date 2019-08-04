#ifndef __GTHREE_DEPTH_MATERIAL_H__
#define __GTHREE_DEPTH_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreemeshmaterial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MESH_DEPTH_MATERIAL      (gthree_mesh_depth_material_get_type ())
#define GTHREE_MESH_DEPTH_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_MESH_DEPTH_MATERIAL, \
                                                                     GthreeMeshDepthMaterial))
#define GTHREE_IS_MESH_DEPTH_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_MESH_DEPTH_MATERIAL))

struct _GthreeMeshDepthMaterial {
  GthreeMeshMaterial parent;
};

typedef struct {
  GthreeMeshMaterialClass parent_class;

} GthreeMeshDepthMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshDepthMaterial, g_object_unref)

GTHREE_API
GthreeMeshDepthMaterial *gthree_mesh_depth_material_new ();
GTHREE_API
GType gthree_mesh_depth_material_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeDepthPackingFormat gthree_mesh_depth_material_get_depth_packing_format (GthreeMeshDepthMaterial  *depth);
GTHREE_API
void                     gthree_mesh_depth_material_set_depth_packing_format (GthreeMeshDepthMaterial  *depth,
                                                                              GthreeDepthPackingFormat  format);

G_END_DECLS

#endif /* __GTHREE_DEPTHMATERIAL_H__ */
