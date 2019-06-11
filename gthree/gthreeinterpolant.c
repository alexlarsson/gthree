#include "gthreeinterpolant.h"

typedef struct {
  GthreeAttributeArray *parameter_positions; /* Must be float */
  GthreeAttributeArray *sample_values; /* Any kind of value */
  GthreeAttributeArray *result_buffer; /* Same type as sample_values */
  int cached_index;
} GthreeInterpolantPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeInterpolant, gthree_interpolant, G_TYPE_OBJECT)

GthreeInterpolant *
gthree_interpolant_create (GType type,
                           GthreeAttributeArray *parameter_positions,
                           GthreeAttributeArray *sample_values)
{
  GthreeInterpolant *interpolant = g_object_new (type, NULL);
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);

  g_assert (gthree_attribute_array_get_attribute_type (priv->parameter_positions) == GTHREE_ATTRIBUTE_TYPE_FLOAT);
  g_assert (gthree_attribute_array_get_stride (priv->parameter_positions) == 1);

  g_assert (gthree_attribute_array_get_count (priv->parameter_positions) == gthree_attribute_array_get_count (priv->sample_values));

  priv->parameter_positions = gthree_attribute_array_ref (parameter_positions);
  priv->sample_values = gthree_attribute_array_ref (sample_values);
  priv->result_buffer = gthree_attribute_array_new (gthree_attribute_array_get_attribute_type (sample_values),
                                                    1, gthree_attribute_array_get_stride (sample_values));

  return interpolant;
}

static void
gthree_interpolant_init (GthreeInterpolant *interpolant)
{
}

static void
gthree_interpolant_finalize (GObject *obj)
{
  GthreeInterpolant *interpolant = GTHREE_INTERPOLANT (obj);
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);

  gthree_attribute_array_unref (priv->parameter_positions);
  gthree_attribute_array_unref (priv->sample_values);
  gthree_attribute_array_unref (priv->result_buffer);

  G_OBJECT_CLASS (gthree_interpolant_parent_class)->finalize (obj);
}

static void
gthree_interpolant_class_init (GthreeInterpolantClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_interpolant_finalize;
}

GthreeAttributeArray *
gthree_interpolant_get_parameter_positions (GthreeInterpolant *interpolant)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  return priv->parameter_positions;
}

GthreeAttributeArray *
gthree_interpolant_get_sample_values (GthreeInterpolant *interpolant)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  return priv->sample_values;
}

int
gthree_interpolant_get_n_positions (GthreeInterpolant *interpolant)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  return gthree_attribute_array_get_count (priv->parameter_positions);
}

int
gthree_interpolant_get_sample_size (GthreeInterpolant *interpolant)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  return gthree_attribute_array_get_stride (priv->sample_values);
}

GthreeAttributeType
gthree_interpolant_get_sample_type (GthreeInterpolant *interpolant)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  return gthree_attribute_array_get_attribute_type (priv->sample_values);
}

void
gthree_interpolant_copy_sample_value (GthreeInterpolant *interpolant, int index)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);

  gthree_attribute_array_copy_at (priv->result_buffer, 0, 0,
                                  priv->sample_values, index, 0,
                                  gthree_attribute_array_get_stride (priv->sample_values),
                                  1);
}

static GthreeAttributeArray *
gthree_interpolant_before_start (GthreeInterpolant *interpolant, int i, float t, float t1)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);

  gthree_interpolant_copy_sample_value (interpolant, i);

  return priv->result_buffer;
}

static GthreeAttributeArray *
gthree_interpolant_after_end (GthreeInterpolant *interpolant, int i, float t, float t0)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);

  gthree_interpolant_copy_sample_value (interpolant, i);

  return priv->result_buffer;
}

static void
gthree_interpolant_interval_changed (GthreeInterpolant *interpolant, int i1, float t0, float t1)
{
  GthreeInterpolantClass *class = GTHREE_INTERPOLANT_GET_CLASS(interpolant);

  if (class->interval_changed)
    class->interval_changed (interpolant, i1, t0, t1);
}

