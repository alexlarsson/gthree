#include <math.h>
#include <epoxy/gl.h>

#include "gthreeprogram.h"
#include "gthreeuniforms.h"
#include "gthreeshader.h"
#include "gthreerenderer.h"
#include "gthreeprivate.h"

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
get_uniform_locations (GthreeProgram *program)
{
  GthreeProgramPrivate *priv = gthree_program_get_instance_private (program);
  GLint n, i, max_len;
  char *buffer;

  priv->uniform_locations = g_hash_table_new (g_direct_hash, g_direct_equal);

  glGetProgramiv (priv->gl_program,  GL_ACTIVE_UNIFORM_MAX_LENGTH,  &max_len);
  buffer = g_alloca (max_len + 1);

  n = 0;
  glGetProgramiv (priv->gl_program,  GL_ACTIVE_UNIFORMS,  &n);
  for (i = 0; i < n; i++)
    {
      GLint size;
      GLenum type;
      int location;

      glGetActiveUniform(priv->gl_program,  i,  max_len,  NULL,
                         &size, &type, buffer);

      location = glGetUniformLocation (priv->gl_program, buffer);
      g_hash_table_insert (priv->uniform_locations,
                           GINT_TO_POINTER (g_quark_from_string (buffer)), GINT_TO_POINTER (location));
    }
}

static void
string_replace (GString *string,
                const gchar *find,
                const gchar *replace)
{
  const gchar *at;
  gssize pos;

  at = strstr (string->str, find);
  while (at != NULL)
    {
      pos = at - string->str;
      g_string_erase (string, pos, strlen (find));
      g_string_insert (string, pos, replace);

      at = strstr (string->str, find);
    }
}

static void
string_replace_i (GString *string,
                  const gchar *find,
                  int i)
{
  g_autofree char *str = g_strdup_printf ("%d", i);
  string_replace (string, find, str);
}


static void
replace_light_nums (GString *str, GthreeProgramParameters *parameters)
{
  /* Order matters: longer patterns must be replaced before shorter
     ones that are prefixes (e.g. NUM_SPOT_LIGHT_COORDS before NUM_SPOT_LIGHTS) */
  string_replace_i (str, "NUM_DIR_LIGHT_SHADOWS", 0);
  string_replace_i (str, "NUM_DIR_LIGHTS", parameters->num_dir_lights);
  string_replace_i (str, "NUM_SPOT_LIGHT_SHADOWS_WITH_MAPS", 0);
  string_replace_i (str, "NUM_SPOT_LIGHT_SHADOWS", 0);
  string_replace_i (str, "NUM_SPOT_LIGHT_MAPS", 0);
  string_replace_i (str, "NUM_SPOT_LIGHT_COORDS", 0);
  string_replace_i (str, "NUM_SPOT_LIGHTS", parameters->num_spot_lights);
  string_replace_i (str, "NUM_RECT_AREA_LIGHTS", parameters->num_rect_area_lights);
  string_replace_i (str, "NUM_POINT_LIGHT_SHADOWS", 0);
  string_replace_i (str, "NUM_POINT_LIGHTS", parameters->num_point_lights);
  string_replace_i (str, "NUM_HEMI_LIGHTS", parameters->num_hemi_lights);
}

static void
replace_clipping_plane_nums (GString *str, GthreeProgramParameters *parameters)
{
  string_replace_i (str, "NUM_CLIPPING_PLANES", parameters->num_clipping_planes);
  string_replace_i (str, "UNION_CLIPPING_PLANES", parameters->num_clipping_planes - parameters->num_clip_intersection);
}


static gboolean
unroll_replace_cb (const GMatchInfo *info,
                   GString          *res,
                   gpointer          data)
{
  g_autofree gchar *start_s = g_match_info_fetch (info, 1);
  g_autofree gchar *end_s = g_match_info_fetch (info, 2);
  g_autofree gchar *snippet = g_match_info_fetch (info, 3);
  int start, end, i;

  start = atoi (start_s);
  end = atoi (end_s);

  for (i = start; i < end; i++)
    {
      g_autoptr(GString) s = g_string_new (snippet);
      g_autofree char *i_s = g_strdup_printf ("[ %d ]", i);
      g_autofree char *index_s = g_strdup_printf ("%d", i);

      string_replace (s, "[ i ]", i_s);
      string_replace (s, "UNROLLED_LOOP_INDEX", index_s);
      g_string_append (res, s->str);
    }

  return FALSE;
}

