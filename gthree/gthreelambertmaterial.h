#ifndef __GTHREE_LAMBERT_MATERIAL_H__
#define __GTHREE_LAMBERT_MATERIAL_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreebasicmaterial.h>
#include <gthree/gthreetexture.h>

G_BEGIN_DECLS


#define GTHREE_TYPE_LAMBERT_MATERIAL      (gthree_lambert_material_get_type ())
#define GTHREE_LAMBERT_MATERIAL(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                                     GTHREE_TYPE_LAMBERT_MATERIAL, \
                                                                     GthreeLambertMaterial))
#define GTHREE_IS_LAMBERT_MATERIAL(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                                                     GTHREE_TYPE_LAMBERT_MATERIAL))

struct _GthreeLambertMaterial {
  GthreeBasicMaterial parent;
};

typedef struct {
  GthreeBasicMaterialClass parent_class;

} GthreeLambertMaterialClass;

GthreeLambertMaterial *gthree_lambert_material_new ();
GType gthree_lambert_material_get_type (void) G_GNUC_CONST;

const GdkRGBA * gthree_lambert_material_get_emissive_color(GthreeLambertMaterial *lambert);
void            gthree_lambert_material_set_emissive_color(GthreeLambertMaterial *lambert,
                                                           const GdkRGBA       *color);

G_END_DECLS

#endif /* __GTHREE_LAMBERTMATERIAL_H__ */
