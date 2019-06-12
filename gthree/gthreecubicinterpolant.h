#ifndef __GTHREE_CUBIC_INTERPOLANT_H__
#define __GTHREE_CUBIC_INTERPOLANT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeinterpolant.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_CUBIC_INTERPOLANT      (gthree_interpolant_get_type ())
#define GTHREE_CUBIC_INTERPOLANT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                 GTHREE_TYPE_CUBIC_INTERPOLANT, \
                                                 GthreeCubicInterpolant))
#define GTHREE_IS_CUBIC_INTERPOLANT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                 GTHREE_TYPE_CUBIC_INTERPOLANT))
#define GTHREE_CUBIC_INTERPOLANT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_CUBIC_INTERPOLANT, GthreeCubicInterpolantClass))


typedef struct {
  GthreeInterpolant parent;
} GthreeCubicInterpolant;

typedef struct {
  GthreeInterpolantClass parent_class;
} GthreeCubicInterpolantClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeCubicInterpolant, g_object_unref)

GType gthree_cubic_interpolant_get_type (void) G_GNUC_CONST;

GthreeInterpolant * gthree_cubic_interpolant_new (GthreeAttributeArray *parameter_positions,
                                                   GthreeAttributeArray *sample_values);

G_END_DECLS

#endif /* __GTHREE_CUBIC_INTERPOLANT_H__ */
