#include <math.h>
#include <epoxy/gl.h>

#include "gthreeuniforms.h"
#include "gthreeprivate.h"

struct _GthreeUniform {
  GQuark name;
  GthreeUniformType type;
  gint location;
  gboolean needs_update;
  union {
    float floats[4];
    int ints[4];
    float *more_floats; /* Used for matrix types w/ 9/16 floats */
    GArray *array;
    GthreeTexture *texture;
    GPtrArray *ptr_array;
  } value;
};

typedef struct {
  GHashTable *hash;
} GthreeUniformsPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeUniforms, gthree_uniforms, G_TYPE_OBJECT);

static void gthree_uniform_free (GthreeUniform *uniform);
static GthreeUniform *gthree_uniform_clone (GthreeUniform *uniform);
static void gthree_uniforms_init_libs ();

GthreeUniforms *
gthree_uniforms_new ()
{
  GthreeUniforms *uniforms;

  uniforms = g_object_new (gthree_uniforms_get_type (),
                          NULL);

  return uniforms;
}

static void
gthree_uniforms_init (GthreeUniforms *uniforms)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);

  priv->hash = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)gthree_uniform_free);
}

static void
gthree_uniforms_finalize (GObject *obj)
{
  GthreeUniforms *uniforms = GTHREE_UNIFORMS (obj);
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);

  g_hash_table_destroy (priv->hash);

  G_OBJECT_CLASS (gthree_uniforms_parent_class)->finalize (obj);
}

static void
gthree_uniforms_class_init (GthreeUniformsClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_uniforms_finalize;

  gthree_uniforms_init_libs ();
}

void
gthree_uniforms_add (GthreeUniforms *uniforms,
                     GthreeUniform  *uniform)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);

  g_hash_table_replace (priv->hash, GINT_TO_POINTER (uniform->name), uniform);
}

GthreeUniform *
gthree_uniforms_lookup (GthreeUniforms *uniforms,
                        GQuark name)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);

  return g_hash_table_lookup (priv->hash, GINT_TO_POINTER (name));
}

GList  *
gthree_uniforms_get_all (GthreeUniforms *uniforms)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);
  GList *all;
  GHashTableIter iter;
  gpointer value;

  all = NULL;
  g_hash_table_iter_init (&iter, priv->hash);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      GthreeUniform *uniform = value;
      all = g_list_prepend (all, uniform);
    }

  return all;
}

GthreeUniform *
gthree_uniforms_lookup_from_string (GthreeUniforms *uniforms,
                                    const char *name)
{
  return gthree_uniforms_lookup (uniforms, g_quark_from_string (name));
}

GthreeUniforms *
gthree_uniforms_clone (GthreeUniforms *uniforms)
{
  GthreeUniforms *clone;

  clone = gthree_uniforms_new ();
  gthree_uniforms_merge (clone, uniforms);

  return clone;
}

void
gthree_uniforms_merge (GthreeUniforms *uniforms,
                       GthreeUniforms *source)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);
  GthreeUniformsPrivate *source_priv = gthree_uniforms_get_instance_private (source);
  GHashTableIter iter;
  GthreeUniform *uniform;

  g_hash_table_iter_init (&iter, source_priv->hash);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&uniform))
    g_hash_table_replace (priv->hash, GINT_TO_POINTER (uniform->name), gthree_uniform_clone (uniform));
}

void
gthree_uniforms_load (GthreeUniforms *uniforms,
                      GthreeRenderer *renderer)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);
  GHashTableIter iter;
  GthreeUniform *uniform;

  g_hash_table_iter_init (&iter, priv->hash);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&uniform))
    gthree_uniform_load (uniform, renderer);
}

void
gthree_uniforms_set_float (GthreeUniforms  *uniforms,
                           const char      *name,
                           double           value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_float (uni, value);
}

void
gthree_uniforms_set_float_array (GthreeUniforms  *uniforms,
                                 const char      *name,
                                 GArray          *array)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_float_array (uni, array);
}

void
gthree_uniforms_set_float3_array (GthreeUniforms  *uniforms,
                                  const char      *name,
                                  GArray          *array)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_float3_array (uni, array);
}

void
gthree_uniforms_set_matrix4_array (GthreeUniforms  *uniforms,
                                   const char      *name,
                                   GArray          *array)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_matrix4_array (uni, array);
}

void
gthree_uniforms_set_int (GthreeUniforms  *uniforms,
                         const char      *name,
                         int              value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_int (uni, value);
}

void
gthree_uniforms_set_vec4 (GthreeUniforms  *uniforms,
                          const char      *name,
                          graphene_vec4_t *value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_vec4 (uni, value);
}