static GthreeAttributeArray *
gthree_interpolant_interpolate (GthreeInterpolant *interpolant, int i1, float t0, float t, float t1)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  GthreeInterpolantClass *class = GTHREE_INTERPOLANT_GET_CLASS(interpolant);

  class->interpolate (interpolant, i1, t0, t, t1, priv->result_buffer);

  return priv->result_buffer;
}

GthreeAttributeArray *
gthree_interpolant_evaluate (GthreeInterpolant *interpolant, float t)
{
  GthreeInterpolantPrivate *priv = gthree_interpolant_get_instance_private (interpolant);
  int i1 = priv->cached_index;
  float *pp = gthree_attribute_array_peek_float (priv->parameter_positions);
  int pp_len = gthree_attribute_array_get_count (priv->parameter_positions);
  int right, mid;

  // We assume cached_index is always a in [0, pp_len] (i.e. can be at most one index oob)
  // And "t" is between [cached_index-1,cached_index)
  float t1 = i1 < pp_len ? pp[i1] : G_MAXFLOAT;
  float t0 = i1 > 0 ? pp[i1-1] : -G_MAXFLOAT;

  if (t >= t1)
    {
      int give_up_at = i1 + 2;

      // linear forward scan
      while (TRUE)
        {
          if (i1 == pp_len)
            {
              // After end
              priv->cached_index = i1;
              return gthree_interpolant_after_end (interpolant, i1 - 1, t, t0);
            }

          if (i1 == give_up_at)
            break;

          t0 = t1;
          t1 = pp[++i1]; // This is ok, because we checked pp_len above
          if (t < t1)
            {
              priv->cached_index = i1;
              gthree_interpolant_interval_changed (interpolant, i1, t0, t1);
              return gthree_interpolant_interpolate (interpolant, i1, t0, t, t1);
            }
        }

      // prepare binary search on the right side of the index
      right = pp_len;
    }
  else if (t < t0)
    {
      int give_up_at;

      // Did we loop back to 0? Quick check
      float t1global = pp[1];
      if ( t < t1global )
        {
          i1 = 2; // + 1, using the scan for the details
          t0 = t1global;
        }

      give_up_at = i1 - 2;

      // linear reverse scan
      while (TRUE)
        {
          if (i1 == 0)
            {
              // Before start
              priv->cached_index = 0;
              return gthree_interpolant_before_start (interpolant, 0, t, t1);
            }

          if (i1 == give_up_at)
            break;

          t1 = t0;
          t0 = pp[i1 - 1]; // This is ok, because we checked i1 == 0 above
          if (t >= t0)
            {
              priv->cached_index = i1;
              gthree_interpolant_interval_changed (interpolant, i1, t0, t1);
              return gthree_interpolant_interpolate (interpolant, i1, t0, t, t1);
            }
          i1--;
        }

      // prepare binary search on the left side of the index
      right = i1;
      i1 = 0;
    }
  else
    {
      /* Already valid */
      if (i1 == pp_len)
        return gthree_interpolant_after_end (interpolant, i1 - 1, t, pp[i1-1]);
      if (i1 == 0)
        return gthree_interpolant_before_start (interpolant, 0, t, pp[0]);
      return gthree_interpolant_interpolate (interpolant, i1, t0, t, t1);
    }

  /* Binary search between i1 and right */
  while (i1 < right)
    {
      mid = (i1 + right) / 2;
      if (t < pp[mid])
        right = mid;
      else
        i1 = mid + 1;
    }

  priv->cached_index = i1;
  if (i1 == pp_len)
    return gthree_interpolant_after_end (interpolant, i1 - 1, t, pp[i1-1]);
  if (i1 == 0)
    return gthree_interpolant_before_start (interpolant, 0, t, pp[0]);

  t1 = pp[i1];
  t0 = pp[i1-1];
  gthree_interpolant_interval_changed (interpolant, i1, t0, t1);
  return gthree_interpolant_interpolate (interpolant, i1, t0, t, t1);
}
