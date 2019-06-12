#ifndef __GTHREE_DISCRETE_INTERPOLANT_H__
#define __GTHREE_DISCRETE_INTERPOLANT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreeinterpolant.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_DISCRETE_INTERPOLANT      (gthree_interpolant_get_type ())
#define GTHREE_DISCRETE_INTERPOLANT(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                 GTHREE_TYPE_DISCRETE_INTERPOLANT, \
                                                 GthreeDiscreteInterpolant))
#define GTHREE_IS_DISCRETE_INTERPOLANT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                 GTHREE_TYPE_DISCRETE_INTERPOLANT))
#define GTHREE_DISCRETE_INTERPOLANT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_DISCRETE_INTERPOLANT, GthreeDiscreteInterpolantClass))


typedef struct {
  GthreeInterpolant parent;
} GthreeDiscreteInterpolant;

typedef struct {
  GthreeInterpolantClass parent_class;
} GthreeDiscreteInterpolantClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeDiscreteInterpolant, g_object_unref)

GType gthree_discrete_interpolant_get_type (void) G_GNUC_CONST;

GthreeInterpolant * gthree_discrete_interpolant_new (GthreeAttributeArray *parameter_positions,
                                                     GthreeAttributeArray *sample_values);

G_END_DECLS

#endif /* __GTHREE_DISCRETE_INTERPOLANT_H__ */
