#include <math.h>
#include <epoxy/gl.h>

#include "gthreeprogram.h"
#include "gthreeuniforms.h"
#include "gthreeshader.h"
#include "gthreematerial.h"

typedef struct {
  int dummy;
} GthreeProgramPrivate;


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
create_shader (int type, const char const *code)
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

static GHashTable *
cache_uniform_locations (GLuint program, char **identifiers) {
  GHashTable *uniforms = g_hash_table_new (g_str_hash, g_str_equal);
  int i;

  for (i = 0; identifiers[i] != NULL; i++)
    {
      int location = glGetUniformLocation (program, identifiers[i]);
      g_hash_table_insert (uniforms, identifiers[i], GINT_TO_POINTER (location));
    }

  return uniforms;
}

static GHashTable *
cache_attribute_locations (GLuint program, char **identifiers) {
  GHashTable *attributes = g_hash_table_new (g_str_hash, g_str_equal);
  int i;

  for (i = 0; identifiers[i] != NULL; i++)
    {
      int location = glGetAttribLocation (program, identifiers[i]);
      g_hash_table_insert (attributes, identifiers[i], GINT_TO_POINTER (location));
    }

  return attributes;
}

GthreeProgram *
gthree_program_new (gpointer code, GthreeMaterial *material, GthreeProgramParameters *parameters)
{
  GthreeProgram *program;
  char **defines;
  GthreeUniforms *uniforms;
  const char *vertex_shader, *fragment_shader;
  char *index0AttributeName;
  char *shadowMapTypeDefine;
  GLuint gl_program;
  GString *vertex, *fragment;
  GLuint glVertexShader, glFragmentShader;
  GLint status;
  GPtrArray *identifiers;
  GList *uniform_list, *l;

  program = g_object_new (gthree_program_get_type (),
                          NULL);

  defines = NULL;
  //var defines = material.defines;
  //var attributes = material.attributes;
  uniforms = material->shader->uniforms;
  vertex_shader = material->shader->vertex_shader;
  fragment_shader = material->shader->fragment_shader;

  index0AttributeName = NULL;
  //index0AttributeName = material.index0AttributeName;

#if TODO
  if ( index0AttributeName == NULL && parameters.morphTargets === true ) {
    // programs with morphTargets displace position out of attribute 0
    index0AttributeName = "position";
  }
#endif

  shadowMapTypeDefine = "SHADOWMAP_TYPE_BASIC";

#if TODO
  if (parameters.shadowMapType === THREE.PCFShadowMap) {
    shadowMapTypeDefine = "SHADOWMAP_TYPE_PCF";
  } else if (parameters.shadowMapType === THREE.PCFSoftShadowMap) {
    shadowMapTypeDefine = "SHADOWMAP_TYPE_PCF_SOFT";
  }
#endif

  // console.log( "building new program " );

  //

  //var customDefines = generateDefines( defines );

  //

  gl_program = glCreateProgram ();


  vertex = g_string_new ("");
  fragment = g_string_new ("");

  if (TRUE /*! material instanceof THREE.RawShaderMaterial */)
    {
      g_string_append (vertex, "#version 120\n");
      //g_string_append_printf (vertex, "precision %s float;\n", precision_to_string (parameters->precision));
      //g_string_append_printf (vertex, "precision %s int;\n", precision_to_string (parameters->precision));

      //customDefines,

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
      if (parameters->vertex_colors != GTHREE_COLOR_NONE)
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

      g_string_append (fragment, "#version 120\n");
      //g_string_append_printf (fragment, "precision %s float;\n", precision_to_string (parameters->precision));
      //g_string_append_printf (fragment, "precision %s int;\n", precision_to_string (parameters->precision));

      //TODO      ( parameters.bumpMap || parameters.normalMap ) ? "#extension GL_OES_standard_derivatives : enable" : "",

      //customDefines,

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

      //parameters.alphaTest ? "#define ALPHATEST " + parameters.alphaTest: "",

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

      if (parameters->vertex_colors != GTHREE_COLOR_NONE)
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

  program->uniform_locations = cache_uniform_locations (gl_program, (char **)identifiers->pdata);

  g_ptr_array_free (identifiers, FALSE);

  // cache attributes locations

  identifiers = g_ptr_array_new ();

  {
    int i;
    char *s[] =  {
      "position", "normal", "uv", "uv2", "tangent", "color",
      "skinIndex", "skinWeight", "lineDistance"
    };

    for (i = 0; i < G_N_ELEMENTS (s); i++)
      g_ptr_array_add (identifiers, s[i]);
  }

#ifdef TODO
  for ( var i = 0; i < parameters.maxMorphTargets; i ++ ) {
    identifiers.push( "morphTarget" + i );
  }

  for ( var i = 0; i < parameters.maxMorphNormals; i ++ ) {

    identifiers.push( "morphNormal" + i );

  }

  for ( var a in attributes ) {
    identifiers.push( a );
  }
#endif

  g_ptr_array_add (identifiers, NULL);

  program->attribute_locations = cache_attribute_locations (gl_program, (char **)identifiers->pdata);

  g_ptr_array_free (identifiers, FALSE);

  program->code = code;
  program->usedTimes = 1;
  program->gl_program = gl_program;

  return program;
}

static void
gthree_program_init (GthreeProgram *program)
{
  //GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

}

static void
gthree_program_finalize (GObject *obj)
{
  GthreeProgram *program = GTHREE_PROGRAM (obj);
  //GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);

  if (program->gl_program)
    {
      glDeleteProgram (program->gl_program);
      program->gl_program = 0;
    }

  G_OBJECT_CLASS (gthree_program_parent_class)->finalize (obj);
}

static void
gthree_program_class_init (GthreeProgramClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_program_finalize;
}

guint
gthree_program_get_program (GthreeProgram *program)
{
  return program->gl_program;
}

gint
gthree_program_lookup_uniform_location (GthreeProgram *program,
                                        const char *uniform)
{
  gpointer location;

  if (g_hash_table_lookup_extended (program->uniform_locations,
                                    uniform, NULL, &location))
    return GPOINTER_TO_INT (location);
  return -1;
}

gint
gthree_program_lookup_attribute_location (GthreeProgram *program,
                                             const char *attribute)
{
  gpointer location;

  if (g_hash_table_lookup_extended (program->attribute_locations,
                                    attribute, NULL, &location))
    return GPOINTER_TO_INT (location);
  return -1;
}
