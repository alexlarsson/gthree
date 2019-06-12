#ifndef __GTHREE_QUATERNION_INTERPOLANT_H__
#define __GTHREE_QUATERNION_INTERPOLANT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeinterpolant.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_QUATERNION_INTERPOLANT      (gthree_interpolant_get_type ())
#define GTHREE_QUATERNION_INTERPOLANT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                 GTHREE_TYPE_QUATERNION_INTERPOLANT, \
                                                 GthreeQuaternionInterpolant))
#define GTHREE_IS_QUATERNION_INTERPOLANT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                 GTHREE_TYPE_QUATERNION_INTERPOLANT))
#define GTHREE_QUATERNION_INTERPOLANT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_QUATERNION_INTERPOLANT, GthreeQuaternionInterpolantClass))


typedef struct {
  GthreeInterpolant parent;
} GthreeQuaternionInterpolant;

typedef struct {
  GthreeInterpolantClass parent_class;
} GthreeQuaternionInterpolantClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeQuaternionInterpolant, g_object_unref)

GType gthree_quaternion_interpolant_get_type (void) G_GNUC_CONST;

GthreeInterpolant * gthree_quaternion_interpolant_new (GthreeAttributeArray *parameter_positions,
                                                       GthreeAttributeArray *sample_values);

G_END_DECLS

#endif /* __GTHREE_QUATERNION_INTERPOLANT_H__ */
