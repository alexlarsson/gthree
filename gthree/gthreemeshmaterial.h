#ifndef __GTHREE_MESH_MATERIAL_H__
#define __GTHREE_MESH_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreematerial.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_MESH_MATERIAL      (gthree_mesh_material_get_type ())
#define GTHREE_MESH_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_MESH_MATERIAL, \
                                                                     GthreeMeshMaterial))
#define GTHREE_IS_MESH_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_MESH_MATERIAL))

struct _GthreeMeshMaterial {
  GthreeMaterial parent;
};

typedef struct {
  GthreeMaterialClass parent_class;
} GthreeMeshMaterialClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeMeshMaterial, g_object_unref)

GType gthree_mesh_material_get_type (void) G_GNUC_CONST;

gboolean gthree_mesh_material_get_is_wireframe         (GthreeMeshMaterial *material);
void     gthree_mesh_material_set_is_wireframe         (GthreeMeshMaterial *material,
                                                        gboolean            is_wireframe);
float    gthree_mesh_material_get_wireframe_line_width (GthreeMeshMaterial *material);
void     gthree_mesh_material_set_wireframe_line_width (GthreeMeshMaterial *material,
                                                        float               line_width);
gboolean gthree_mesh_material_get_skinning             (GthreeMeshMaterial *material);
void     gthree_mesh_material_set_skinning             (GthreeMeshMaterial *material,
                                                        gboolean            value);
gboolean gthree_mesh_material_get_morph_targets        (GthreeMeshMaterial *material);
void     gthree_mesh_material_set_morph_targets        (GthreeMeshMaterial *material,
                                                        gboolean            value);
gboolean gthree_mesh_material_get_morph_normals        (GthreeMeshMaterial *material);
void     gthree_mesh_material_set_morph_normals        (GthreeMeshMaterial *material,
                                                        gboolean            value);


G_END_DECLS

#endif /* __GTHREE_MESHMATERIAL_H__ */