static char *
unroll_loops (GString *str)
{
  g_autoptr(GRegex) regex = g_regex_new (
    "#pragma unroll_loop_start\\s+for\\s*\\(\\s*int\\s+i\\s*=\\s*(\\d+)\\s*;\\s*i\\s*<\\s*(\\d+)\\s*;\\s*i\\s*\\+\\+\\s*\\)\\s*\\{([\\s\\S]+?)\\}\\s+#pragma unroll_loop_end",
    0, 0, NULL);

  return g_regex_replace_eval (regex, str->str, str->len, 0, 0, unroll_replace_cb, NULL, NULL);
}

static void
parse_include (char *file,
               GString *s)
{
  GBytes *bytes;
  char *end;
  g_autofree char *full_path = NULL;

  while (*file != '<' && *file != 0)
    file++;

  if (*file == 0)
    {
      g_warning ("No initial \" in include");
      return;
    }

  file++;

  end = file;
  while (*end != '>' && *end != 0)
    end++;

  if (*end == 0)
    {
      g_warning ("No final \" in include");
      return;
    }

  *end = 0;

  full_path = g_strconcat ("/org/gnome/gthree/shader_chunks/", file, ".glsl", NULL);
  bytes = g_resources_lookup_data (full_path, 0, NULL);
  if (bytes == NULL)
    {
      g_warning ("shader snipped %s not found", file);
      return;
    }

  g_string_append_printf (s, "// Include: %s\n", file);
  g_string_append_len (s,
                        g_bytes_get_data (bytes, NULL),
                        g_bytes_get_size (bytes));
  g_string_append_c (s, '\n');
  g_bytes_unref (bytes);
}

static char *
parse_text_with_includes (const char *text)
{
  GString *s;
  char **lines;
  int i;

  s = g_string_new ("");

  lines = g_strsplit (text, "\n", -1);
  for (i = 0; lines[i] != NULL; i++)
    {
      char *line = lines[i];
      while (g_ascii_isspace (*line))
        line++;
      if (g_str_has_prefix (line, "#include"))
        parse_include (lines[i] + strlen ("#include"), s);
      else
        {
          g_string_append (s, lines[i]);
          g_string_append_c (s, '\n');
        }
    }

  g_strfreev (lines);

  return g_string_free (s, FALSE);
}

static void
get_texel_encoding_function (GString *shader,
                             const char *function_name)
{
  g_string_append_printf (shader,
                          "vec4 %s( vec4 value ) { return sRGBTransferOETF( value ); }\n",
                          function_name);
}

static const char *
get_tone_mapping_function (int tone_mapping)
{
  switch (tone_mapping)
    {
    case 1: return "LinearToneMapping";
    case 2: return "ReinhardToneMapping";
    case 3: return "OptimizedCineonToneMapping";
    case 4: return "ACESFilmicToneMapping";
    case 5: return "AgXToneMapping";
    case 6: return "NeutralToneMapping";
    default: return "LinearToneMapping";
    }
}

static void
get_luminance_function (GString *shader)
{
  g_string_append (shader,
                   "float luminance( const in vec3 rgb ) {\n"
                   "  const vec3 weights = vec3( 0.2126, 0.7152, 0.0722 );\n"
                   "  return dot( weights, rgb );\n"
                   "}\n");
}

