#include <math.h>

#include "gthreepropertymixerprivate.h"

typedef void (*BufferMixFunc) (float *buffer, int dst_offset, int src_offset, float t, int stride);


typedef struct {
  GthreePropertyBinding *binding;
  GthreeValueType value_type;
  int value_size;

  // layout: [ incoming | accu0 | accu1 | orig ]
  //
  // interpolators can use .buffer as their .result
  // the data then goes to 'incoming'
  //
  // 'accu0' and 'accu1' are used frame-interleaved for
  // the cumulative result and are compared to detect
  // changes
  //
  // 'orig' stores the original state of the property
  float *buffer;

  float cumulative_weight;
  BufferMixFunc mix_func;
} GthreePropertyMixerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreePropertyMixer, gthree_property_mixer, G_TYPE_OBJECT)

static void
gthree_property_mixer_init (GthreePropertyMixer *mixer)
{
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);

  priv->cumulative_weight = 0.0;
}

static void
gthree_property_mixer_finalize (GObject *obj)
{
  GthreePropertyMixer *mixer = GTHREE_PROPERTY_MIXER (obj);
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);

  g_clear_object (&priv->binding);

  g_free (priv->buffer);

  G_OBJECT_CLASS (gthree_property_mixer_parent_class)->finalize (obj);
}

static void
gthree_property_mixer_class_init (GthreePropertyMixerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_property_mixer_finalize;
}

static void
mix_buffer_region_quat (float *buffer, int dst_offset, int src_offset, float t, int stride)
{
  graphene_quaternion_t a, b;
  graphene_vec4_t res;

  g_assert (stride == 4);
  graphene_quaternion_init (&a,
                            buffer[src_offset + 0],
                            buffer[src_offset + 1],
                            buffer[src_offset + 2],
                            buffer[src_offset + 3]);
  graphene_quaternion_init (&b,
                            buffer[dst_offset + 0],
                            buffer[dst_offset + 1],
                            buffer[dst_offset + 2],
                            buffer[dst_offset + 3]);
  graphene_quaternion_slerp (&a, &b, t, &b);
  graphene_quaternion_to_vec4 (&b, &res);
  graphene_vec4_to_float (&res, &buffer[dst_offset]);
}

static void
mix_buffer_region_linear_general (float *buffer, int dst_offset, int src_offset, float t, int stride)
{
  // Linear interpolation
  float s = 1 - t;
  int i;

  for (i = 0; i < stride; i++)
    {
      int j = dst_offset + i;
      buffer[j] = buffer[j] * s + buffer[src_offset + i] * t;
    }
}

static void
mix_buffer_region_linear_vec3 (float *buffer, int dst_offset, int src_offset, float t, int stride)
{
  // Linear interpolation with vec3
  graphene_vec3_t a, b;

  graphene_vec3_init_from_float (&a, &buffer[src_offset]);
  graphene_vec3_init_from_float (&b, &buffer[dst_offset]);
  graphene_vec3_scale (&a, t, &a);
  graphene_vec3_scale (&b, 1 - t, &b);
  graphene_vec3_add (&a, &b, &b);
  graphene_vec3_to_float (&b, &buffer[dst_offset]);
}

// accumulate data in the 'incoming' region into 'accu<i>'
void
gthree_property_mixer_accumulate (GthreePropertyMixer *mixer, int accu_index, float weight)
{
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);
  int stride = priv->value_size;
  float *buffer = priv->buffer;
  int offset = accu_index * stride + stride;
  float current_weight = priv->cumulative_weight;
  int i;

  // note: happily accumulating nothing when weight = 0, the caller knows
  // the weight and shouldn't have made the call in the first place

  if (current_weight == 0)
    {
      // accuN := incoming * weight
      for (i = 0; i < stride; i++)
        buffer[offset + i] = buffer[i];

      current_weight = weight;
    }
  else
    {
      // accuN := accuN + incoming * weight
      current_weight += weight;
      priv->mix_func (buffer, offset, 0, weight / current_weight, stride);
    }

  priv->cumulative_weight = current_weight;
}

// apply the state of 'accu<i>' to the binding when accus differ
void
gthree_property_mixer_apply (GthreePropertyMixer *mixer, int accu_index)
{
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);
  int stride = priv->value_size;
  float *buffer = priv->buffer;
  int offset = accu_index * stride + stride;
  float weight = priv->cumulative_weight;
  gboolean changed;
  int i;

  priv->cumulative_weight = 0;
  if (weight < 1)
    {
      // accuN := accuN + original * ( 1 - cumulativeWeight )
      int original_value_offset = stride * 3;
      priv->mix_func (buffer, offset, original_value_offset, 1 - weight, stride);
    }

  changed = FALSE;
  for (i = stride; i < stride + stride; i++)
    {
      if (buffer[i] != buffer[i + stride])
        {
          changed = TRUE;
          break;
        }
    }

  // If some component has changed -> update scene graph
  if (changed)
    ghtree_property_binding_set_value (priv->binding, buffer, offset);
}

// remember the state of the bound property and copy it to both accus
void
gthree_property_mixer_save_original_state (GthreePropertyMixer *mixer, int accu_index, float weight)
{
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);
  int stride = priv->value_size;
  float *buffer = priv->buffer;
  int original_value_offset = stride * 3;
  int i;

  ghtree_property_binding_get_value (priv->binding, buffer, original_value_offset);

  // accu[0..1] := orig -- initially detect changes against the original
  for (i = stride; i < original_value_offset; i++)
    buffer[i] = buffer[original_value_offset + (i % stride)];

  priv->cumulative_weight = 0;
}

// apply the state previously taken via 'saveOriginalState' to the binding
void
gthree_property_mixer_restore_original_state (GthreePropertyMixer *mixer, int accu_index, float weight)
{
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);
  int stride = priv->value_size;
  float *buffer = priv->buffer;
  int original_value_offset = stride * 3;

 ghtree_property_binding_set_value (priv->binding,
                                    buffer, original_value_offset);
}

GthreePropertyMixer *
gthree_property_mixer_new (GthreePropertyBinding *binding, GthreeValueType value_type, int value_size)
{
  GthreePropertyMixer *mixer = g_object_new (GTHREE_TYPE_PROPERTY_MIXER, NULL);
  GthreePropertyMixerPrivate *priv = gthree_property_mixer_get_instance_private (mixer);

  priv->binding = g_object_ref (binding);
  priv->value_type = value_type;
  priv->value_size = value_size;

  priv->buffer = g_new (float, 4 * value_size);

  switch (value_type)
    {
    case GTHREE_VALUE_TYPE_QUATERNION:
      if (value_size == 4)
        priv->mix_func = mix_buffer_region_quat;
      else
        {
          g_warning ("quaternion value type with wrong size");
          priv->mix_func = mix_buffer_region_linear_general;
        }
      break;
    case GTHREE_VALUE_TYPE_COLOR:
    case GTHREE_VALUE_TYPE_NUMBER:
    case GTHREE_VALUE_TYPE_VECTOR:
    default:
      if (value_size == 3)
        priv->mix_func = mix_buffer_region_linear_vec3;
      else
        priv->mix_func = mix_buffer_region_linear_general;
      break;
    }

  return mixer;
}
