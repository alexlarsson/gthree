#include "gthreequaternioninterpolant.h"
#include "gthreeprivate.h"

typedef struct {
  graphene_quaternion_t *quats;
} GthreeQuaternionInterpolantPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeQuaternionInterpolant, gthree_quaternion_interpolant, GTHREE_TYPE_INTERPOLANT)

GthreeInterpolant *
gthree_quaternion_interpolant_new (GthreeAttributeArray *parameter_positions,
                                   GthreeAttributeArray *sample_values)
{
  GthreeInterpolant *interpolant;
  GthreeQuaternionInterpolantPrivate *priv;
  int sample_size, n_quats;

  interpolant = gthree_interpolant_create (GTHREE_TYPE_QUATERNION_INTERPOLANT, parameter_positions, sample_values);
  priv = gthree_quaternion_interpolant_get_instance_private (GTHREE_QUATERNION_INTERPOLANT (interpolant));

  sample_size = gthree_interpolant_get_sample_size (interpolant);

  g_assert (gthree_attribute_array_get_attribute_type (sample_values) == GTHREE_ATTRIBUTE_TYPE_FLOAT);
  g_assert (sample_size % 4 == 0);

  n_quats = sample_size/4;

  priv->quats = g_new (graphene_quaternion_t, 2 * n_quats);

  return interpolant;
}

static void
gthree_quaternion_interpolant_init (GthreeQuaternionInterpolant *interpolant)
{
}

static void
gthree_quaternion_interpolant_interval_changed (GthreeInterpolant *interpolant, int i1, float t0, float t1)
{
  GthreeQuaternionInterpolant *quaternion = GTHREE_QUATERNION_INTERPOLANT (interpolant);
  GthreeQuaternionInterpolantPrivate *priv = gthree_quaternion_interpolant_get_instance_private (quaternion);
  GthreeAttributeArray *sample_values;
  int sample_size, i, n_quats;

  sample_size = gthree_interpolant_get_sample_size (interpolant);
  sample_values = gthree_interpolant_get_sample_values (interpolant);

  n_quats = sample_size/4;

  /* i1 - 1 */
  for (i = 0; i < n_quats; i++)
    {
      graphene_vec4_t v;
      gthree_attribute_array_get_vec4 (sample_values, i1 - 1, i * 4, &v);
      graphene_quaternion_init_from_vec4 (&priv->quats[i], &v);
    }

  /* i1 */
  for (i = 0; i < n_quats; i++)
    {
      graphene_vec4_t v;
      gthree_attribute_array_get_vec4 (sample_values, i1, i * 4, &v);
      graphene_quaternion_init_from_vec4 (&priv->quats[i + n_quats], &v);
    }
}

static void
gthree_quaternion_interpolant_interpolate (GthreeInterpolant *interpolant, int i1, float t0, float t, float t1, GthreeAttributeArray *dest)
{
  GthreeQuaternionInterpolant *quaternion = GTHREE_QUATERNION_INTERPOLANT (interpolant);
  GthreeQuaternionInterpolantPrivate *priv = gthree_quaternion_interpolant_get_instance_private (quaternion);
  float alpha;
  int sample_size, i, n_quats;

  sample_size = gthree_interpolant_get_sample_size (interpolant);
  n_quats = sample_size/4;

  alpha = ( t - t0 ) / ( t1 - t0 );

  for (i = 0; i < n_quats; i++)
    {
      graphene_quaternion_t res;
      graphene_vec4_t v;

      graphene_quaternion_slerp (&priv->quats[i],
                                 &priv->quats[i + n_quats],
                                 alpha, &res);
      graphene_quaternion_to_vec4 (&res, &v);

      gthree_attribute_array_set_vec4 (dest, 0, i * 4, &v);
    }

}

static void
gthree_quaternion_interpolant_finalize (GObject *obj)
{
  GthreeQuaternionInterpolant *interpolant = GTHREE_QUATERNION_INTERPOLANT (obj);
  GthreeQuaternionInterpolantPrivate *priv = gthree_quaternion_interpolant_get_instance_private (interpolant);

  g_free (priv->quats);

  G_OBJECT_CLASS (gthree_quaternion_interpolant_parent_class)->finalize (obj);
}

static void
gthree_quaternion_interpolant_class_init (GthreeQuaternionInterpolantClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_quaternion_interpolant_finalize;
  GTHREE_INTERPOLANT_CLASS (klass)->interval_changed = gthree_quaternion_interpolant_interval_changed;
  GTHREE_INTERPOLANT_CLASS (klass)->interpolate = gthree_quaternion_interpolant_interpolate;
}