void
gthree_uniforms_set_vec3 (GthreeUniforms  *uniforms,
                          const char      *name,
                          graphene_vec3_t *value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_vec3 (uni, value);
}

void
gthree_uniforms_set_vec2 (GthreeUniforms  *uniforms,
                          const char      *name,
                          graphene_vec2_t *value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_vec2 (uni, value);
}

void
gthree_uniforms_set_texture (GthreeUniforms  *uniforms,
                             const char      *name,
                             GthreeTexture   *value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_texture (uni, value);
}

void
gthree_uniforms_set_texture_array (GthreeUniforms  *uniforms,
                                   const char      *name,
                                   GPtrArray       *value)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_texture_array (uni, value);
}

void
gthree_uniforms_set_uarray (GthreeUniforms  *uniforms,
                            const char      *name,
                            GPtrArray       *value,
                            gboolean         update_existing)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, name);
  if (uni)
    gthree_uniform_set_uarray (uni, value, update_existing);
}

void
gthree_uniforms_copy_values (GthreeUniforms *uniforms,
                             GthreeUniforms *source)
{
  GthreeUniformsPrivate *priv = gthree_uniforms_get_instance_private (uniforms);
  GthreeUniformsPrivate *source_priv = gthree_uniforms_get_instance_private (source);
  GHashTableIter iter;
  GthreeUniform *src_uniform;

  g_hash_table_iter_init (&iter, source_priv->hash);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&src_uniform))
    {
      GthreeUniform *dst_uniform = g_hash_table_lookup (priv->hash, GINT_TO_POINTER (src_uniform->name));

      if (dst_uniform == NULL)
        g_warning ("gthree_uniforms_copy_values, dst has no %s uniform", gthree_uniform_get_name (src_uniform));
      else
        gthree_uniform_copy_value (dst_uniform, src_uniform);
    }
}

GthreeUniform *
gthree_uniform_newq (GQuark name, GthreeUniformType type)
{
  GthreeUniform *uniform = g_slice_new0 (GthreeUniform);

  uniform->name = name;
  uniform->type = type;
  uniform->location = 0;
  uniform->needs_update = TRUE;

  return uniform;
}

const char *
gthree_uniform_get_name (GthreeUniform *uniform)
{
  return g_quark_to_string (uniform->name);
}

GQuark
gthree_uniform_get_qname (GthreeUniform *uniform)
{
  return uniform->name;
}

GthreeUniformType
gthree_uniform_get_type (GthreeUniform *uniform)
{
  return uniform->type;
}


GthreeUniform *
gthree_uniform_new (const char *name, GthreeUniformType type)
{
  return gthree_uniform_newq (g_quark_from_string (name), type);
}

static void
gthree_uniform_free (GthreeUniform *uniform)
{
  switch (uniform->type)
    {
    case GTHREE_UNIFORM_TYPE_MATRIX3:
    case GTHREE_UNIFORM_TYPE_MATRIX4:
      g_free (uniform->value.more_floats);
      break;

    case GTHREE_UNIFORM_TYPE_INT_ARRAY:
    case GTHREE_UNIFORM_TYPE_INT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT2_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC2_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC3_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC4_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY:
      if (uniform->value.array)
        g_array_unref (uniform->value.array);
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE:
      g_clear_object (&uniform->value.texture);
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
    case GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY:
      if (uniform->value.ptr_array)
        g_ptr_array_unref (uniform->value.ptr_array);
      break;
    case GTHREE_UNIFORM_TYPE_INT:
    case GTHREE_UNIFORM_TYPE_FLOAT:
    case GTHREE_UNIFORM_TYPE_FLOAT2:
    case GTHREE_UNIFORM_TYPE_FLOAT3:
    case GTHREE_UNIFORM_TYPE_FLOAT4:
    case GTHREE_UNIFORM_TYPE_VECTOR2:
    case GTHREE_UNIFORM_TYPE_VECTOR3:
    case GTHREE_UNIFORM_TYPE_VECTOR4:
      /* Do nothing */
      break;
    }

  g_slice_free (GthreeUniform, uniform);
}

void
gthree_uniform_set_location (GthreeUniform *uniform,
                             int location)
{
  uniform->location = location;
}

