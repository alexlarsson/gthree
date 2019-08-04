#ifndef __GTHREE_LINEAR_INTERPOLANT_H__
#define __GTHREE_LINEAR_INTERPOLANT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeinterpolant.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_LINEAR_INTERPOLANT      (gthree_linear_interpolant_get_type ())
#define GTHREE_LINEAR_INTERPOLANT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                 GTHREE_TYPE_LINEAR_INTERPOLANT, \
                                                 GthreeLinearInterpolant))
#define GTHREE_IS_LINEAR_INTERPOLANT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                 GTHREE_TYPE_LINEAR_INTERPOLANT))
#define GTHREE_LINEAR_INTERPOLANT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_LINEAR_INTERPOLANT, GthreeLinearInterpolantClass))


typedef struct {
  GthreeInterpolant parent;
} GthreeLinearInterpolant;

typedef struct {
  GthreeInterpolantClass parent_class;
} GthreeLinearInterpolantClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeLinearInterpolant, g_object_unref)

GTHREE_API
GType gthree_linear_interpolant_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeInterpolant * gthree_linear_interpolant_new (GthreeAttributeArray *parameter_positions,
                                                   GthreeAttributeArray *sample_values);

G_END_DECLS

#endif /* __GTHREE_LINEAR_INTERPOLANT_H__ */
