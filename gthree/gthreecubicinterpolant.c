#include "gthreecubicinterpolant.h"
#include "gthreeprivate.h"

typedef struct {
  float *floats;
  float weight_prev, weight_next;
} GthreeCubicInterpolantPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeCubicInterpolant, gthree_cubic_interpolant, GTHREE_TYPE_INTERPOLANT)

GthreeInterpolant *
gthree_cubic_interpolant_new (GthreeAttributeArray *parameter_positions,
                               GthreeAttributeArray *sample_values)
{
  GthreeInterpolant *interpolant;
  GthreeCubicInterpolantPrivate *priv;
  int sample_size;

  interpolant = gthree_interpolant_create (GTHREE_TYPE_CUBIC_INTERPOLANT, parameter_positions, sample_values);
  priv = gthree_cubic_interpolant_get_instance_private (GTHREE_CUBIC_INTERPOLANT (interpolant));

  sample_size = gthree_interpolant_get_sample_size (interpolant);

  priv->floats = g_new (float, 5 * sample_size);

  return interpolant;
}

static void
gthree_cubic_interpolant_init (GthreeCubicInterpolant *interpolant)
{
}

static void
gthree_cubic_interpolant_interval_changed (GthreeInterpolant *interpolant, int i1, float t0, float t1)
{
  GthreeCubicInterpolant *cubic = GTHREE_CUBIC_INTERPOLANT (interpolant);
  GthreeCubicInterpolantPrivate *priv = gthree_cubic_interpolant_get_instance_private (cubic);
  GthreeAttributeArray *sample_values;
  GthreeAttributeArray *parameter_positions;
  float *pp;
  int sample_size, n_positions;
  int i_prev, i_next;
  float t_prev, t_next, half_dt;

  sample_size = gthree_interpolant_get_sample_size (interpolant);
  sample_values = gthree_interpolant_get_sample_values (interpolant);
  n_positions = gthree_interpolant_get_n_positions (interpolant);
  parameter_positions = gthree_interpolant_get_parameter_positions (interpolant);
  pp = gthree_attribute_array_peek_float (parameter_positions);

  /* prev */
  if (i1 > 1)
    {
      i_prev = i1 - 2;
      t_prev = pp[i_prev];
    }
  else
    {
      switch (gthree_interpolant_get_start_ending_mode (interpolant))
        {
        case GTHREE_ENDING_MODE_ZERO_CURVATURE:
          // f'(t0) = 0
          i_prev = i1;
          t_prev = 2 * t0 - t1;
          break;
        case GTHREE_ENDING_MODE_ZERO_SLOPE:
          // use the other end of the curve
          i_prev = n_positions - 2;
          t_prev = t0 + pp[i_prev] - pp[i_prev + 1];
          break;
        default:
        case GTHREE_ENDING_MODE_WRAP_AROUND:
          // f''(t0) = 0 a.k.a. Natural Spline
          i_prev = i1;
          t_prev = t1;
          break;
        }
    }

  /* next */
  if (i1 + 1 < sample_size)
    {
      i_next = i1 + 1;
      t_next = pp[i_next];
    }
  else
    {
      switch (gthree_interpolant_get_end_ending_mode (interpolant))
        {
        case GTHREE_ENDING_MODE_ZERO_CURVATURE:
          // f'(t0) = 0
          i_next = i1;
          t_next = 2 * t1 - t0;
          break;
        case GTHREE_ENDING_MODE_ZERO_SLOPE:
          // use the other end of the curve
          i_next = 1;
          t_next = t1 + pp[1] - pp[0];
          break;
        default:
        case GTHREE_ENDING_MODE_WRAP_AROUND:
          // f''(t0) = 0 a.k.a. Natural Spline
          i_next = i1 - 1;
          t_next = t0;
          break;
        }
    }

  half_dt = (t1 - t0) * 0.5;
  priv->weight_prev = half_dt / (t0 - t_prev);
  priv->weight_next = half_dt / (t_next - t1);

  /* vP */
  gthree_attribute_array_get_elements_as_float (sample_values, i_prev, 0,
                                                priv->floats + 0 * sample_size, sample_size);

  /* v0 */
  gthree_attribute_array_get_elements_as_float (sample_values, i1 - 1, 0,
                                                priv->floats + 1 * sample_size, sample_size);
  /* v1 */
  gthree_attribute_array_get_elements_as_float (sample_values, i1, 0,
                                                priv->floats + 2 * sample_size, sample_size);
  /* vN */
  gthree_attribute_array_get_elements_as_float (sample_values, i_next, 0,
                                                priv->floats + 3 * sample_size, sample_size);
}

static void
gthree_cubic_interpolant_interpolate (GthreeInterpolant *interpolant, int i1, float t0, float t, float t1, GthreeAttributeArray *dest)
{
  GthreeCubicInterpolant *cubic = GTHREE_CUBIC_INTERPOLANT (interpolant);
  GthreeCubicInterpolantPrivate *priv = gthree_cubic_interpolant_get_instance_private (cubic);
  int i, sample_size;
  float wP, wN, p, pp, ppp, sP, s0, s1, sN;
  float *vP, *v0, *v1, *vN, *d;

  wP = priv->weight_prev;
  wN = priv->weight_next;

  p = ( t - t0 ) / ( t1 - t0 );
  pp = p * p;
  ppp = pp * p;


  // evaluate polynomials
  sP = - wP * ppp + 2 * wP * pp - wP * p;
  s0 = ( 1 + wP ) * ppp + ( - 1.5 - 2 * wP ) * pp + ( - 0.5 + wP ) * p + 1;
  s1 = ( - 1 - wN ) * ppp + ( 1.5 + wN ) * pp + 0.5 * p;
  sN = wN * ppp - wN * pp;

  sample_size = gthree_interpolant_get_sample_size (interpolant);

  vP = priv->floats + 0 * sample_size;
  v0 = priv->floats + 1 * sample_size;
  v1 = priv->floats + 2 * sample_size;
  vN = priv->floats + 3 * sample_size;
  d  = priv->floats + 4 * sample_size;

  // combine data linearly
  for (i = 0; i < sample_size; i++)
    d[i] = sP * vP[i] + s0 * v0[i] + s1 * v1[i] + sN * vN[i];

  gthree_attribute_array_set_elements_from_float (dest, 0, 0, d, sample_size);
}

static void
gthree_cubic_interpolant_finalize (GObject *obj)
{
  GthreeCubicInterpolant *interpolant = GTHREE_CUBIC_INTERPOLANT (obj);
  GthreeCubicInterpolantPrivate *priv = gthree_cubic_interpolant_get_instance_private (interpolant);

  g_free (priv->floats);

  G_OBJECT_CLASS (gthree_cubic_interpolant_parent_class)->finalize (obj);
}

static void
gthree_cubic_interpolant_class_init (GthreeCubicInterpolantClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_cubic_interpolant_finalize;
  GTHREE_INTERPOLANT_CLASS (klass)->interval_changed = gthree_cubic_interpolant_interval_changed;
  GTHREE_INTERPOLANT_CLASS (klass)->interpolate = gthree_cubic_interpolant_interpolate;
}
