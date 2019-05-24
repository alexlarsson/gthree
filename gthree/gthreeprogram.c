#include <math.h>
#include <epoxy/gl.h>

#include "gthreeprogram.h"
#include "gthreeuniforms.h"
#include "gthreeshader.h"

typedef struct {
  GHashTable *uniform_locations;
  GHashTable *attribute_locations;

  GLuint gl_program;

  /* Cache keys: */
  GthreeProgramCache *cache;
  GthreeShader *shader;
  GthreeProgramParameters params;
} GthreeProgramPrivate;

struct _GthreeProgramCache
{
    GHashTable *hash;
};

static void gthree_program_cache_remove (GthreeProgramCache *cache, GthreeProgram *program);

G_DEFINE_TYPE_WITH_PRIVATE (GthreeProgram, gthree_program, G_TYPE_OBJECT);

const char *
precision_to_string (GthreePrecision prec)
{
  if (prec == GTHREE_PRECISION_LOW)
    return "lowp";
  if (prec == GTHREE_PRECISION_MEDIUM)
    return "mediump";
  return "highp";
}

static const char *
get_vertex_type_name (int type)
{
  switch (type)
    {
    case GL_VERTEX_SHADER:
      return "vertex";
    case GL_GEOMETRY_SHADER:
      return "geometry";
    case GL_FRAGMENT_SHADER:
      return "fragment";
    }
  return "unknown";
}

GLuint
create_shader (int type, const char *code)
{
  GLuint shader = glCreateShader (type);
  GLint status;

  glShaderSource (shader, 1, &code, NULL);
  glCompileShader (shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
    {
      GLint log_len;
      char *buffer;

      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_len);

      buffer = g_malloc (log_len + 1);

      glGetShaderInfoLog (shader, log_len, NULL, buffer);

      g_warning ("Compile failure in %s shader:\n%s\n", get_vertex_type_name (type), buffer);

      g_free (buffer);
    }

  return shader;
}

static void
cache_uniform_locations (GHashTable *uniforms, GLuint program, char **identifiers)
{
  int i;

  for (i = 0; identifiers[i] != NULL; i++)
    {
      int location = glGetUniformLocation (program, identifiers[i]);
      g_hash_table_insert (uniforms, GINT_TO_POINTER (g_quark_from_string (identifiers[i])), GINT_TO_POINTER (location));
    }
}

static void
generate_defines (GString *out, GPtrArray *defines)
{
  int i;

  for (i = 0; i + 1 < defines->len; i += 2)
    {
      char *key = g_ptr_array_index (defines, i);
      char *value = g_ptr_array_index (defines, i + 1);

      if (value == NULL)
        continue;

      g_string_append_printf (out, "#define %s %s\n", key, value);
    }
}

static void
cache_attribute_location (GHashTable *attributes, GLuint program, char *identifier)
{
  int location = glGetAttribLocation (program, identifier);
  g_hash_table_insert (attributes, GINT_TO_POINTER (g_quark_from_string (identifier)), GINT_TO_POINTER (location));
}

