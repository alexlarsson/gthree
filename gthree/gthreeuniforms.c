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
        g_array_free (uniform->value.array, TRUE);
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE:
      g_clear_object (&uniform->value.texture);
      break;
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
      if (uniform->value.ptr_array)
        g_ptr_array_free (uniform->value.ptr_array, TRUE);
      break;
    case GTHREE_UNIFORM_TYPE_INT:
    case GTHREE_UNIFORM_TYPE_FLOAT:
    case GTHREE_UNIFORM_TYPE_FLOAT2:
    case GTHREE_UNIFORM_TYPE_FLOAT3:
    case GTHREE_UNIFORM_TYPE_FLOAT4:
    case GTHREE_UNIFORM_TYPE_VECTOR2:
    case GTHREE_UNIFORM_TYPE_VECTOR3:
    case GTHREE_UNIFORM_TYPE_VECTOR4:
    case GTHREE_UNIFORM_TYPE_COLOR:
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

void
gthree_uniform_set_needs_update (GthreeUniform *uniform,
				 gboolean needs_update)
{
  uniform->needs_update = needs_update;
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
          guint len = uniform->value.ptr_array->len;
          clone->value.ptr_array = g_ptr_array_sized_new (len);
          // TODO: ref? duplicate?
          // TODO: Copy free func?
          memcpy (clone->value.ptr_array->pdata, uniform->value.ptr_array->pdata, len * sizeof (gpointer));
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
    case GTHREE_UNIFORM_TYPE_COLOR:
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

void
gthree_uniform_set_int (GthreeUniform *uniform,
                        int val)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_INT);
  uniform->value.ints[0] = val;
}

void
gthree_uniform_set_color (GthreeUniform *uniform,
                          GdkRGBA *val)
{
  g_return_if_fail (uniform->type == GTHREE_UNIFORM_TYPE_COLOR);
  uniform->value.floats[0] = val->red;
  uniform->value.floats[1] = val->green;
  uniform->value.floats[2] = val->blue;
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
    case GTHREE_UNIFORM_TYPE_COLOR:
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
	glUniform2fv (uniform->location, uniform->value.array->len, &g_array_index (uniform->value.array, float, 0));
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY:
      if (uniform->value.array)
	glUniform3fv (uniform->location, uniform->value.array->len, &g_array_index (uniform->value.array, float, 0));
      break;
    case GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY:
      if (uniform->value.array)
	glUniform4fv (uniform->location, uniform->value.array->len, &g_array_index (uniform->value.array, float, 0));
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
        gthree_texture_load (uniform->value.texture, gthree_renderer_allocate_texture_unit (renderer));
      break;
    case GTHREE_UNIFORM_TYPE_VEC2_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC3_ARRAY:
    case GTHREE_UNIFORM_TYPE_VEC4_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX3_ARRAY:
    case GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY:
    case GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY:
      g_warning ("gthree_uniform_load() - unsupported uniform type %d\n", uniform->type);
    }
}

static int i0 = 0;
static float f1 = 1.0;
static float fm1 = -1.0;
static float f2000 = 2000;
static float fp98 = 0.98;
static float fp00025 = 0.00025;
static GdkRGBA grey = { 0.9333333333333333, 0.9333333333333333, 0.9333333333333333, 1.0 };
static GdkRGBA white = { 1, 1, 1, 1.0 };
static float default_offset_repeat[4] = { 0, 0, 1, 1 };
static float onev2[2] = { 1, 1 };