gboolean
gthree_uniform_is_array (GthreeUniform *uniform)
{
  switch (uniform->type)
    {
    case GTHREE_UNIFORM_TYPE_INT_ARRAY:
    case GTHREE_UNIFORM_TYPE_INT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT2_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC2_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC3_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC4_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY:
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
    case GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY:
      return TRUE;

    default:
    case GTHREE_UNIFORM_TYPE_TEXTURE:
    case GTHREE_UNIFORM_TYPE_INT:
    case GTHREE_UNIFORM_TYPE_FLOAT:
    case GTHREE_UNIFORM_TYPE_FLOAT2:
    case GTHREE_UNIFORM_TYPE_FLOAT3:
    case GTHREE_UNIFORM_TYPE_FLOAT4:
    case GTHREE_UNIFORM_TYPE_VECTOR2:
    case GTHREE_UNIFORM_TYPE_VECTOR3:
    case GTHREE_UNIFORM_TYPE_VECTOR4:
    case GTHREE_UNIFORM_TYPE_MATRIX3:
    case GTHREE_UNIFORM_TYPE_MATRIX4:
      return FALSE;
    }
}

void
gthree_uniform_set_needs_update (GthreeUniform *uniform,
                                 gboolean needs_update)
{
  uniform->needs_update = needs_update;
}

static void
maybe_g_object_unref (gpointer object)
{
  if (object)
    g_object_unref (object);
}

void
gthree_uniform_copy_value (GthreeUniform   *uniform,
                           GthreeUniform   *source)
{
  g_assert (uniform->type == source->type);

  uniform->value = source->value;

  switch (uniform->type)
    {
    case GTHREE_UNIFORM_TYPE_INT_ARRAY:
    case GTHREE_UNIFORM_TYPE_INT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT2_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC2_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC3_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC4_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY:
      if (uniform->value.array)
        {
          gsize elem_size = g_array_get_element_size (uniform->value.array);
          guint len = uniform->value.array->len;
          uniform->value.array =
            g_array_sized_new (FALSE, FALSE, elem_size, len);
          g_array_set_size (uniform->value.array, len);
          memcpy (uniform->value.array->data, source->value.array->data, len * elem_size);
        }
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE:
      if (uniform->value.texture)
        g_object_ref (uniform->value.texture);
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
      if (uniform->value.ptr_array)
        {
          guint i, len = uniform->value.ptr_array->len;
          uniform->value.ptr_array = g_ptr_array_new_with_free_func (maybe_g_object_unref);
          for (i = 0; i < len; i++)
            g_ptr_array_add (uniform->value.ptr_array,
                             g_object_ref (g_ptr_array_index (source->value.ptr_array, i)));
        }
      break;
    case GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY:
      if (uniform->value.ptr_array)
        {
          guint i, len = uniform->value.ptr_array->len;
          uniform->value.ptr_array = g_ptr_array_new_with_free_func (g_object_unref);

          /* Deep clone */
          for (i = 0; i < len; i++)
            g_ptr_array_add (uniform->value.ptr_array,
                             gthree_uniform_clone (g_ptr_array_index (source->value.ptr_array, i)));
        }
      break;
    case GTHREE_UNIFORM_TYPE_MATRIX3:
      uniform->value.more_floats = g_memdup (uniform->value.more_floats, sizeof (float) * 9);
      break;
    case GTHREE_UNIFORM_TYPE_MATRIX4:
      uniform->value.more_floats = g_memdup (uniform->value.more_floats, sizeof (float) * 16);
      break;
    case GTHREE_UNIFORM_TYPE_INT:
    case GTHREE_UNIFORM_TYPE_FLOAT:
    case GTHREE_UNIFORM_TYPE_FLOAT2:
    case GTHREE_UNIFORM_TYPE_FLOAT3:
    case GTHREE_UNIFORM_TYPE_FLOAT4:
    case GTHREE_UNIFORM_TYPE_VECTOR2:
    case GTHREE_UNIFORM_TYPE_VECTOR3:
    case GTHREE_UNIFORM_TYPE_VECTOR4:
      /* Do nothing */
      break;
    }
}

