#include "gthreelinearinterpolant.h"
#include "gthreeprivate.h"

typedef struct {
  float *floats;
} GthreeLinearInterpolantPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLinearInterpolant, gthree_linear_interpolant, GTHREE_TYPE_INTERPOLANT)

GthreeInterpolant *
gthree_linear_interpolant_new (GthreeAttributeArray *parameter_positions,
                               GthreeAttributeArray *sample_values)
{
  GthreeInterpolant *interpolant;
  GthreeLinearInterpolantPrivate *priv;
  int sample_size;

  interpolant = gthree_interpolant_create (GTHREE_TYPE_LINEAR_INTERPOLANT, parameter_positions, sample_values);
  priv = gthree_linear_interpolant_get_instance_private (GTHREE_LINEAR_INTERPOLANT (interpolant));

  sample_size = gthree_interpolant_get_sample_size (interpolant);

  priv->floats = g_new (float, 3*sample_size);

  return interpolant;
}

static void
gthree_linear_interpolant_init (GthreeLinearInterpolant *interpolant)
{
}

static void
gthree_linear_interpolant_interval_changed (GthreeInterpolant *interpolant, int i1, float t0, float t1)
{
  GthreeLinearInterpolant *linear = GTHREE_LINEAR_INTERPOLANT (interpolant);
  GthreeLinearInterpolantPrivate *priv = gthree_linear_interpolant_get_instance_private (linear);
  GthreeAttributeArray *sample_values;
  int sample_size;

  sample_size = gthree_interpolant_get_sample_size (interpolant);
  sample_values = gthree_interpolant_get_sample_values (interpolant);

  gthree_attribute_array_get_elements_as_float (sample_values, i1 - 1, 0,
                                                priv->floats + 0, sample_size);
  gthree_attribute_array_get_elements_as_float (sample_values, i1, 0,
                                                priv->floats + sample_size, sample_size);
}

static void
gthree_linear_interpolant_interpolate (GthreeInterpolant *interpolant, int i1, float t0, float t, float t1, GthreeAttributeArray *dest)
{
  GthreeLinearInterpolant *linear = GTHREE_LINEAR_INTERPOLANT (interpolant);
  GthreeLinearInterpolantPrivate *priv = gthree_linear_interpolant_get_instance_private (linear);

  float weight1 = ( t - t0 ) / ( t1 - t0 );
  float weight0 = 1 - weight1;
  int sample_size, i;
  float *v0, *v1, *d;

  sample_size = gthree_interpolant_get_sample_size (interpolant);

  v0 = priv->floats + 0;
  v1 = priv->floats + sample_size;
  d = priv->floats + 2 *sample_size;

  for (i = 0; i < sample_size; i++)
    d[i] = v0[i] * weight0 + v1[i] * weight1;

  gthree_attribute_array_set_elements_from_float (dest, 0, 0, d, sample_size);
}

static void
gthree_linear_interpolant_finalize (GObject *obj)
{
  GthreeLinearInterpolant *interpolant = GTHREE_LINEAR_INTERPOLANT (obj);
  GthreeLinearInterpolantPrivate *priv = gthree_linear_interpolant_get_instance_private (interpolant);

  g_free (priv->floats);

  G_OBJECT_CLASS (gthree_linear_interpolant_parent_class)->finalize (obj);
}

static void
gthree_linear_interpolant_class_init (GthreeLinearInterpolantClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_linear_interpolant_finalize;
  GTHREE_INTERPOLANT_CLASS (klass)->interval_changed = gthree_linear_interpolant_interval_changed;
  GTHREE_INTERPOLANT_CLASS (klass)->interpolate = gthree_linear_interpolant_interpolate;
}