GthreeProgram *
gthree_program_new (GthreeShader *shader, GthreeProgramParameters *parameters)
{
  GthreeProgram *program;
  GthreeProgramPrivate *priv;
  GPtrArray *defines;
  GthreeUniforms *uniforms;
  const char *vertex_shader, *fragment_shader;
  char *index0AttributeName;
  //char *shadowMapTypeDefine;
  GLuint gl_program;
  GString *vertex, *fragment;
  GLuint glVertexShader, glFragmentShader;
  GLint status;
  GPtrArray *identifiers;
  GList *uniform_list, *l;

  program = g_object_new (gthree_program_get_type (),
                          NULL);
  priv = gthree_program_get_instance_private (program);

  priv->shader = g_object_ref (shader);
  memcpy (&priv->params, parameters, sizeof (GthreeProgramParameters));

  //var attributes = material.attributes;
  defines = gthree_shader_get_defines (shader);
  uniforms = gthree_shader_get_uniforms (shader);
  vertex_shader = gthree_shader_get_vertex_shader_text (shader);
  fragment_shader = gthree_shader_get_fragment_shader_text (shader);

  index0AttributeName = NULL;
  //index0AttributeName = material.index0AttributeName;

#if TODO
  if ( index0AttributeName == NULL && parameters.morphTargets === true ) {
    // programs with morphTargets displace position out of attribute 0
    index0AttributeName = "position";
  }
#endif

  //shadowMapTypeDefine = "SHADOWMAP_TYPE_BASIC";

#if TODO
  if (parameters.shadowMapType === THREE.PCFShadowMap) {
    shadowMapTypeDefine = "SHADOWMAP_TYPE_PCF";
  } else if (parameters.shadowMapType === THREE.PCFSoftShadowMap) {
    shadowMapTypeDefine = "SHADOWMAP_TYPE_PCF_SOFT";
  }
#endif

  // console.log( "building new program " );

  //


  gl_program = glCreateProgram ();


  vertex = g_string_new ("");
  fragment = g_string_new ("");

  if (TRUE /*! material instanceof THREE.RawShaderMaterial */)
    {
      g_string_append (vertex, "#version 130\n");
      g_string_append_printf (vertex, "precision %s float;\n", precision_to_string (parameters->precision));
      g_string_append_printf (vertex, "precision %s int;\n", precision_to_string (parameters->precision));

      if (defines)
        generate_defines (vertex, defines);

      if (parameters->supports_vertex_textures)
        g_string_append (vertex, "#define VERTEX_TEXTURES\n");

      //_this.gammaInput ? "#define GAMMA_INPUT" : "",
      //_this.gammaOutput ? "#define GAMMA_OUTPUT" : "",

      g_string_append_printf (vertex,
                              "#define MAX_DIR_LIGHTS %d\n"
                              "#define MAX_POINT_LIGHTS %d\n"
                              "#define MAX_SPOT_LIGHTS %d\n"
                              "#define MAX_HEMI_LIGHTS %d\n",
                              parameters->max_dir_lights,
                              parameters->max_point_lights,
                              parameters->max_spot_lights,
                              parameters->max_hemi_lights);

      g_string_append_printf (vertex,
                              "#define MAX_SHADOWS %d\n"
                              "#define MAX_BONES %d\n",
                              parameters->max_shadows,
                              parameters->max_bones);

      if (parameters->map)
        g_string_append (vertex, "#define USE_MAP\n");
      if (parameters->env_map)
        g_string_append (vertex, "#define USE_ENVMAP\n");
      if (parameters->light_map)
        g_string_append (vertex, "#define USE_LIGHTMAP\n");
      if (parameters->bump_map)
        g_string_append (vertex, "#define USE_BUMPMAP\n");
      if (parameters->vertex_colors)
        g_string_append (vertex, "#define USE_COLOR\n");

      if (parameters->normal_map)
        g_string_append (vertex, "#define USE_NORMALMAP\n");
      if (parameters->specular_map)
        g_string_append (vertex, "#define USE_SPECULARMAP\n");
      if (parameters->alpha_map)
        g_string_append (vertex, "#define USE_ALPHAMAP\n");

      if (parameters->wrap_around)
        g_string_append (vertex, "#define WRAP_AROUND\n");

#if TODO
        parameters.skinning ? "#define USE_SKINNING" : "",
        parameters.useVertexTexture ? "#define BONE_TEXTURE" : "",

        parameters.morphTargets ? "#define USE_MORPHTARGETS" : "",
        parameters.morphNormals ? "#define USE_MORPHNORMALS" : "",
#endif

      if (parameters->double_sided)
        g_string_append (vertex, "#define DOUBLE_SIDED\n");
      if (parameters->flip_sided)
        g_string_append (vertex, "#define FLIP_SIDED\n");

#ifdef TODO
      parameters.shadowMapEnabled ? "#define USE_SHADOWMAP" : "",
        parameters.shadowMapEnabled ? "#define " + shadowMapTypeDefine : "",
        parameters.shadowMapDebug ? "#define SHADOWMAP_DEBUG" : "",
        parameters.shadowMapCascade ? "#define SHADOWMAP_CASCADE" : "",

        parameters.sizeAttenuation ? "#define USE_SIZEATTENUATION" : "",

        parameters.logarithmicDepthBuffer ? "#define USE_LOGDEPTHBUF" : "",
        //_this._glExtensionFragDepth ? "#define USE_LOGDEPTHBUF_EXT" : "",
#endif

        g_string_append (vertex,
                         "uniform mat4 modelMatrix;\n"
                         "uniform mat4 modelViewMatrix;\n"
                         "uniform mat4 projectionMatrix;\n"
                         "uniform mat4 viewMatrix;\n"
                         "uniform mat3 normalMatrix;\n"
                         "uniform vec3 cameraPosition;\n"

                         "attribute vec3 position;\n"
                         "attribute vec3 normal;\n"
                         "attribute vec2 uv;\n"
                         "attribute vec2 uv2;\n"

                         "#ifdef USE_COLOR\n"
                         "	attribute vec3 color;\n"
                         "#endif\n"

                         "#ifdef USE_MORPHTARGETS\n"
                         "	attribute vec3 morphTarget0;\n"
                         "	attribute vec3 morphTarget1;\n"
                         "	attribute vec3 morphTarget2;\n"
                         "	attribute vec3 morphTarget3;\n"

                         "	#ifdef USE_MORPHNORMALS\n"
                         "		attribute vec3 morphNormal0;\n"
                         "		attribute vec3 morphNormal1;\n"
                         "		attribute vec3 morphNormal2;\n"
                         "		attribute vec3 morphNormal3;\n"
                         "	#else\n"
                         "		attribute vec3 morphTarget4;\n"
                         "		attribute vec3 morphTarget5;\n"
                         "		attribute vec3 morphTarget6;\n"
                         "		attribute vec3 morphTarget7;\n"
                         "	#endif\n"
                         "#endif\n"
                         "#ifdef USE_SKINNING\n"
                         "	attribute vec4 skinIndex;\n"
                         "	attribute vec4 skinWeight;\n"
                         "#endif\n");

      /* fragment shader prefix */

      g_string_append (fragment, "#version 130\n");
      g_string_append_printf (fragment, "precision %s float;\n", precision_to_string (parameters->precision));
      g_string_append_printf (fragment, "precision %s int;\n", precision_to_string (parameters->precision));

      //TODO      ( parameters.bumpMap || parameters.normalMap ) ? "#extension GL_OES_standard_derivatives : enable" : "",

      if (defines)
        generate_defines (fragment, defines);

      g_string_append_printf (fragment,
                              "#define MAX_DIR_LIGHTS %d\n"
                              "#define MAX_POINT_LIGHTS %d\n"
                              "#define MAX_SPOT_LIGHTS %d\n"
                              "#define MAX_HEMI_LIGHTS %d\n",
                              parameters->max_dir_lights,
                              parameters->max_point_lights,
                              parameters->max_spot_lights,
                              parameters->max_hemi_lights);

      g_string_append_printf (vertex,
                              "#define MAX_SHADOWS %d\n",
                              parameters->max_shadows);

      if (parameters->alpha_test != 0)
        g_string_append_printf (vertex,
                                "#define ALPHATEST %f\n",
                                parameters->alpha_test);

      //_this.gammaInput ? "#define GAMMA_INPUT" : "",
      //_this.gammaOutput ? "#define GAMMA_OUTPUT" : "",

      if (parameters->use_fog && parameters->fog)
        g_string_append (fragment, "#define USE_FOG\n");
      if (parameters->use_fog && parameters->fog_exp)
        g_string_append (fragment, "#define FOG_EXP2\n");

      if (parameters->map)
        g_string_append (fragment, "#define USE_MAP\n");
      if (parameters->env_map)
        g_string_append (fragment, "#define USE_ENVMAP\n");
      if (parameters->light_map)
        g_string_append (fragment, "#define USE_LIGHTMAP\n");
      if (parameters->bump_map)
        g_string_append (fragment, "#define USE_BUMPMAP\n");
      if (parameters->normal_map)
        g_string_append (fragment, "#define USE_NORMALMAP\n");
      if (parameters->specular_map)
        g_string_append (fragment, "#define USE_SPECULARMAP\n");
      if (parameters->alpha_map)
        g_string_append (fragment, "#define USE_ALPHAMAP\n");

      if (parameters->vertex_colors)
        g_string_append (fragment, "#define USE_COLOR\n");

      if (parameters->metal)
        g_string_append (fragment, "#define METAL\n");
      if (parameters->wrap_around)
        g_string_append (fragment, "#define WRAP_AROUND\n");
      if (parameters->double_sided)
        g_string_append (fragment, "#define DOUBLE_SIDED\n");
      if (parameters->flip_sided)
        g_string_append (fragment, "#define FLIP_SIDED\n");

#ifdef TODO
      parameters.shadowMapEnabled ? "#define USE_SHADOWMAP" : "",
        parameters.shadowMapEnabled ? "#define " + shadowMapTypeDefine : "",
        parameters.shadowMapDebug ? "#define SHADOWMAP_DEBUG" : "",
        parameters.shadowMapCascade ? "#define SHADOWMAP_CASCADE" : "",

        parameters.logarithmicDepthBuffer ? "#define USE_LOGDEPTHBUF" : "",
        //_this._glExtensionFragDepth ? "#define USE_LOGDEPTHBUF_EXT" : "",
#endif
        g_string_append (fragment,
                         "uniform mat4 viewMatrix;\n"
                         "uniform vec3 cameraPosition;\n");
  }

  g_string_append (vertex, vertex_shader);
  g_string_append (fragment, fragment_shader);

  if (0)
    {
      g_print ("************ VERTEX *******************************************************\n%s\n",
               vertex->str);
      g_print ("************ FRAGMENT *******************************************************\n%s\n",
               fragment->str);
    }

  glVertexShader = create_shader (GL_VERTEX_SHADER, vertex->str);
  glFragmentShader = create_shader (GL_FRAGMENT_SHADER, fragment->str);

  g_string_free (vertex, TRUE);
  g_string_free (fragment, TRUE);

  glAttachShader (gl_program, glVertexShader);
  glAttachShader (gl_program, glFragmentShader);

  if (index0AttributeName != NULL) {

    // Force a particular attribute to index 0.
    // because potentially expensive emulation is done by browser if attribute 0 is disabled.
    // And, color, for example is often automatically bound to index 0 so disabling it

    glBindAttribLocation (gl_program, 0, index0AttributeName);
  }

  glLinkProgram (gl_program);

  glGetProgramiv (gl_program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
    {
      GLint log_len;
      char *buffer;

      glGetProgramiv (gl_program, GL_INFO_LOG_LENGTH, &log_len);

      buffer = g_malloc (log_len + 1);
      glGetProgramInfoLog (gl_program, log_len, NULL, buffer);
      g_warning ("Linker failure: %s\n", buffer);
      g_free (buffer);
    }

  // clean up

  glDeleteShader (glVertexShader);
  glDeleteShader (glFragmentShader);

  // cache uniform locations

  identifiers = g_ptr_array_new ();

  {
    int i;
    char *s[] =  {
      "viewMatrix", "modelViewMatrix", "projectionMatrix", "normalMatrix",
      "modelMatrix", "cameraPosition", "morphTargetInfluences", "bindMatrix",
      "bindMatrixInverse"
    };

    for (i = 0; i < G_N_ELEMENTS (s); i++)
      g_ptr_array_add (identifiers, s[i]);
  }

  if (parameters->use_vertex_texture)
    {
      g_ptr_array_add (identifiers, "boneTexture");
      g_ptr_array_add (identifiers, "boneTextureWidth");
      g_ptr_array_add (identifiers, "boneTextureHeight");
    }
  else
    {
      g_ptr_array_add (identifiers, "boneGlobalMatrices");
    }

#if TODO
  if ( parameters.logarithmicDepthBuffer ) {
    identifiers.push('logDepthBufFC');
  }
#endif

  uniform_list = gthree_uniforms_get_all (uniforms);

  for (l = uniform_list; l != NULL; l = l->next)
    g_ptr_array_add (identifiers, (char *)gthree_uniform_get_name (l->data));

  g_list_free (uniform_list);
  g_ptr_array_add (identifiers, NULL);

  cache_uniform_locations (priv->uniform_locations, gl_program, (char **)identifiers->pdata);

  g_ptr_array_free (identifiers, FALSE);

  priv->gl_program = gl_program;

  return program;
}

static void
gthree_program_init (GthreeProgram *program)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

  priv->uniform_locations = g_hash_table_new (g_direct_hash, g_direct_equal);
}