static GthreeUniform *
gthree_uniform_clone (GthreeUniform *uniform)
{
  GthreeUniform *clone = g_slice_new (GthreeUniform);

  *clone = *uniform;

  switch (uniform->type)
    {
    case GTHREE_UNIFORM_TYPE_INT_ARRAY:
    case GTHREE_UNIFORM_TYPE_INT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT2_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY:
    case GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC2_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC3_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC4_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY:
      if (uniform->value.array)
        {
          gsize elem_size = g_array_get_element_size (uniform->value.array);
          guint len = uniform->value.array->len;
          clone->value.array =
            g_array_sized_new (FALSE, FALSE, elem_size, len);
          g_array_set_size (clone->value.array, len);
          memcpy (clone->value.array->data, uniform->value.array->data, len * elem_size);
        }
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE:
      if (clone->value.texture)
        g_object_ref (clone->value.texture);
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
      if (uniform->value.ptr_array)
        {
          guint i, len = uniform->value.ptr_array->len;
          uniform->value.ptr_array = g_ptr_array_new_with_free_func (maybe_g_object_unref);
          for (i = 0; i < len; i++)
            g_ptr_array_add (uniform->value.ptr_array,
                             g_object_ref (g_ptr_array_index (uniform->value.ptr_array, i)));
        }
      break;
    case GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY:
      if (uniform->value.ptr_array)
        {
          guint i, len = uniform->value.ptr_array->len;
          clone->value.ptr_array = g_ptr_array_new_with_free_func (g_object_unref);

          /* Deep clone */
          for (i = 0; i < len; i++)
            g_ptr_array_add (clone->value.ptr_array,
                             gthree_uniform_clone (g_ptr_array_index (uniform->value.ptr_array, i)));
        }
      break;
    case GTHREE_UNIFORM_TYPE_MATRIX3:
      clone->value.more_floats = g_memdup (clone->value.more_floats, sizeof (float) * 9);
      break;
    case GTHREE_UNIFORM_TYPE_MATRIX4:
      clone->value.more_floats = g_memdup (clone->value.more_floats, sizeof (float) * 16);
      break;
    case GTHREE_UNIFORM_TYPE_INT:
    case GTHREE_UNIFORM_TYPE_FLOAT:
    case GTHREE_UNIFORM_TYPE_FLOAT2:
    case GTHREE_UNIFORM_TYPE_FLOAT3:
    case GTHREE_UNIFORM_TYPE_FLOAT4:
    case GTHREE_UNIFORM_TYPE_VECTOR2:
    case GTHREE_UNIFORM_TYPE_VECTOR3:
    case GTHREE_UNIFORM_TYPE_VECTOR4:
      /* Do nothing */
      break;
    }

  return clone;
}

void
gthree_uniform_set_float (GthreeUniform *uniform,
                          double val)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_FLOAT);
  uniform->value.floats[0] = val;
}

static void
set_array (GthreeUniform *uniform, GArray *array)
{
  if (array)
    g_array_ref (array);
  if (uniform->value.array)
    g_array_unref (uniform->value.array);

  uniform->value.array = array;
}

void
gthree_uniform_set_float_array (GthreeUniform *uniform,
                                GArray *array)
{
 g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_FLOAT_ARRAY);

 set_array (uniform, array);
}

void
gthree_uniform_set_float3_array (GthreeUniform *uniform,
                                 GArray *array)
{
 g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY);

 set_array (uniform, array);
}

void
gthree_uniform_set_float4_array (GthreeUniform *uniform,
                                 GArray *array)
{
 g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY);

 set_array (uniform, array);
}

void
gthree_uniform_set_matrix4_array (GthreeUniform  *uniform,
                                  GArray          *array)
{
 g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY);

 set_array (uniform, array);
}

void
gthree_uniform_set_int (GthreeUniform *uniform,
                        int val)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_INT);
  uniform->value.ints[0] = val;
}

void
gthree_uniform_set_vec4 (GthreeUniform *uniform,
                         graphene_vec4_t *value)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_VECTOR4);
  uniform->value.floats[0] = graphene_vec4_get_x (value);
  uniform->value.floats[1] = graphene_vec4_get_y (value);
  uniform->value.floats[2] = graphene_vec4_get_z (value);
  uniform->value.floats[3] = graphene_vec4_get_w (value);
}

void
gthree_uniform_set_vec3 (GthreeUniform *uniform,
                         graphene_vec3_t *value)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_VECTOR3);
  uniform->value.floats[0] = graphene_vec3_get_x (value);
  uniform->value.floats[1] = graphene_vec3_get_y (value);
  uniform->value.floats[2] = graphene_vec3_get_z (value);
}

void
gthree_uniform_set_vec2 (GthreeUniform *uniform,
                         graphene_vec2_t *value)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_VECTOR2);
  uniform->value.floats[0] = graphene_vec2_get_x (value);
  uniform->value.floats[1] = graphene_vec2_get_y (value);
}

void
gthree_uniform_set_texture (GthreeUniform *uniform,
                            GthreeTexture *value)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_TEXTURE);

  if (value)
    g_object_ref (value);
  if (uniform->value.texture)
    g_object_unref (uniform->value.texture);

  uniform->value.texture = value;
}