static GthreeUniforms *common;
static GthreeUniformsDefinition common_lib[] = {
  {"diffuse", GTHREE_UNIFORM_TYPE_COLOR, &grey },
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },

  {"map", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"offsetRepeat", GTHREE_UNIFORM_TYPE_VECTOR4, &default_offset_repeat},

  {"lightMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"specularMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"alphaMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },

  {"envMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"flipEnvMap", GTHREE_UNIFORM_TYPE_FLOAT, &fm1 },
  {"useRefract", GTHREE_UNIFORM_TYPE_INT, &i0 },
  {"reflectivity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"refractionRatio", GTHREE_UNIFORM_TYPE_FLOAT, &fp98 },
  {"combine", GTHREE_UNIFORM_TYPE_INT, &i0 },

  {"morphTargetInfluences", GTHREE_UNIFORM_TYPE_FLOAT, 0 }
};

static GthreeUniforms *bump;
static GthreeUniformsDefinition bump_lib[] = {
  {"bumpMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"bumpScale", GTHREE_UNIFORM_TYPE_FLOAT, &f1 }
};

static GthreeUniforms *normalmap;
static GthreeUniformsDefinition normalmap_lib[] = {
  {"normalMap", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
  {"normalScale", GTHREE_UNIFORM_TYPE_VECTOR2, &onev2}
};

static GthreeUniforms *fog;
static GthreeUniformsDefinition fog_lib[] = {
  {"fogDensity", GTHREE_UNIFORM_TYPE_FLOAT, &fp00025 },
  {"fogNear", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"fogFar", GTHREE_UNIFORM_TYPE_FLOAT, &f2000 },
  {"fogColor", GTHREE_UNIFORM_TYPE_COLOR, &white }
};

static GthreeUniforms *lights;
static GthreeUniformsDefinition lights_lib[] = {
  {"ambientLightColor", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},

  {"directionalLightDirection", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"directionalLightColor", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},

  {"hemisphereLightDirection", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"hemisphereLightSkyColor", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"hemisphereLightGroundColor", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},

  {"pointLightColor", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"pointLightPosition", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"pointLightDistance", GTHREE_UNIFORM_TYPE_FLOAT_ARRAY, NULL},

  {"spotLightColor", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"spotLightPosition", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"spotLightDirection", GTHREE_UNIFORM_TYPE_FLOAT3_ARRAY, NULL},
  {"spotLightDistance", GTHREE_UNIFORM_TYPE_FLOAT_ARRAY, NULL},
  {"spotLightAngleCos", GTHREE_UNIFORM_TYPE_FLOAT_ARRAY, NULL},
  {"spotLightExponent", GTHREE_UNIFORM_TYPE_FLOAT_ARRAY, NULL}
};

static GthreeUniforms *particle;
static GthreeUniformsDefinition particle_lib[] = {
  {"psColor", GTHREE_UNIFORM_TYPE_COLOR, &grey },
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"size", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"scale", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"map", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },

  {"fogDensity", GTHREE_UNIFORM_TYPE_FLOAT, &fp00025 },
  {"fogNear", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"fogFar", GTHREE_UNIFORM_TYPE_FLOAT, &f2000 },
  {"fogColor", GTHREE_UNIFORM_TYPE_COLOR, &white}
};

static GthreeUniforms *shadowmap;
static GthreeUniformsDefinition shadowmap_lib[] = {
  {"shadowMap", GTHREE_UNIFORM_TYPE_TEXTURE_ARRAY, NULL},
  {"shadowMapSize", GTHREE_UNIFORM_TYPE_VEC2_ARRAY, NULL},

  {"shadowBias", GTHREE_UNIFORM_TYPE_FLOAT_ARRAY, NULL},
  {"shadowDarkness", GTHREE_UNIFORM_TYPE_FLOAT_ARRAY, NULL},

  {"shadowMatrix", GTHREE_UNIFORM_TYPE_MATRIX4_ARRAY, NULL}
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
            case GTHREE_UNIFORM_TYPE_COLOR:
              uniform->value.floats[0] = ((GdkRGBA *)value)->red;
              uniform->value.floats[1] = ((GdkRGBA *)value)->green;
              uniform->value.floats[2] = ((GdkRGBA *)value)->blue;
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
  bump = gthree_uniforms_new_from_definitions (bump_lib, G_N_ELEMENTS (bump_lib));
  normalmap = gthree_uniforms_new_from_definitions (normalmap_lib, G_N_ELEMENTS (normalmap_lib));
  fog = gthree_uniforms_new_from_definitions (fog_lib, G_N_ELEMENTS (fog_lib));
  lights = gthree_uniforms_new_from_definitions (lights_lib, G_N_ELEMENTS (lights_lib));
  particle = gthree_uniforms_new_from_definitions (particle_lib, G_N_ELEMENTS (particle_lib));
  shadowmap = gthree_uniforms_new_from_definitions (shadowmap_lib, G_N_ELEMENTS (shadowmap_lib));

  initialized = TRUE;
}

GthreeUniforms *
gthree_get_uniforms_from_library (const char *name)
{
  gthree_uniforms_init_libs ();

  if (strcmp (name, "common") == 0)
    return common;
  if (strcmp (name, "bump") == 0)
    return bump;
  if (strcmp (name, "normalmap") == 0)
    return normalmap;
  if (strcmp (name, "fog") == 0)
    return fog;
  if (strcmp (name, "lights") == 0)
    return lights;
  if (strcmp (name, "particle") == 0)
    return particle;
  if (strcmp (name, "shadowmap") == 0)
    return shadowmap;

  g_warning ("can't find uniform library %s\n", name);
  return NULL;
}