GthreeProgram *
gthree_program_new (GthreeShader *shader, GthreeProgramParameters *parameters, GthreeRenderer *renderer)
{
  GthreeProgram *program;
  GthreeProgramPrivate *priv;
  GPtrArray *defines;
  const char *vertex_shader, *fragment_shader;
  char *index0AttributeName;
  const char *shadow_map_type_define;
  const char *env_map_type_define;
  const char *env_map_mode_define;
  const char *env_map_blending_define;
  GLuint gl_program;
  GString *vertex, *fragment;
  g_autofree char *vertex_unrolled = NULL;
  g_autofree char *fragment_unrolled = NULL;
  g_autofree char *vertex_expanded = NULL;
  g_autofree char *fragment_expanded = NULL;
  const char *shader_name;
  GLuint glVertexShader, glFragmentShader;
  GLint status;
  char formatd_buffer[G_ASCII_DTOSTR_BUF_SIZE];

  program = g_object_new (gthree_program_get_type (),
                          NULL);
  priv = gthree_program_get_instance_private (program);

  priv->shader = g_object_ref (shader);
  memcpy (&priv->params, parameters, sizeof (GthreeProgramParameters));

  //var attributes = material.attributes;
  defines = gthree_shader_get_defines (shader);
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

  shadow_map_type_define = "SHADOWMAP_TYPE_BASIC";
  if (parameters->shadow_map_type == GTHREE_SHADOW_MAP_TYPE_PCF)
    {
      shadow_map_type_define = "SHADOWMAP_TYPE_PCF";
    }
  else if (parameters->shadow_map_type == GTHREE_SHADOW_MAP_TYPE_PCF_SOFT)
    {
      shadow_map_type_define = "SHADOWMAP_TYPE_PCF_SOFT";
    }

  env_map_type_define = "ENVMAP_TYPE_CUBE";
  env_map_mode_define = "ENVMAP_MODE_REFLECTION";
  env_map_blending_define = "ENVMAP_BLENDING_MULTIPLY";

  if (parameters->env_map)
    {
      switch (parameters->env_map_mode)
        {
        case GTHREE_MAPPING_CUBE_REFLECTION:
        case GTHREE_MAPPING_CUBE_REFRACTION:
          env_map_type_define = "ENVMAP_TYPE_CUBE";
          break;
          break;
        case GTHREE_MAPPING_SPHERICAL_REFLECTION:
        case GTHREE_MAPPING_SPHERICAL_REFRACTION:
          env_map_type_define = "ENVMAP_TYPE_SPHERE";
          break;
        }
      switch (parameters->env_map_mode)
        {
        case GTHREE_MAPPING_CUBE_REFRACTION:
          env_map_mode_define = "ENVMAP_MODE_REFRACTION";
        }
    }

#if TODO
  if ( parameters.envMap ) {
		switch ( material.combine ) {
			case MultiplyOperation:
				envMapBlendingDefine = 'ENVMAP_BLENDING_MULTIPLY';
				break;
			case MixOperation:
				envMapBlendingDefine = 'ENVMAP_BLENDING_MIX';
				break;
			case AddOperation:
				envMapBlendingDefine = 'ENVMAP_BLENDING_ADD';
				break;
		}
	}
#endif

  // console.log( "building new program " );

  gl_program = glCreateProgram ();

  vertex = g_string_new ("");
  fragment = g_string_new ("");

  shader_name = gthree_shader_get_name (shader);

  if (TRUE /*! material instanceof THREE.RawShaderMaterial */)
    {
      gboolean is_gles = !epoxy_is_desktop_gl ();

      if (is_gles)
        {
          g_string_append (vertex, "#version 300 es\n");
          g_string_append_printf (vertex, "precision %s float;\n", precision_to_string (parameters->precision));
          g_string_append_printf (vertex, "precision %s int;\n", precision_to_string (parameters->precision));
          g_string_append (vertex, "#define attribute in\n");
          g_string_append (vertex, "#define varying out\n");
          g_string_append (vertex, "#define texture2D texture\n");
          g_string_append (vertex, "#define textureCube texture\n");
        }
      else
        {
          g_string_append (vertex, "#version 130\n");
          g_string_append_printf (vertex, "precision %s float;\n", precision_to_string (parameters->precision));
          g_string_append_printf (vertex, "precision %s int;\n", precision_to_string (parameters->precision));
        }

      if (shader_name)
        g_string_append_printf (vertex, "#define SHADER_NAME %s\n", shader_name);

      if (defines)
        generate_defines (vertex, defines);

      if (parameters->supports_vertex_textures)
        g_string_append (vertex, "#define VERTEX_TEXTURES\n");

      if (parameters->use_fog && parameters->fog)
        g_string_append (vertex, "#define USE_FOG\n");
      if (parameters->use_fog && parameters->fog_exp)
        g_string_append (vertex, "#define FOG_EXP2\n");

      if (parameters->map)
        g_string_append (vertex, "#define USE_MAP\n");
      if (parameters->env_map)
        {
          g_string_append_printf (vertex,
                                  "#define USE_ENVMAP\n"
                                  "#define %s\n", env_map_mode_define);
        }
      if (parameters->light_map)
        g_string_append (vertex, "#define USE_LIGHTMAP\n");
      if (parameters->ao_map)
        g_string_append (vertex, "#define USE_AOMAP\n");
      if (parameters->emissive_map)
        g_string_append (vertex, "#define USE_EMISSIVEMAP\n");
      if (parameters->bump_map)
        g_string_append (vertex, "#define USE_BUMPMAP\n");
      if (parameters->normal_map)
        g_string_append (vertex, "#define USE_NORMALMAP\n");
      if (parameters->normal_map_object_space)
        g_string_append (vertex, "#define USE_NORMALMAP_OBJECTSPACE\n");
      if (parameters->normal_map_tangent_space)
        g_string_append (vertex, "#define USE_NORMALMAP_TANGENTSPACE\n");
      if (parameters->displacement_map && parameters->supports_vertex_textures)
        g_string_append (vertex, "#define USE_DISPLACEMENTMAP\n");
      if (parameters->specular_map)
        g_string_append (vertex, "#define USE_SPECULARMAP\n");
      if (parameters->specular_color_map)
        g_string_append (vertex, "#define USE_SPECULAR_COLORMAP\n");
      if (parameters->specular_intensity_map)
        g_string_append (vertex, "#define USE_SPECULAR_INTENSITYMAP\n");
      if (parameters->roughness_map)
        g_string_append (vertex, "#define USE_ROUGHNESSMAP\n");
      if (parameters->metalness_map)
        g_string_append (vertex, "#define USE_METALNESSMAP\n");
      if (parameters->alpha_map)
        g_string_append (vertex, "#define USE_ALPHAMAP\n");
      if (parameters->alpha_hash)
        g_string_append (vertex, "#define USE_ALPHAHASH\n");

      /* UV channel mapping — all maps currently use channel 0 (uv) */
      if (parameters->map)
        g_string_append (vertex, "#define MAP_UV uv\n");
      if (parameters->alpha_map)
        g_string_append (vertex, "#define ALPHAMAP_UV uv\n");
      if (parameters->light_map)
        g_string_append (vertex, "#define LIGHTMAP_UV uv\n");
      if (parameters->ao_map)
        g_string_append (vertex, "#define AOMAP_UV uv\n");
      if (parameters->emissive_map)
        g_string_append (vertex, "#define EMISSIVEMAP_UV uv\n");
      if (parameters->bump_map)
        g_string_append (vertex, "#define BUMPMAP_UV uv\n");
      if (parameters->normal_map)
        g_string_append (vertex, "#define NORMALMAP_UV uv\n");
      if (parameters->displacement_map)
        g_string_append (vertex, "#define DISPLACEMENTMAP_UV uv\n");
      if (parameters->metalness_map)
        g_string_append (vertex, "#define METALNESSMAP_UV uv\n");
      if (parameters->roughness_map)
        g_string_append (vertex, "#define ROUGHNESSMAP_UV uv\n");
      if (parameters->specular_map)
        g_string_append (vertex, "#define SPECULARMAP_UV uv\n");
      if (parameters->specular_color_map)
        g_string_append (vertex, "#define SPECULAR_COLORMAP_UV uv\n");
      if (parameters->specular_intensity_map)
        g_string_append (vertex, "#define SPECULAR_INTENSITYMAP_UV uv\n");
      if (parameters->anisotropy_map)
        g_string_append (vertex, "#define ANISOTROPYMAP_UV uv\n");
      if (parameters->clearcoat_map)
        g_string_append (vertex, "#define CLEARCOATMAP_UV uv\n");
      if (parameters->clearcoat_normal_map)
        g_string_append (vertex, "#define CLEARCOAT_NORMALMAP_UV uv\n");
      if (parameters->clearcoat_roughness_map)
        g_string_append (vertex, "#define CLEARCOAT_ROUGHNESSMAP_UV uv\n");
      if (parameters->iridescence_map)
        g_string_append (vertex, "#define IRIDESCENCEMAP_UV uv\n");
      if (parameters->iridescence_thickness_map)
        g_string_append (vertex, "#define IRIDESCENCE_THICKNESSMAP_UV uv\n");
      if (parameters->sheen_color_map)
        g_string_append (vertex, "#define SHEEN_COLORMAP_UV uv\n");
      if (parameters->sheen_roughness_map)
        g_string_append (vertex, "#define SHEEN_ROUGHNESSMAP_UV uv\n");
      if (parameters->transmission_map)
        g_string_append (vertex, "#define TRANSMISSIONMAP_UV uv\n");
      if (parameters->thickness_map)
        g_string_append (vertex, "#define THICKNESSMAP_UV uv\n");
      if (parameters->gradient_map)
        g_string_append (vertex, "#define GRADIENTMAP_UV uv\n");

      if (parameters->anisotropy)
        g_string_append (vertex, "#define USE_ANISOTROPY\n");
      if (parameters->anisotropy_map)
        g_string_append (vertex, "#define USE_ANISOTROPYMAP\n");
      if (parameters->clearcoat_map)
        g_string_append (vertex, "#define USE_CLEARCOATMAP\n");
      if (parameters->clearcoat_roughness_map)
        g_string_append (vertex, "#define USE_CLEARCOAT_ROUGHNESSMAP\n");
      if (parameters->clearcoat_normal_map)
        g_string_append (vertex, "#define USE_CLEARCOAT_NORMALMAP\n");
      if (parameters->iridescence_map)
        g_string_append (vertex, "#define USE_IRIDESCENCEMAP\n");
      if (parameters->iridescence_thickness_map)
        g_string_append (vertex, "#define USE_IRIDESCENCE_THICKNESSMAP\n");
      if (parameters->sheen_color_map)
        g_string_append (vertex, "#define USE_SHEEN_COLORMAP\n");
      if (parameters->sheen_roughness_map)
        g_string_append (vertex, "#define USE_SHEEN_ROUGHNESSMAP\n");
      if (parameters->transmission_map)
        g_string_append (vertex, "#define USE_TRANSMISSIONMAP\n");
      if (parameters->thickness_map)
        g_string_append (vertex, "#define USE_THICKNESSMAP\n");

      if (parameters->vertex_tangents)
        g_string_append (vertex, "#define USE_TANGENT\n");
      if (parameters->vertex_colors)
        g_string_append (vertex, "#define USE_COLOR\n");
      if (parameters->vertex_alphas)
        g_string_append (vertex, "#define USE_COLOR_ALPHA\n");
      if (parameters->vertex_uv1s)
        g_string_append (vertex, "#define USE_UV1\n");
      if (parameters->vertex_uv2s)
        g_string_append (vertex, "#define USE_UV2\n");
      if (parameters->vertex_uv3s)
        g_string_append (vertex, "#define USE_UV3\n");
      if (parameters->points_uvs)
        g_string_append (vertex, "#define USE_POINTS_UV\n");

      if (parameters->flat_shading)
        g_string_append (vertex, "#define FLAT_SHADED\n");

      if (parameters->skinning)
        g_string_append (vertex, "#define USE_SKINNING\n");
      if (parameters->use_vertex_texture)
        g_string_append (vertex, "#define BONE_TEXTURE\n");
      if (parameters->skinning && !parameters->use_vertex_texture)
        g_string_append_printf (vertex, "#define MAX_BONES %d\n",
                                parameters->max_bones);

      if (parameters->morph_targets)
        g_string_append (vertex, "#define USE_MORPHTARGETS\n");
      if (parameters->morph_normals && !parameters->flat_shading)
        g_string_append (vertex, "#define USE_MORPHNORMALS\n");
      if (parameters->morph_colors)
        g_string_append (vertex, "#define USE_MORPHCOLORS\n");

      if (parameters->double_sided)
        g_string_append (vertex, "#define DOUBLE_SIDED\n");
      if (parameters->flip_sided)
        g_string_append (vertex, "#define FLIP_SIDED\n");

      if (parameters->shadow_map_enabled)
        g_string_append_printf (vertex,
                                "#define USE_SHADOWMAP\n"
                                "#define %s\n",
                                shadow_map_type_define);

      if (parameters->size_attenuation)
        g_string_append (vertex, "#define USE_SIZEATTENUATION\n");

      if (parameters->logarithmic_depth_buffer)
        {
          g_string_append (vertex, "#define USE_LOGDEPTHBUF\n");
          g_string_append (vertex, "#define USE_LOGDEPTHBUF_EXT\n");
        }

      g_string_append (vertex,
                       "uniform mat4 modelMatrix;\n"
                       "uniform mat4 modelViewMatrix;\n"
                       "uniform mat4 projectionMatrix;\n"
                       "uniform mat4 viewMatrix;\n"
                       "uniform mat3 normalMatrix;\n"
                       "uniform vec3 cameraPosition;\n"
                       "uniform bool isOrthographic;\n"

                       "#ifdef USE_INSTANCING\n"
                       "	attribute mat4 instanceMatrix;\n"
                       "#endif\n"

                       "#ifdef USE_INSTANCING_COLOR\n"
                       "	attribute vec3 instanceColor;\n"
                       "#endif\n"

                       "#ifdef USE_INSTANCING_MORPH\n"
                       "	uniform sampler2D morphTexture;\n"
                       "#endif\n"

                       "attribute vec3 position;\n"
                       "attribute vec3 normal;\n"
                       "attribute vec2 uv;\n"

                       "#ifdef USE_UV1\n"
                       "	attribute vec2 uv1;\n"
                       "#endif\n"

                       "#ifdef USE_UV2\n"
                       "	attribute vec2 uv2;\n"
                       "#endif\n"

                       "#ifdef USE_UV3\n"
                       "	attribute vec2 uv3;\n"
                       "#endif\n"

                       "#ifdef USE_TANGENT\n"
                       "	attribute vec4 tangent;\n"
                       "#endif\n"

                       "#ifdef USE_COLOR_ALPHA\n"
                       "	attribute vec4 color;\n"
                       "#elif defined( USE_COLOR )\n"
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

      if (is_gles)
        {
          g_string_append (fragment, "#version 300 es\n");
          g_string_append_printf (fragment, "precision %s float;\n", precision_to_string (parameters->precision));
          g_string_append_printf (fragment, "precision %s int;\n", precision_to_string (parameters->precision));
          g_string_append (fragment, "#define varying in\n");
          g_string_append (fragment, "#define texture2D texture\n");
          g_string_append (fragment, "#define textureCube texture\n");
          g_string_append (fragment, "#define texture2DLodEXT textureLod\n");
          g_string_append (fragment, "#define textureCubeLodEXT textureLod\n");
          g_string_append (fragment, "#define gl_FragColor pc_fragColor\n");
          g_string_append (fragment, "#define gl_FragDepthEXT gl_FragDepth\n");
          g_string_append (fragment, "out vec4 pc_fragColor;\n");
        }
      else
        {
          g_string_append (fragment, "#version 130\n");
          g_string_append_printf (fragment, "precision %s float;\n", precision_to_string (parameters->precision));
          g_string_append_printf (fragment, "precision %s int;\n", precision_to_string (parameters->precision));
          g_string_append (fragment, "#define texture2DLodEXT textureLod\n");
          g_string_append (fragment, "#define textureCubeLodEXT textureLod\n");
          g_string_append (fragment, "#define gl_FragDepthEXT gl_FragDepth\n");
        }

      if (shader_name)
        g_string_append_printf (fragment, "#define SHADER_NAME %s\n", shader_name);

      if (defines)
        generate_defines (fragment, defines);

      if (parameters->alpha_test > 0)
        {
          g_string_append_printf (fragment, "#define ALPHATEST %s\n",
                                  g_ascii_formatd (formatd_buffer, sizeof(formatd_buffer),
                                                   "%.3f", parameters->alpha_test / 255.0));
        }

      if (parameters->use_fog && parameters->fog)
        g_string_append (fragment, "#define USE_FOG\n");
      if (parameters->use_fog && parameters->fog_exp)
        g_string_append (fragment, "#define FOG_EXP2\n");

      if (parameters->map)
        g_string_append (fragment, "#define USE_MAP\n");
      if (parameters->matcap)
        g_string_append (fragment, "#define USE_MATCAP\n");
      if (parameters->env_map)
        {
          g_string_append_printf (fragment,
                                  "#define USE_ENVMAP\n"
                                  "#define %s\n"
                                  "#define %s\n"
                                  "#define %s\n",
                                  env_map_type_define, env_map_mode_define, env_map_blending_define);
        }
      if (parameters->light_map)
        g_string_append (fragment, "#define USE_LIGHTMAP\n");
      if (parameters->ao_map)
        g_string_append (fragment, "#define USE_AOMAP\n");
      if (parameters->emissive_map)
        g_string_append (fragment, "#define USE_EMISSIVEMAP\n");
      if (parameters->bump_map)
        g_string_append (fragment, "#define USE_BUMPMAP\n");
      if (parameters->normal_map)
        g_string_append (fragment, "#define USE_NORMALMAP\n");
      if (parameters->normal_map_object_space)
        g_string_append (fragment, "#define USE_NORMALMAP_OBJECTSPACE\n");
      if (parameters->normal_map_tangent_space)
        g_string_append (fragment, "#define USE_NORMALMAP_TANGENTSPACE\n");
      if (parameters->specular_map)
        g_string_append (fragment, "#define USE_SPECULARMAP\n");
      if (parameters->specular_color_map)
        g_string_append (fragment, "#define USE_SPECULAR_COLORMAP\n");
      if (parameters->specular_intensity_map)
        g_string_append (fragment, "#define USE_SPECULAR_INTENSITYMAP\n");
      if (parameters->roughness_map)
        g_string_append (fragment, "#define USE_ROUGHNESSMAP\n");
      if (parameters->metalness_map)
        g_string_append (fragment, "#define USE_METALNESSMAP\n");
      if (parameters->alpha_map)
        g_string_append (fragment, "#define USE_ALPHAMAP\n");
      if (parameters->alpha_hash)
        g_string_append (fragment, "#define USE_ALPHAHASH\n");

      if (parameters->vertex_tangents)
        g_string_append (fragment, "#define USE_TANGENT\n");
      if (parameters->vertex_colors)
        g_string_append (fragment, "#define USE_COLOR\n");
      if (parameters->vertex_alphas)
        g_string_append (fragment, "#define USE_COLOR_ALPHA\n");
      if (parameters->vertex_uv1s)
        g_string_append (fragment, "#define USE_UV1\n");
      if (parameters->vertex_uv2s)
        g_string_append (fragment, "#define USE_UV2\n");
      if (parameters->vertex_uv3s)
        g_string_append (fragment, "#define USE_UV3\n");

      if (parameters->gradient_map)
        g_string_append (fragment, "#define USE_GRADIENTMAP\n");

      if (parameters->flat_shading)
        g_string_append (fragment, "#define FLAT_SHADED\n");

      if (parameters->double_sided)
        g_string_append (fragment, "#define DOUBLE_SIDED\n");
      if (parameters->flip_sided)
        g_string_append (fragment, "#define FLIP_SIDED\n");

      if (parameters->shadow_map_enabled)
        g_string_append_printf (fragment,
                                "#define USE_SHADOWMAP\n"
                                "#define %s\n",
                                shadow_map_type_define);

      if (parameters->premultiplied_alpha)
        g_string_append (fragment, "#define PREMULTIPLIED_ALPHA\n");

      if (parameters->physically_correct_lights)
        g_string_append (fragment, "#define PHYSICALLY_CORRECT_LIGHTS\n");

      if (parameters->logarithmic_depth_buffer)
        {
          g_string_append (fragment, "#define USE_LOGDEPTHBUF\n");
          g_string_append (fragment, "#define USE_LOGDEPTHBUF_EXT\n");
        }
      if (parameters->env_map)
        g_string_append (fragment, "#define TEXTURE_LOD_EXT\n");

      if (parameters->anisotropy)
        g_string_append (fragment, "#define USE_ANISOTROPY\n");
      if (parameters->clearcoat)
        g_string_append (fragment, "#define USE_CLEARCOAT\n");
      if (parameters->clearcoat_map)
        g_string_append (fragment, "#define USE_CLEARCOATMAP\n");
      if (parameters->clearcoat_roughness_map)
        g_string_append (fragment, "#define USE_CLEARCOAT_ROUGHNESSMAP\n");
      if (parameters->clearcoat_normal_map)
        g_string_append (fragment, "#define USE_CLEARCOAT_NORMALMAP\n");
      if (parameters->dispersion)
        g_string_append (fragment, "#define USE_DISPERSION\n");
      if (parameters->iridescence)
        g_string_append (fragment, "#define USE_IRIDESCENCE\n");
      if (parameters->iridescence_map)
        g_string_append (fragment, "#define USE_IRIDESCENCEMAP\n");
      if (parameters->iridescence_thickness_map)
        g_string_append (fragment, "#define USE_IRIDESCENCE_THICKNESSMAP\n");
      if (parameters->sheen)
        g_string_append (fragment, "#define USE_SHEEN\n");
      if (parameters->sheen_color_map)
        g_string_append (fragment, "#define USE_SHEEN_COLORMAP\n");
      if (parameters->sheen_roughness_map)
        g_string_append (fragment, "#define USE_SHEEN_ROUGHNESSMAP\n");
      if (parameters->transmission)
        g_string_append (fragment, "#define USE_TRANSMISSION\n");
      if (parameters->transmission_map)
        g_string_append (fragment, "#define USE_TRANSMISSIONMAP\n");
      if (parameters->thickness_map)
        g_string_append (fragment, "#define USE_THICKNESSMAP\n");

      if (parameters->alpha_to_coverage)
        g_string_append (fragment, "#define ALPHA_TO_COVERAGE\n");
      if (parameters->opaque)
        g_string_append (fragment, "#define OPAQUE\n");

      g_string_append (fragment,
                       "uniform mat4 viewMatrix;\n"
                       "uniform vec3 cameraPosition;\n"
                       "uniform bool isOrthographic;\n");

      if (parameters->tone_mapping != 0)
        {
          g_string_append (fragment, "#define TONE_MAPPING\n");
          g_string_append (fragment, "#include <tonemapping_pars_fragment>\n");
          g_string_append_printf (fragment,
                                 "vec3 toneMapping( vec3 color ) { return %s( color ); }\n",
                                 get_tone_mapping_function (parameters->tone_mapping));
        }

      if (parameters->dithering)
        g_string_append (fragment, "#define DITHERING\n");

      g_string_append (fragment, "#include <colorspace_pars_fragment>\n");
      get_texel_encoding_function (fragment, "linearToOutputTexel");
      get_luminance_function (fragment);

      if (parameters->depth_packing > 0)
        {
          GthreeDepthPackingFormat format = parameters->depth_packing - 1;
          if (format == GTHREE_DEPTH_PACKING_FORMAT_BASIC)
            g_string_append_printf (fragment, "#define DEPTH_PACKING 3200\n");
          else
            g_string_append_printf (fragment, "#define DEPTH_PACKING 3201\n");
        }
  }

  g_string_append (vertex, vertex_shader);
  replace_light_nums (vertex, parameters);
  replace_clipping_plane_nums (vertex, parameters);

  g_string_append (fragment, fragment_shader);
  replace_light_nums (fragment, parameters);
  replace_clipping_plane_nums (fragment, parameters);

  vertex_unrolled = unroll_loops (vertex);
  fragment_unrolled = unroll_loops (fragment);

  vertex_expanded = parse_text_with_includes (vertex_unrolled);
  fragment_expanded = parse_text_with_includes (fragment_unrolled);

  if (0)
    {
      g_print ("************ VERTEX *******************************************************\n%s\n",
               vertex_expanded);
      g_print ("************ FRAGMENT *******************************************************\n%s\n",
               fragment_expanded);
    }

  glVertexShader = create_shader (GL_VERTEX_SHADER, vertex_expanded);
  glFragmentShader = create_shader (GL_FRAGMENT_SHADER, fragment_expanded);

  g_string_free (vertex, TRUE);
  g_string_free (fragment, TRUE);

  glAttachShader (gl_program, glVertexShader);
  glAttachShader (gl_program, glFragmentShader);

#ifdef DEBUG_LABELS
  if (shader_name)
    {
      g_autofree char *vlabel = g_strdup_printf ("%s.vert", shader_name);
      g_autofree char *flabel = g_strdup_printf ("%s.frag", shader_name);
      glObjectLabel (GL_SHADER, glVertexShader, strlen (vlabel), vlabel);
      glObjectLabel (GL_SHADER, glFragmentShader, strlen (flabel), flabel);
      glObjectLabel (GL_PROGRAM, gl_program, strlen (shader_name), shader_name);
    }
#endif

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

  priv->gl_program = gl_program;

  return program;
}

static void
gthree_program_init (GthreeProgram *program)
{

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

  if (priv->uniform_locations == NULL)
    get_uniform_locations (program);

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
          int location;

          glGetActiveAttrib (priv->gl_program,  i,  max_len,  NULL,  &size,  &type,  buffer);
          location = glGetAttribLocation (priv->gl_program, buffer);
          g_hash_table_insert (priv->attribute_locations,
                               GINT_TO_POINTER (g_quark_from_string (buffer)), GINT_TO_POINTER (location));
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

/* Returns instance owned by cache */
GthreeProgram *
gthree_program_cache_get (GthreeProgramCache *cache, GthreeShader *shader, GthreeProgramParameters *parameters, GthreeRenderer *renderer)
{
  GthreeProgramPrivate *priv;
  GthreeProgramPrivate key = {NULL};
  GthreeProgram *program;

  key.shader = shader;
  memcpy (&key.params, parameters, sizeof (GthreeProgramParameters));

  program = g_hash_table_lookup (cache->hash, &key);
  if (program)
    return program;

  program = gthree_program_new (shader, parameters, renderer);
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