static void
gthree_program_finalize (GObject *obj)
{
  GthreeProgram *program = GTHREE_PROGRAM (obj);
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

  if (priv->gl_program)
    {
      glDeleteProgram (priv->gl_program);
      priv->gl_program = 0;
    }

  g_hash_table_destroy (priv->uniform_locations);
  g_hash_table_destroy (priv->attribute_locations);

  if (priv->cache)
    gthree_program_cache_remove (priv->cache, program);

  g_clear_object (&priv->shader);

  G_OBJECT_CLASS (gthree_program_parent_class)->finalize (obj);
}

static void
gthree_program_class_init (GthreeProgramClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_program_finalize;
}

void
gthree_program_use (GthreeProgram *program)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

  glUseProgram (priv->gl_program);
}

gint
gthree_program_lookup_uniform_location (GthreeProgram *program,
                                        GQuark uniform)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);
  gpointer location;

  if (g_hash_table_lookup_extended (priv->uniform_locations,
                                    GINT_TO_POINTER (uniform), NULL, &location))
    return GPOINTER_TO_INT (location);
  return -1;
}

gint
gthree_program_lookup_uniform_location_from_string (GthreeProgram *program,
                                                    const char *uniform)
{
  return gthree_program_lookup_uniform_location (program, g_quark_from_string (uniform));
}

