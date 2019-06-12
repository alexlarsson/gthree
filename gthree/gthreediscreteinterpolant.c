#include "gthreediscreteinterpolant.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeDiscreteInterpolant, gthree_discrete_interpolant, GTHREE_TYPE_INTERPOLANT)

GthreeInterpolant *
gthree_discrete_interpolant_new (GthreeAttributeArray *parameter_positions,
                                 GthreeAttributeArray *sample_values)
{
  GthreeInterpolant *interpolant;

  interpolant = gthree_interpolant_create (GTHREE_TYPE_DISCRETE_INTERPOLANT, parameter_positions, sample_values);

  return interpolant;
}

static void
gthree_discrete_interpolant_init (GthreeDiscreteInterpolant *interpolant)
{
}

static void
gthree_discrete_interpolant_interpolate (GthreeInterpolant *interpolant, int i1, float t0, float t, float t1, GthreeAttributeArray *dest)
{
  gthree_interpolant_copy_sample_value (interpolant, i1 - 1);
}

static void
gthree_discrete_interpolant_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_discrete_interpolant_parent_class)->finalize (obj);
}

static void
gthree_discrete_interpolant_class_init (GthreeDiscreteInterpolantClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_discrete_interpolant_finalize;
  GTHREE_INTERPOLANT_CLASS (klass)->interpolate = gthree_discrete_interpolant_interpolate;
}