void
gthree_uniform_set_texture_array (GthreeUniform *uniform,
                                  GPtrArray *value)
{
  int i;

  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY);

  if (uniform->value.ptr_array == NULL)
    uniform->value.ptr_array = g_ptr_array_new_with_free_func (maybe_g_object_unref);

  g_ptr_array_set_size (uniform->value.ptr_array, 0);
  for (i = 0; i < value->len; i++)
    {
      GthreeTexture *src = g_ptr_array_index (value, i);
      if (src)
        g_ptr_array_add (uniform->value.ptr_array, g_object_ref (src));
      else
        g_ptr_array_add (uniform->value.ptr_array, NULL);
    }
}

void
gthree_uniform_set_uarray (GthreeUniform *uniform,
                           GPtrArray     *value,
                           gboolean       update_existing)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY);
  int i;

  if (uniform->value.ptr_array == NULL)
    uniform->value.ptr_array = g_ptr_array_new_with_free_func (g_object_unref);

  if (update_existing)
    {
      g_assert (uniform->value.ptr_array->len == value->len); // You should always update with the same size
      for (i = 0; i < value->len; i++)
        {
          GthreeUniforms *src = g_ptr_array_index (value, i);
          GthreeUniforms *dst = g_ptr_array_index (uniform->value.ptr_array, i);

          gthree_uniforms_copy_values (dst, src);
        }
    }
  else
    {
      /* Start from scratch */
      g_ptr_array_set_size (uniform->value.ptr_array, 0);

      for (i = 0; i < value->len; i++)
        {
          GthreeUniforms *src = g_ptr_array_index (value, i);
          g_ptr_array_add (uniform->value.ptr_array, gthree_uniforms_clone (src));
        }
    }
}

GPtrArray *
gthree_uniform_get_uarray (GthreeUniform *uniform)
{
  g_return_val_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY, NULL);

  return uniform->value.ptr_array;
}

void
gthree_uniform_load (GthreeUniform *uniform,
                     GthreeRenderer *renderer)
{
  if (uniform->location == -1)
    return;

  if (!uniform->needs_update)
    return;

  switch (uniform->type)
    {
    case GTHREE_UNIFORM_TYPE_INT:
      glUniform1i (uniform->location, uniform->value.ints[0]);
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT:
      glUniform1f (uniform->location, uniform->value.floats[0]);
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT2:
    case GTHREE_UNIFORM_TYPE_VECTOR2:
      glUniform2f (uniform->location, uniform->value.floats[0], uniform->value.floats[1]);
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT3:
    case GTHREE_UNIFORM_TYPE_VECTOR3:
      glUniform3f (uniform->location, uniform->value.floats[0], uniform->value.floats[1], uniform->value.floats[2]);
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT4:
    case GTHREE_UNIFORM_TYPE_VECTOR4:
      glUniform4f (uniform->location, uniform->value.floats[0], uniform->value.floats[1], uniform->value.floats[2], uniform->value.floats[3]);
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT_ARRAY:
      if (uniform->value.array)
        glUniform1fv (uniform->location, uniform->value.array->len, &g_array_index (uniform->value.array, float, 0));
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT2_ARRAY:
      if (uniform->value.array)
        glUniform2fv (uniform->location, uniform->value.array->len / 2, &g_array_index (uniform->value.array, float, 0));
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY:
      if (uniform->value.array)
        glUniform3fv (uniform->location, uniform->value.array->len / 3, &g_array_index (uniform->value.array, float, 0));
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY:
      if (uniform->value.array)
        glUniform4fv (uniform->location, uniform->value.array->len / 4, &g_array_index (uniform->value.array, float, 0));
      break;
    case GTHREE_UNIFORM_TYPE_MATRIX3:
      glUniformMatrix3fv (uniform->location, 1, FALSE, uniform->value.more_floats);
      break;
    case GTHREE_UNIFORM_TYPE_MATRIX4:
      glUniformMatrix4fv (uniform->location, 1, FALSE, uniform->value.more_floats);
      break;
    case GTHREE_UNIFORM_TYPE_INT_ARRAY:
      if (uniform->value.array)
        glUniform1iv (uniform->location, uniform->value.array->len, &g_array_index (uniform->value.array, int, 0));
      break;
    case GTHREE_UNIFORM_TYPE_INT3_ARRAY:
      if (uniform->value.array)
        glUniform3iv (uniform->location, uniform->value.array->len, &g_array_index (uniform->value.array, int, 0));
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE:
      if (uniform->value.texture)
        {
          int unit = gthree_renderer_allocate_texture_unit (renderer);
          gthree_texture_load (uniform->value.texture, renderer, unit);
          glUniform1i(uniform->location, unit);
        }

      break;
    case GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY:
      if (uniform->value.ptr_array)
        {
          int i;
          for (i = 0; i < uniform->value.ptr_array->len; i++)
            {
              GthreeUniforms *child_unis = g_ptr_array_index (uniform->value.ptr_array, i);
              gthree_uniforms_load (child_unis, renderer);
            }
        }
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
      if (uniform->value.ptr_array)
        {
          guint i, len = uniform->value.ptr_array->len;
          int *units = g_alloca (len * sizeof (int));
          for (i = 0; i < len; i++)
            {
              GthreeTexture *texture = g_ptr_array_index (uniform->value.ptr_array, i);
              if (texture)
                {
                  units[i] = gthree_renderer_allocate_texture_unit (renderer);
                  gthree_texture_load (texture, renderer, units[i]);
                  glUniform1iv (uniform->location, len, units);
                }
            }
        }

      break;
    case GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY:
      if (uniform->value.array)
        {
          guint i, len = uniform->value.array->len;
          float *floats = g_alloca (len * sizeof(float) * 16);

          for (i = 0; i < len; i++)
            {
              graphene_matrix_t *m = &g_array_index (uniform->value.array, graphene_matrix_t, i);
              graphene_matrix_to_float (m, &floats[16*i]);
            }

          glUniformMatrix4fv (uniform->location, len, FALSE, floats);
        }
      break;
    case GTHREE_UNIFORM_TYPE_VEC2_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC3_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC4_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY:
      g_warning ("gthree_uniform_load() - unsupported uniform type %d\n", uniform->type);
    }
}