gint
gthree_program_lookup_attribute_location (GthreeProgram *program,
                                          GQuark attribute)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);
  gpointer location;

  if (g_hash_table_lookup_extended (priv->attribute_locations,
                                    GINT_TO_POINTER (attribute), NULL, &location))
    return GPOINTER_TO_INT (location);
  return -1;
}

gint
gthree_program_lookup_attribute_location_from_string (GthreeProgram *program,
                                                      const char *attribute)
{
  return gthree_program_lookup_attribute_location (program,
                                                   g_quark_from_string (attribute));
}

GHashTable *
gthree_program_get_attribute_locations (GthreeProgram *program)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

  if (priv->attribute_locations == NULL)
    {
      GLint n, i, max_len;
      char *buffer;

      priv->attribute_locations = g_hash_table_new (g_direct_hash, g_direct_equal);

      glGetProgramiv (priv->gl_program,  GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,  &max_len);
      buffer = g_alloca (max_len + 1);

      glGetProgramiv (priv->gl_program,  GL_ACTIVE_ATTRIBUTES,  &n);
      for (i = 0; i < n; i++)
        {
          GLint size;
          GLenum type;

          glGetActiveAttrib (priv->gl_program,  i,  max_len,  NULL,  &size,  &type,  buffer);
          cache_attribute_location (priv->attribute_locations, priv->gl_program, buffer);
        }
    }

  return priv->attribute_locations;
}