static int i0 = 0;
static float f0 = 0.0;
static float f1 = 1.0;
static float fm1 = -1.0;
static float f2000 = 2000;
static float fp98 = 0.98;
static float fp00025 = 0.00025;
static float grey[3] = { 0.9333333333333333, 0.9333333333333333, 0.9333333333333333 };
static float white[3] = { 1, 1, 1 };
static float onev2[2] = { 1, 1 };
static float halfv2[2] = { 0.5, 0.5 };
static float one_matrix3[9] = { 1, 0, 0,
                                0, 1, 0,
                                0, 0, 1};

static GthreeUniforms *common;
static GthreeUniformsDefinition common_lib[] = {
  {"diffuse", GTHREE_UNIFORM_TYPE_VECTOR3, &grey },
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },

  {"map", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"uvTransform", GTHREE_UNIFORM_TYPE_MATRIX3, &one_matrix3 },

  {"alphaMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};


static GthreeUniforms *specularmap;
static GthreeUniformsDefinition specularmap_lib[] = {
  {"specularMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};

static GthreeUniforms *envmap;
static GthreeUniformsDefinition envmap_lib[] = {
  {"envMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"flipEnvMap", GTHREE_UNIFORM_TYPE_FLOAT, &fm1 },
  {"reflectivity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"refractionRatio", GTHREE_UNIFORM_TYPE_FLOAT, &fp98 },
  {"maxMipLevel",  GTHREE_UNIFORM_TYPE_INT, &i0 },
};

static GthreeUniforms *aomap;
static GthreeUniformsDefinition aomap_lib[] = {
  {"aoMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"aoMapIntensity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
};

static GthreeUniforms *lightmap;
static GthreeUniformsDefinition lightmap_lib[] = {
  {"lightMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"lightMapIntensity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
};

static GthreeUniforms *emissivemap;
static GthreeUniformsDefinition emissivemap_lib[] = {
  {"emissiveMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};

static GthreeUniforms *bumpmap;
static GthreeUniformsDefinition bumpmap_lib[] = {
  {"bumpMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"bumpScale", GTHREE_UNIFORM_TYPE_FLOAT, &f1 }
};

static GthreeUniforms *normalmap;
static GthreeUniformsDefinition normalmap_lib[] = {
  {"normalMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"normalScale", GTHREE_UNIFORM_TYPE_VECTOR2, &onev2}
};

static GthreeUniforms *displacementmap;
static GthreeUniformsDefinition displacementmap_lib[] = {
  {"displacementMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"displacementScale", GTHREE_UNIFORM_TYPE_FLOAT, &f1},
  {"displacementBias", GTHREE_UNIFORM_TYPE_FLOAT, &f0},
};

static GthreeUniforms *roughnessmap;
static GthreeUniformsDefinition roughnessmap_lib[] = {
  {"roughnessMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};

static GthreeUniforms *glossinessmap;
static GthreeUniformsDefinition glossinessmap_lib[] = {
  {"glossinessMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};

static GthreeUniforms *metalnessmap;
static GthreeUniformsDefinition metalnessmap_lib[] = {
  {"metalnessMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};

static GthreeUniforms *gradientmap;
static GthreeUniformsDefinition gradientmap_lib[] = {
  {"gradientMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};

static GthreeUniforms *fog;
static GthreeUniformsDefinition fog_lib[] = {
  {"fogDensity", GTHREE_UNIFORM_TYPE_FLOAT, &fp00025 },
  {"fogNear", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"fogFar", GTHREE_UNIFORM_TYPE_FLOAT, &f2000 },
  {"fogColor", GTHREE_UNIFORM_TYPE_VECTOR3, &white }
};

/* TODO: Convert this to the structures above */
static GthreeUniforms *lights;
static GthreeUniformsDefinition lights_lib[] = {
  {"ambientLightColor", GTHREE_UNIFORM_TYPE_VECTOR3, NULL},

#ifdef TODO
  lightProbe: { value: [] },
#endif

  {"directionalLights", GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY, NULL},
  {"directionalShadowMap", GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY, NULL},
  {"directionalShadowMatrix", GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY, NULL},
  /*
    properties: {
      direction: {},
      color: {},
      shadow: {},
      shadowBias: {},
      shadowRadius: {},
      shadowMapSize: {}
      }
  */

  {"pointLights", GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY, NULL},
  {"pointShadowMap", GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY, NULL},
  {"pointShadowMatrix", GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY, NULL},
  /*
     properties: {
       color: {},
       position: {},
       decay: {},
       distance: {},

       shadow: {},
       shadowBias: {},
       shadowRadius: {},
       shadowMapSize: {},
       shadowCameraNear: {},
       shadowCameraFar: {}
       }
  */

  {"spotLights", GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY, NULL},
  {"spotShadowMap", GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY, NULL},
  {"spotShadowMatrix", GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY, NULL},
  /*properties: {
    color: {},
    position: {},
    direction: {},
    distance: {},
    coneCos: {},
    penumbraCos: {},
    decay: {},
    shadow: {},
    shadowBias: {},
    shadowRadius: {},
    shadowMapSize: {}
    }
  */

  {"hemisphereLights", GTHREE_UNIFORM_TYPE_UNIFORMS_ARRAY, NULL},
  /*
    properties: {
    direction: {},
    skyColor: {},
    groundColor: {}
    }
  */

/*
  lightProbe: { value: [] },
  // TODO (abelnation): RectAreaLight BRDF data needs to be moved from example to main src
  rectAreaLights: { value: [], properties: {
    color: {},
    position: {},
    width: {},
    height: {}
  } }
  },
*/

};

static GthreeUniforms *points;
static GthreeUniformsDefinition points_lib[] = {
  {"diffuse", GTHREE_UNIFORM_TYPE_VECTOR3, &grey },
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"size", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"scale", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"map", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"uvTransform", GTHREE_UNIFORM_TYPE_MATRIX3, &one_matrix3 },
};

static GthreeUniforms *sprite;
static GthreeUniformsDefinition sprite_lib[] = {
  {"diffuse", GTHREE_UNIFORM_TYPE_VECTOR3, &grey },
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"center", GTHREE_UNIFORM_TYPE_VECTOR2, &halfv2 },
  {"rotation", GTHREE_UNIFORM_TYPE_FLOAT, &f0 },
  {"map", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"uvTransform", GTHREE_UNIFORM_TYPE_MATRIX3, &one_matrix3 },
};


GthreeUniforms *
gthree_uniforms_new_from_definitions (GthreeUniformsDefinition *element, int len)
{
  int i;
  GthreeUniforms *uniforms = gthree_uniforms_new ();
  gpointer value;

  for (i = 0; i < len; i++)
    {
      GthreeUniform *uniform = gthree_uniform_newq (g_quark_from_static_string (element->name), element->type);

      gthree_uniforms_add (uniforms, uniform);
      value = element->value;
      if (value != NULL)
        {
          switch (element->type)
            {
            case GTHREE_UNIFORM_TYPE_INT:
              uniform->value.ints[0] = *(gint *)value;
              break;
            case GTHREE_UNIFORM_TYPE_FLOAT:
              uniform->value.floats[0] = *(float *)value;
              break;
            case GTHREE_UNIFORM_TYPE_VECTOR2:
              uniform->value.floats[0] = ((float *)value)[0];
              uniform->value.floats[1] = ((float *)value)[1];
              break;
            case GTHREE_UNIFORM_TYPE_VECTOR3:
              uniform->value.floats[0] = ((float *)value)[0];
              uniform->value.floats[1] = ((float *)value)[1];
              uniform->value.floats[2] = ((float *)value)[2];
              break;
            case GTHREE_UNIFORM_TYPE_VECTOR4:
              uniform->value.floats[0] = ((float *)value)[0];
              uniform->value.floats[1] = ((float *)value)[1];
              uniform->value.floats[2] = ((float *)value)[2];
              uniform->value.floats[3] = ((float *)value)[3];
              break;
            case GTHREE_UNIFORM_TYPE_MATRIX3:
              uniform->value.more_floats = g_memdup (value, sizeof (float) * 9);
              break;
            default:
              g_error ("Unsupported type %d in uniform library\n", element->type);
              break;
            }
        }

      element += 1;
    }

  return uniforms;
}


static void
gthree_uniforms_init_libs ()
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  common = gthree_uniforms_new_from_definitions (common_lib, G_N_ELEMENTS (common_lib));
  specularmap = gthree_uniforms_new_from_definitions (specularmap_lib, G_N_ELEMENTS (specularmap_lib));
  envmap = gthree_uniforms_new_from_definitions (envmap_lib, G_N_ELEMENTS (envmap_lib));
  aomap = gthree_uniforms_new_from_definitions (aomap_lib, G_N_ELEMENTS (aomap_lib));
  lightmap = gthree_uniforms_new_from_definitions (lightmap_lib, G_N_ELEMENTS (lightmap_lib));
  emissivemap = gthree_uniforms_new_from_definitions (emissivemap_lib, G_N_ELEMENTS (emissivemap_lib));
  bumpmap = gthree_uniforms_new_from_definitions (bumpmap_lib, G_N_ELEMENTS (bumpmap_lib));
  normalmap = gthree_uniforms_new_from_definitions (normalmap_lib, G_N_ELEMENTS (normalmap_lib));
  displacementmap = gthree_uniforms_new_from_definitions (displacementmap_lib, G_N_ELEMENTS (displacementmap_lib));
  roughnessmap = gthree_uniforms_new_from_definitions (roughnessmap_lib, G_N_ELEMENTS (roughnessmap_lib));
  glossinessmap = gthree_uniforms_new_from_definitions (glossinessmap_lib, G_N_ELEMENTS (glossinessmap_lib));
  metalnessmap = gthree_uniforms_new_from_definitions (metalnessmap_lib, G_N_ELEMENTS (metalnessmap_lib));
  gradientmap = gthree_uniforms_new_from_definitions (gradientmap_lib, G_N_ELEMENTS (gradientmap_lib));
  fog = gthree_uniforms_new_from_definitions (fog_lib, G_N_ELEMENTS (fog_lib));
  lights = gthree_uniforms_new_from_definitions (lights_lib, G_N_ELEMENTS (lights_lib));
  points = gthree_uniforms_new_from_definitions (points_lib, G_N_ELEMENTS (points_lib));
  sprite = gthree_uniforms_new_from_definitions (sprite_lib, G_N_ELEMENTS (sprite_lib));

  initialized = TRUE;
}

GthreeUniforms *
gthree_get_uniforms_from_library (const char *name)
{
  gthree_uniforms_init_libs ();

  if (strcmp (name, "common") == 0)
    return common;

  if (strcmp (name, "specularmap") == 0)
    return specularmap;

  if (strcmp (name, "envmap") == 0)
    return envmap;

  if (strcmp (name, "aomap") == 0)
    return aomap;

  if (strcmp (name, "lightmap") == 0)
    return lightmap;

  if (strcmp (name, "emissivemap") == 0)
    return emissivemap;

  if (strcmp (name, "bumpmap") == 0)
    return bumpmap;

  if (strcmp (name, "normalmap") == 0)
    return normalmap;

  if (strcmp (name, "displacementmap") == 0)
    return displacementmap;

  if (strcmp (name, "roughnessmap") == 0)
    return roughnessmap;

  if (strcmp (name, "glossinessmap") == 0)
    return glossinessmap;

  if (strcmp (name, "metalnessmap") == 0)
    return metalnessmap;

  if (strcmp (name, "gradientmap") == 0)
    return gradientmap;

  if (strcmp (name, "fog") == 0)
    return fog;

  if (strcmp (name, "lights") == 0)
    return lights;

  if (strcmp (name, "points") == 0)
    return points;

  if (strcmp (name, "sprite") == 0)
    return sprite;

  g_warning ("can't find uniform library %s\n", name);
  return NULL;
}