static guint
gthree_program_parameters_hash (GthreeProgramParameters *params)
{
  guint32 *ptr = (guint32 *)params;
  int i, len = sizeof (GthreeProgramParameters) / 4;
  guint32 h = 0;

  for (i = 0; i < len; i++)
    h ^= ptr[i];

  return h;
}

static gboolean
gthree_program_parameters_equal (GthreeProgramParameters *a, GthreeProgramParameters *b)
{
  return memcmp (a, b, sizeof (GthreeProgramParameters)) == 0;
}

static guint
gthree_program_priv_hash (GthreeProgramPrivate *priv)
{
  return gthree_shader_hash (priv->shader) ^
    gthree_program_parameters_hash (&priv->params);
}

static gboolean
gthree_program_priv_equal (GthreeProgramPrivate *a,
                           GthreeProgramPrivate *b)
{
  return gthree_shader_equal (a->shader, b->shader) &&
    gthree_program_parameters_equal (&a->params, &b->params);
}

GthreeProgramCache *
gthree_program_cache_new (void)
{
  GthreeProgramCache *cache;

  cache = g_new0 (GthreeProgramCache, 1);

  cache->hash = g_hash_table_new ((GHashFunc)gthree_program_priv_hash, (GEqualFunc)gthree_program_priv_equal);

  return cache;
}

static void
gthree_program_cache_remove (GthreeProgramCache *cache, GthreeProgram *program)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);
  g_hash_table_remove (cache->hash, gthree_program_get_instance_private (program));
  priv->cache = NULL;
}

GthreeProgram *
gthree_program_cache_get (GthreeProgramCache *cache, GthreeShader *shader, GthreeProgramParameters *parameters)
{
  GthreeProgramPrivate *priv;
  GthreeProgramPrivate key = {NULL};
  GthreeProgram *program;

  key.shader = shader;
  memcpy (&key.params, parameters, sizeof (GthreeProgramParameters));

  program = g_hash_table_lookup (cache->hash, &key);
  if (program)
    return g_object_ref (program);

  program = gthree_program_new (shader, parameters);
  priv = gthree_program_get_instance_private (program);
  priv->cache = cache;

  g_hash_table_insert (cache->hash, gthree_program_get_instance_private (program), program);

  return program;
}

void
gthree_program_cache_free (GthreeProgramCache *cache)
{
  GthreeProgramPrivate *priv;
  GHashTableIter iter;
  gpointer value;
  GthreeProgram *program;

  g_hash_table_iter_init (&iter, cache->hash);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      program = value;
      priv = gthree_program_get_instance_private (program);
      priv->cache = NULL;
    }

  g_hash_table_destroy (cache->hash);
  g_free (cache);
}
