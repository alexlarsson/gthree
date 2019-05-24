#include <math.h>
#include <epoxy/gl.h>

#include "gthreeshader.h"
#include "gthreeprogram.h"


typedef struct {
  GPtrArray *defines;
  GthreeUniforms *uniforms;
  char *vertex_shader_text;
  char *fragment_shader_text;
  GthreeShader *owner_of_shader_text;
  guint hash;
} GthreeShaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeShader, gthree_shader, G_TYPE_OBJECT);

static void gthree_shader_init_libs ();
static void gthree_shader_init_hash (GthreeShader *shader);

GthreeShader *
gthree_shader_new (GPtrArray *defines,
                   GthreeUniforms *uniforms,
                   const char *vertex_shader_text,
                   const char *fragment_shader_text)
{
  GthreeShader *shader;
  GthreeShaderPrivate *priv;

  shader = g_object_new (gthree_shader_get_type (), NULL);
  priv = gthree_shader_get_instance_private (shader);

  if (defines)
    priv->defines = g_ptr_array_ref (defines);
  if (uniforms)
    priv->uniforms = g_object_ref (uniforms);

  if (vertex_shader_text)
    priv->vertex_shader_text = g_strdup (vertex_shader_text);
  else
    priv->vertex_shader_text = g_strdup ("void main() {\n\tgl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );\n}");

  if (fragment_shader_text)
    priv->fragment_shader_text = g_strdup (fragment_shader_text);
  else
    priv->fragment_shader_text = g_strdup ("void main() {\n\tgl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );\n}");

  priv->owner_of_shader_text = NULL;

  gthree_shader_init_hash (shader);

  return shader;
}

static void
gthree_shader_init (GthreeShader *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  priv->owner_of_shader_text = NULL;
}

static void
gthree_shader_finalize (GObject *obj)
{
  GthreeShader *shader = GTHREE_SHADER (obj);
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  g_clear_pointer (&priv->defines, g_ptr_array_unref);
  g_clear_object (&priv->uniforms);

  if (priv->owner_of_shader_text)
    g_object_unref (priv->owner_of_shader_text);
  else
    {
      g_clear_pointer (&priv->vertex_shader_text, g_free);
      g_clear_pointer (&priv->fragment_shader_text, g_free);
    }

  G_OBJECT_CLASS (gthree_shader_parent_class)->finalize (obj);
}

static void
gthree_shader_class_init (GthreeShaderClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_shader_finalize;

  gthree_shader_init_libs ();
}

static gboolean
str_equal (const char *a, const char *b)
{
  if (a == b)
    return TRUE;

  return strcmp (a, b) == 0;
}

static gboolean
defines_equal (GPtrArray *a, GPtrArray *b)
{
  int i;

  if (a == b)
    return TRUE;

  if (a == NULL)
    return b == NULL;
  if (b == NULL)
    return a == NULL;

  if (a->len != b->len)
    return FALSE;

  for (i = 0; i < a->len; i++)
    {
      if (!str_equal (g_ptr_array_index (a, i), g_ptr_array_index (b, i)))
        return FALSE;
    }

  return TRUE;
}

static guint
defines_hash (GPtrArray *a)
{
  guint hash = 0;
  int i;

  if (a != NULL)
    {
      for (i = 0; i < a->len; i++)
        hash ^= g_str_hash (g_ptr_array_index (a, i));
    }

  return hash;
}

gboolean
gthree_shader_equal (GthreeShader *a,
                     GthreeShader *b)
{
  GthreeShaderPrivate *priv_a = gthree_shader_get_instance_private (a);
  GthreeShaderPrivate *priv_b = gthree_shader_get_instance_private (b);

  if (a == b)
    return TRUE;

  return
    defines_equal (priv_a->defines, priv_b->defines) &&
    str_equal (priv_a->vertex_shader_text, priv_b->vertex_shader_text) &&
    str_equal (priv_a->fragment_shader_text, priv_b->fragment_shader_text);
}

static void
gthree_shader_init_hash (GthreeShader *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  priv->hash =
    defines_hash (priv->defines) ^
    g_str_hash (priv->vertex_shader_text) ^
    g_str_hash (priv->fragment_shader_text);
}

guint
gthree_shader_hash (GthreeShader *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  return priv->hash;
}

void
gthree_shader_update_uniform_locations_for_program (GthreeShader *shader,
                                                    GthreeProgram *program)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);
  GList *unis, *l;

  unis = gthree_uniforms_get_all (priv->uniforms);
  for (l = unis; l != NULL; l = l->next)
    {
      GthreeUniform *uni = l->data;
      gint location;

      location = gthree_program_lookup_uniform_location (program, gthree_uniform_get_qname (uni));
      gthree_uniform_set_location (uni, location);
    }
  g_list_free (unis);
}

GPtrArray *
gthree_shader_get_defines (GthreeShader  *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  return priv->defines;
}

GthreeUniforms *
gthree_shader_get_uniforms (GthreeShader  *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  return priv->uniforms;
}

const char *
gthree_shader_get_vertex_shader_text (GthreeShader  *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  return priv->vertex_shader_text;
}

const char *
gthree_shader_get_fragment_shader_text (GthreeShader  *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  return priv->fragment_shader_text;
}

GthreeShader *
gthree_shader_clone (GthreeShader *orig)
{
  GthreeShader *clone;
  GthreeShaderPrivate *orig_priv, *clone_priv;

  orig_priv = gthree_shader_get_instance_private (orig);

  clone = g_object_new (gthree_shader_get_type (), NULL);
  clone_priv = gthree_shader_get_instance_private (clone);

  clone_priv->uniforms = gthree_uniforms_clone (orig_priv->uniforms);
  clone_priv->vertex_shader_text = orig_priv->vertex_shader_text;
  clone_priv->fragment_shader_text = orig_priv->fragment_shader_text;

  clone_priv->hash = orig_priv->hash;

  if (orig_priv->owner_of_shader_text)
    clone_priv->owner_of_shader_text = g_object_ref (orig_priv->owner_of_shader_text);
  else
    clone_priv->owner_of_shader_text = g_object_ref (orig);

  return clone;
}

static void
parse_include (char *file,
               GString *s)
{
  GBytes *bytes;
  char *end;

  while (*file != '"' && *file != 0)
    file++;

  if (*file == 0)
    {
      g_warning ("No initial \" in include");
      return;
    }

  file++;

  end = file;
  while (*end != '"' && *end != 0)
    end++;

  if (*end == 0)
    {
      g_warning ("No final \" in include");
      return;
    }

  *end = 0;
  bytes = g_resources_lookup_data (file, 0, NULL);
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
      if (g_str_has_prefix (lines[i], "#include"))
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

GthreeShader *
gthree_shader_new_from_definitions (const char **lib_uniforms,
                                    GthreeUniformsDefinition *uniform_defs, int defs_len,
                                    const char *vertex_shader,
                                    const char *fragment_shader)
{
  GthreeShader *shader = g_object_new (gthree_shader_get_type (), NULL);
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);
  GthreeUniforms *uniforms;
  int i;

  priv->uniforms = gthree_uniforms_new ();
  for (i = 0; lib_uniforms[i] != NULL; i++)
    {
      uniforms = gthree_get_uniforms_from_library (lib_uniforms[i]);
      gthree_uniforms_merge (priv->uniforms, uniforms);
    }

  if (uniform_defs)
    {
      uniforms = gthree_uniforms_new_from_definitions (uniform_defs, defs_len);
      gthree_uniforms_merge (priv->uniforms, uniforms);
      g_object_unref (uniforms);
    }

  priv->vertex_shader_text = parse_text_with_includes (vertex_shader);
  priv->fragment_shader_text = parse_text_with_includes (fragment_shader);

  gthree_shader_init_hash (shader);

  return shader;
}

static int i0 = 0;
static float f0 = 0.0;
static float f1 = 1.0;
static float f2 = 2.0;
static float f30 = 30;
static float fm1 = -1.0;
static float f2000 = 2000;
static float fp98 = 0.98;
static float fp5 = 0.5;
static GdkRGBA dark_grey = { 0.06666666666666667, 0.06666666666666667, 0.06666666666666667, 1.0 };
static GdkRGBA white = { 1, 1, 1, 1.0 };
static GdkRGBA black = { 0, 0, 0, 1.0 };
static float onev2[2] = { 1, 1 };
static float zerov2[2] = { 0, 0 };
static float onev3[3] = { 1, 1, 1 };

static const char *basic_uniform_libs[] = { "common", "fog", "shadowmap", NULL };
static const char *basic_vertex_shader =
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"
  "void main() {\n"
       "#include \"/org/gnome/gthree/shader_chunks/map_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/lightmap_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/color_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/skinbase_vertex.glsl\"\n"
  "     #ifdef USE_ENVMAP\n"
       "#include \"/org/gnome/gthree/shader_chunks/morphnormal_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/skinnormal_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/defaultnormal_vertex.glsl\"\n"
  "     #endif\n"
       "#include \"/org/gnome/gthree/shader_chunks/morphtarget_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/skinning_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/default_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"

       "#include \"/org/gnome/gthree/shader_chunks/worldpos_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/envmap_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/shadowmap_vertex.glsl\"\n"
  "}";

static const char *basic_fragment_shader =
  "uniform vec3 diffuse;\n"
  "uniform float opacity;\n"
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphamap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/specularmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"
  "void main() {\n"
  "     gl_FragColor = vec4( diffuse, opacity );\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphamap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphatest_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/specularmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/linear_to_gamma_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_fragment.glsl\"\n"
  "}";

static const char *lambert_uniform_libs[] = { "common", "fog", "lights", "shadowmap", NULL };

static GthreeUniformsDefinition lambert_uniforms[] = {
  {"ambient", GTHREE_UNIFORM_TYPE_COLOR, &white },
  {"emissive", GTHREE_UNIFORM_TYPE_COLOR, &black },
  {"wrapRGB", GTHREE_UNIFORM_TYPE_VECTOR3, &onev3}
};


static const char *lambert_vertex_shader =
  "#define LAMBERT\n"
  "varying vec3 vLightFront;\n"
  "#ifdef DOUBLE_SIDED\n"
  "     varying vec3 vLightBack;\n"
  "#endif\n"
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lights_lambert_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"
  "void main() {\n"
     "#include \"/org/gnome/gthree/shader_chunks/map_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/lightmap_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/color_vertex.glsl\"\n"

     "#include \"/org/gnome/gthree/shader_chunks/morphnormal_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/skinbase_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/skinnormal_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/defaultnormal_vertex.glsl\"\n"

     "#include \"/org/gnome/gthree/shader_chunks/morphtarget_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/skinning_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/default_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"

     "#include \"/org/gnome/gthree/shader_chunks/worldpos_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/envmap_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/lights_lambert_vertex.glsl\"\n"
     "#include \"/org/gnome/gthree/shader_chunks/shadowmap_vertex.glsl\"\n"
  "}";

static const char *lambert_fragment_shader =
  "uniform float opacity;\n"
  "varying vec3 vLightFront;\n"
  "#ifdef DOUBLE_SIDED\n"
  "     varying vec3 vLightBack;\n"
  "#endif\n"
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphamap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/specularmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"
  "     gl_FragColor = vec4( vec3( 1.0 ), opacity );\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphamap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphatest_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/specularmap_fragment.glsl\"\n"
  "     #ifdef DOUBLE_SIDED\n"
  //"float isFront = float( gl_FrontFacing );\n"
  //"gl_FragColor.xyz *= isFront * vLightFront + ( 1.0 - isFront ) * vLightBack;\n"
  "             if ( gl_FrontFacing )\n"
  "                     gl_FragColor.xyz *= vLightFront;\n"
  "             else\n"
  "                     gl_FragColor.xyz *= vLightBack;\n"
  "     #else\n"
  "             gl_FragColor.xyz *= vLightFront;\n"
  "     #endif\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_fragment.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/linear_to_gamma_fragment.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/fog_fragment.glsl\"\n"
  "}";

static const char *phong_uniform_libs[] = { "common", "bump", "normalmap", "fog", "lights", "shadowmap", NULL };
static GthreeUniformsDefinition phong_uniforms[] = {
  {"ambient", GTHREE_UNIFORM_TYPE_COLOR, &white },
  {"emissive", GTHREE_UNIFORM_TYPE_COLOR, &black },
  {"specular", GTHREE_UNIFORM_TYPE_COLOR, &dark_grey },
  {"shininess", GTHREE_UNIFORM_TYPE_FLOAT, &f30 },
  {"wrapRGB", GTHREE_UNIFORM_TYPE_VECTOR3, &onev3}
};

static const char *phong_vertex_shader =
  "#define PHONG\n"
  "varying vec3 vViewPosition;\n"
  "#ifndef FLAT_SHADED\n"
  "	varying vec3 vNormal;\n"
  "#endif\n"
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lights_phong_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"

  "#include \"/org/gnome/gthree/shader_chunks/map_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_vertex.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/morphnormal_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinbase_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinnormal_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/defaultnormal_vertex.glsl\"\n"

  "#ifndef FLAT_SHADED\n"
  "     vNormal = normalize( transformedNormal );\n"
  "#endif\n"

  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/default_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"

  "     vViewPosition = -mvPosition.xyz;\n"

  "#include \"/org/gnome/gthree/shader_chunks/worldpos_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lights_phong_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_vertex.glsl\"\n"
  "}";

static const char *phong_fragment_shader =
  "uniform vec3 diffuse;\n"
  "uniform float opacity;\n"

  "uniform vec3 ambient;\n"
  "uniform vec3 emissive;\n"
  "uniform vec3 specular;\n"
  "uniform float shininess;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphamap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lightmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/lights_phong_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/bumpmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/normalmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/specularmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"

  "     gl_FragColor = vec4( vec3( 1.0 ), opacity );\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphamap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphatest_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/specularmap_fragment.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/lights_phong_fragment.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/lightmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/envmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_fragment.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/linear_to_gamma_fragment.glsl\"\n"

  "#include \"/org/gnome/gthree/shader_chunks/fog_fragment.glsl\"\n"
  "}";

static const char *particle_basic_uniform_libs[] = { "particle", "shadowmap", NULL };
static const char *particle_basic_vertex_shader =
  "uniform float size;\n"
  "uniform float scale;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"

  "#include \"/org/gnome/gthree/shader_chunks/color_vertex.glsl\"\n"

  "     vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );\n"

  "     #ifdef USE_SIZEATTENUATION\n"
  "             gl_PointSize = size * ( scale / length( mvPosition.xyz ) );\n"
  "     #else\n"
  "             gl_PointSize = size;\n"
  "     #endif\n"

  "     gl_Position = projectionMatrix * mvPosition;\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/worldpos_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_vertex.glsl\"\n"
  "}";

static const char *particle_basic_fragment_shader =
  "uniform vec3 psColor;\n"
  "uniform float opacity;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_particle_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"

  "     gl_FragColor = vec4( psColor, opacity );\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/map_particle_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/alphatest_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_fragment.glsl\"\n"

  "}";

static const char *dashed_uniform_libs[] = { "common", "fog", NULL };
static GthreeUniformsDefinition dashed_uniforms[] = {
  {"scale", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"dashSize", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"totalSize", GTHREE_UNIFORM_TYPE_FLOAT, &f2 },
};

static const char *dashed_vertex_shader =
  "uniform float scale;\n"
  "attribute float lineDistance;\n"

  "varying float vLineDistance;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"

  "#include \"/org/gnome/gthree/shader_chunks/color_vertex.glsl\"\n"

  "     vLineDistance = scale * lineDistance;\n"

  "     vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );\n"
  "     gl_Position = projectionMatrix * mvPosition;\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"

  "}";

static const char *dashed_fragment_shader =
  "uniform vec3 diffuse;\n"
  "uniform float opacity;\n"

  "uniform float dashSize;\n"
  "uniform float totalSize;\n"

  "varying float vLineDistance;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"
  "     if ( mod( vLineDistance, totalSize ) > dashSize ) {\n"
  "             discard;\n"
  "     }\n"
  "     gl_FragColor = vec4( diffuse, opacity );\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/color_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_fragment.glsl\"\n"
  "}";

static const char *depth_uniform_libs[] = { NULL };
static GthreeUniformsDefinition depth_uniforms[] = {
  {"mNear", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
  {"mFar", GTHREE_UNIFORM_TYPE_FLOAT, &f2000 },
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
};

static const char *depth_vertex_shader =
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"

  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/default_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"

  "}";

static const char *depth_fragment_shader =
  "uniform float mNear;\n"
  "uniform float mFar;\n"
  "uniform float opacity;\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "     #ifdef USE_LOGDEPTHBUF_EXT\n"
  "             float depth = gl_FragDepthEXT / gl_FragCoord.w;\n"
  "     #else\n"
  "             float depth = gl_FragCoord.z / gl_FragCoord.w;\n"
  "     #endif\n"
  "     float color = 1.0 - smoothstep( mNear, mFar, depth );\n"
  "     gl_FragColor = vec4( vec3( color ), opacity );\n"
  "}";

static const char *normal_uniform_libs[] = { NULL };
static GthreeUniformsDefinition normal_uniforms[] = {
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1 },
};

static const char *normal_vertex_shader =
  "varying vec3 vNormal;\n"
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"
  "     vNormal = normalize( normalMatrix * normal );\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/default_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"
  "}";

static const char *normal_fragment_shader =
  "uniform float opacity;\n"
  "varying vec3 vNormal;\n"

  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"
  "     gl_FragColor = vec4( 0.5 * normalize( vNormal ) + 0.5, opacity );\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "}";




/* -------------------------------------------------------------------------
//      Normal map shader
//              - Blinn-Phong
//              - normal + diffuse + specular + AO + displacement + reflection + shadow maps
//              - point and directional lights (use with "lights: true" material option)
------------------------------------------------------------------------- */

static const char *normalmap_uniform_libs[] = { "fog", "lights", "shadowmap", NULL };
static GthreeUniformsDefinition normalmap_uniforms[] = {
  {"enableAO", GTHREE_UNIFORM_TYPE_INT, &i0},
  {"enableDiffuse", GTHREE_UNIFORM_TYPE_INT, &i0},
  {"enableSpecular", GTHREE_UNIFORM_TYPE_INT, &i0},
  {"enableReflection", GTHREE_UNIFORM_TYPE_INT, &i0},
  {"enableDisplacement", GTHREE_UNIFORM_TYPE_INT, &i0},

  {"tDisplacement", GTHREE_UNIFORM_TYPE_TEXTURE, NULL}, // must go first as this is vertex texture
  {"tDiffuse", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
  {"tCube", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
  {"tNormal", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
  {"tSpecular", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
  {"tAO", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},

  {"uNormalScale", GTHREE_UNIFORM_TYPE_VECTOR2, &onev2},

  {"uDisplacementBias", GTHREE_UNIFORM_TYPE_FLOAT, &f0},
  {"uDisplacementScale", GTHREE_UNIFORM_TYPE_FLOAT, &f1},

  {"diffuse", GTHREE_UNIFORM_TYPE_COLOR, &white },
  {"specular", GTHREE_UNIFORM_TYPE_COLOR, &dark_grey },
  {"ambient", GTHREE_UNIFORM_TYPE_COLOR, &white },
  {"shininess", GTHREE_UNIFORM_TYPE_FLOAT, &f30},
  {"opacity", GTHREE_UNIFORM_TYPE_FLOAT, &f1},

  {"useRefract", GTHREE_UNIFORM_TYPE_INT, &i0},
  {"refractionRatio", GTHREE_UNIFORM_TYPE_FLOAT, &fp98},
  {"reflectivity", GTHREE_UNIFORM_TYPE_FLOAT, &fp5},

  {"uOffset", GTHREE_UNIFORM_TYPE_VECTOR2, &zerov2},
  {"uRepeat", GTHREE_UNIFORM_TYPE_VECTOR2, &onev2},

  {"wrapRGB", GTHREE_UNIFORM_TYPE_VECTOR3, &onev3},
};

static const char *normalmap_fragment_shader =
  "uniform vec3 ambient;\n"
  "uniform vec3 diffuse;\n"
  "uniform vec3 specular;\n"
  "uniform float shininess;\n"
  "uniform float opacity;\n"

  "uniform bool enableDiffuse;\n"
  "uniform bool enableSpecular;\n"
  "uniform bool enableAO;\n"
  "uniform bool enableReflection;\n"

  "uniform sampler2D tDiffuse;\n"
  "uniform sampler2D tNormal;\n"
  "uniform sampler2D tSpecular;\n"
  "uniform sampler2D tAO;\n"

  "uniform samplerCube tCube;\n"

  "uniform vec2 uNormalScale;\n"

  "uniform bool useRefract;\n"
  "uniform float refractionRatio;\n"
  "uniform float reflectivity;\n"

  "varying vec3 vTangent;\n"
  "varying vec3 vBinormal;\n"
  "varying vec3 vNormal;\n"
  "varying vec2 vUv;\n"

  "uniform vec3 ambientLightColor;\n"

  "#if MAX_DIR_LIGHTS > 0\n"
  "     uniform vec3 directionalLightColor[ MAX_DIR_LIGHTS ];\n"
  "     uniform vec3 directionalLightDirection[ MAX_DIR_LIGHTS ];\n"
  "#endif\n"

  "#if MAX_HEMI_LIGHTS > 0\n"
  "     uniform vec3 hemisphereLightSkyColor[ MAX_HEMI_LIGHTS ];\n"
  "     uniform vec3 hemisphereLightGroundColor[ MAX_HEMI_LIGHTS ];\n"
  "     uniform vec3 hemisphereLightDirection[ MAX_HEMI_LIGHTS ];\n"
  "#endif\n"

  "#if MAX_POINT_LIGHTS > 0\n"
  "     uniform vec3 pointLightColor[ MAX_POINT_LIGHTS ];\n"
  "     uniform vec3 pointLightPosition[ MAX_POINT_LIGHTS ];\n"
  "     uniform float pointLightDistance[ MAX_POINT_LIGHTS ];\n"
  "#endif\n"

  "#if MAX_SPOT_LIGHTS > 0\n"
  "     uniform vec3 spotLightColor[ MAX_SPOT_LIGHTS ];\n"
  "     uniform vec3 spotLightPosition[ MAX_SPOT_LIGHTS ];\n"
  "     uniform vec3 spotLightDirection[ MAX_SPOT_LIGHTS ];\n"
  "     uniform float spotLightAngleCos[ MAX_SPOT_LIGHTS ];\n"
  "     uniform float spotLightExponent[ MAX_SPOT_LIGHTS ];\n"
  "     uniform float spotLightDistance[ MAX_SPOT_LIGHTS ];\n"
  "#endif\n"

  "#ifdef WRAP_AROUND\n"
  "     uniform vec3 wrapRGB;\n"
  "#endif\n"

  "varying vec3 vWorldPosition;\n"
  "varying vec3 vViewPosition;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_pars_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "     gl_FragColor = vec4( vec3( 1.0 ), opacity );\n"
  "     vec3 specularTex = vec3( 1.0 );\n"

  "     vec3 normalTex = texture2D( tNormal, vUv ).xyz * 2.0 - 1.0;\n"
  "     normalTex.xy *= uNormalScale;\n"
  "     normalTex = normalize( normalTex );\n"

  "     if( enableDiffuse ) {\n"
  "             #ifdef GAMMA_INPUT\n"
  "                     vec4 texelColor = texture2D( tDiffuse, vUv );\n"
  "                     texelColor.xyz *= texelColor.xyz;\n"
  "                     gl_FragColor = gl_FragColor * texelColor;\n"
  "             #else\n"
  "                     gl_FragColor = gl_FragColor * texture2D( tDiffuse, vUv );\n"
  "             #endif\n"
  "     }\n"

  "     if( enableAO ) {\n"
  "             #ifdef GAMMA_INPUT\n"
  "                     vec4 aoColor = texture2D( tAO, vUv );\n"
  "                     aoColor.xyz *= aoColor.xyz;\n"
  "                     gl_FragColor.xyz = gl_FragColor.xyz * aoColor.xyz;\n"
  "             #else\n"
  "                     gl_FragColor.xyz = gl_FragColor.xyz * texture2D( tAO, vUv ).xyz;\n"
  "             #endif\n"
  "     }\n"

  "#include \"/org/gnome/gthree/shader_chunks/alphatest_fragment.glsl\"\n"
  "     if( enableSpecular )\n"
  "             specularTex = texture2D( tSpecular, vUv ).xyz;\n"
  "     mat3 tsb = mat3( normalize( vTangent ), normalize( vBinormal ), normalize( vNormal ) );\n"
  "     vec3 finalNormal = tsb * normalTex;\n"

  "     #ifdef FLIP_SIDED\n"
  "             finalNormal = -finalNormal;\n"
  "     #endif\n"

  "     vec3 normal = normalize( finalNormal );\n"
  "     vec3 viewPosition = normalize( vViewPosition );\n"

  // point lights

  "     #if MAX_POINT_LIGHTS > 0\n"
  "             vec3 pointDiffuse = vec3( 0.0 );\n"
  "             vec3 pointSpecular = vec3( 0.0 );\n"
  "             for ( int i = 0; i < MAX_POINT_LIGHTS; i ++ ) {\n"
  "                     vec4 lPosition = viewMatrix * vec4( pointLightPosition[ i ], 1.0 );\n"
  "                     vec3 pointVector = lPosition.xyz + vViewPosition.xyz;\n"
  "                     float pointDistance = 1.0;\n"
  "                     if ( pointLightDistance[ i ] > 0.0 )\n"
  "                             pointDistance = 1.0 - min( ( length( pointVector ) / pointLightDistance[ i ] ), 1.0 );\n"
  "                     pointVector = normalize( pointVector );\n"

  // diffuse

  "                     #ifdef WRAP_AROUND\n"
  "                             float pointDiffuseWeightFull = max( dot( normal, pointVector ), 0.0 );\n"
  "                             float pointDiffuseWeightHalf = max( 0.5 * dot( normal, pointVector ) + 0.5, 0.0 );\n"
  "                             vec3 pointDiffuseWeight = mix( vec3( pointDiffuseWeightFull ), vec3( pointDiffuseWeightHalf ), wrapRGB );\n"
  "                     #else\n"
  "                             float pointDiffuseWeight = max( dot( normal, pointVector ), 0.0 );\n"
  "                     #endif\n"

  "                     pointDiffuse += pointDistance * pointLightColor[ i ] * diffuse * pointDiffuseWeight;\n"

  // specular

  "                     vec3 pointHalfVector = normalize( pointVector + viewPosition );\n"
  "                     float pointDotNormalHalf = max( dot( normal, pointHalfVector ), 0.0 );\n"
  "                     float pointSpecularWeight = specularTex.r * max( pow( pointDotNormalHalf, shininess ), 0.0 );\n"

  "                     float specularNormalization = ( shininess + 2.0 ) / 8.0;\n"

  "                     vec3 schlick = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( pointVector, pointHalfVector ), 0.0 ), 5.0 );\n"
  "                     pointSpecular += schlick * pointLightColor[ i ] * pointSpecularWeight * pointDiffuseWeight * pointDistance * specularNormalization;\n"
  "             }\n"
  "     #endif\n"

  // spot lights

  "     #if MAX_SPOT_LIGHTS > 0\n"
  "             vec3 spotDiffuse = vec3( 0.0 );\n"
  "             vec3 spotSpecular = vec3( 0.0 );\n"
  "             for ( int i = 0; i < MAX_SPOT_LIGHTS; i ++ ) {\n"
  "                     vec4 lPosition = viewMatrix * vec4( spotLightPosition[ i ], 1.0 );\n"
  "                     vec3 spotVector = lPosition.xyz + vViewPosition.xyz;\n"
  "                     float spotDistance = 1.0;\n"
  "                     if ( spotLightDistance[ i ] > 0.0 )\n"
  "                             spotDistance = 1.0 - min( ( length( spotVector ) / spotLightDistance[ i ] ), 1.0 );\n"
  "                     spotVector = normalize( spotVector );\n"
  "                     float spotEffect = dot( spotLightDirection[ i ], normalize( spotLightPosition[ i ] - vWorldPosition ) );\n"
  "                     if ( spotEffect > spotLightAngleCos[ i ] ) {\n"
  "                             spotEffect = max( pow( max( spotEffect, 0.0 ), spotLightExponent[ i ] ), 0.0 );\n"
  // diffuse
  "                             #ifdef WRAP_AROUND\n"
  "                                     float spotDiffuseWeightFull = max( dot( normal, spotVector ), 0.0 );\n"
  "                                     float spotDiffuseWeightHalf = max( 0.5 * dot( normal, spotVector ) + 0.5, 0.0 );\n"
  "                                     vec3 spotDiffuseWeight = mix( vec3( spotDiffuseWeightFull ), vec3( spotDiffuseWeightHalf ), wrapRGB );\n"
  "                             #else\n"
  "                                     float spotDiffuseWeight = max( dot( normal, spotVector ), 0.0 );\n"
  "                             #endif\n"

  "                             spotDiffuse += spotDistance * spotLightColor[ i ] * diffuse * spotDiffuseWeight * spotEffect;\n"

  // specular

  "                             vec3 spotHalfVector = normalize( spotVector + viewPosition );\n"
  "                             float spotDotNormalHalf = max( dot( normal, spotHalfVector ), 0.0 );\n"
  "                             float spotSpecularWeight = specularTex.r * max( pow( spotDotNormalHalf, shininess ), 0.0 );\n"

  "                             float specularNormalization = ( shininess + 2.0 ) / 8.0;\n"

  "                             vec3 schlick = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( spotVector, spotHalfVector ), 0.0 ), 5.0 );\n"
  "                             spotSpecular += schlick * spotLightColor[ i ] * spotSpecularWeight * spotDiffuseWeight * spotDistance * specularNormalization * spotEffect;\n"
  "                     }\n"
  "             }\n"
  "     #endif\n"

  // directional lights

  "     #if MAX_DIR_LIGHTS > 0\n"
  "             vec3 dirDiffuse = vec3( 0.0 );\n"
  "             vec3 dirSpecular = vec3( 0.0 );\n"
  "             for( int i = 0; i < MAX_DIR_LIGHTS; i++ ) {\n"
  "                     vec3 dirVector = transformDirection( directionalLightDirection[ i ], viewMatrix );\n"

  // diffuse

  "                     #ifdef WRAP_AROUND\n"
  "                             float directionalLightWeightingFull = max( dot( normal, dirVector ), 0.0 );\n"
  "                             float directionalLightWeightingHalf = max( 0.5 * dot( normal, dirVector ) + 0.5, 0.0 );\n"
  "                             vec3 dirDiffuseWeight = mix( vec3( directionalLightWeightingFull ), vec3( directionalLightWeightingHalf ), wrapRGB );\n"
  "                     #else\n"
  "                             float dirDiffuseWeight = max( dot( normal, dirVector ), 0.0 );\n"
  "                     #endif\n"

  "                     dirDiffuse += directionalLightColor[ i ] * diffuse * dirDiffuseWeight;\n"

  // specular

  "                     vec3 dirHalfVector = normalize( dirVector + viewPosition );\n"
  "                     float dirDotNormalHalf = max( dot( normal, dirHalfVector ), 0.0 );\n"
  "                     float dirSpecularWeight = specularTex.r * max( pow( dirDotNormalHalf, shininess ), 0.0 );\n"

  "                     float specularNormalization = ( shininess + 2.0 ) / 8.0;\n"

  "                     vec3 schlick = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( dirVector, dirHalfVector ), 0.0 ), 5.0 );\n"
  "                     dirSpecular += schlick * directionalLightColor[ i ] * dirSpecularWeight * dirDiffuseWeight * specularNormalization;\n"
  "             }\n"

  "     #endif\n"

  // hemisphere lights
  "     #if MAX_HEMI_LIGHTS > 0\n"
  "             vec3 hemiDiffuse = vec3( 0.0 );\n"
  "             vec3 hemiSpecular = vec3( 0.0 );\n" 
  "             for( int i = 0; i < MAX_HEMI_LIGHTS; i ++ ) {\n"
  "                     vec3 lVector = transformDirection( hemisphereLightDirection[ i ], viewMatrix );\n"

  // diffuse
  "                     float dotProduct = dot( normal, lVector );\n"
  "                     float hemiDiffuseWeight = 0.5 * dotProduct + 0.5;\n"

  "                     vec3 hemiColor = mix( hemisphereLightGroundColor[ i ], hemisphereLightSkyColor[ i ], hemiDiffuseWeight );\n"
  "                     hemiDiffuse += diffuse * hemiColor;\n"

  // specular (sky light)
  "                     vec3 hemiHalfVectorSky = normalize( lVector + viewPosition );\n"
  "                     float hemiDotNormalHalfSky = 0.5 * dot( normal, hemiHalfVectorSky ) + 0.5;\n"
  "                     float hemiSpecularWeightSky = specularTex.r * max( pow( max( hemiDotNormalHalfSky, 0.0 ), shininess ), 0.0 );\n"

  // specular (ground light)
  "                     vec3 lVectorGround = -lVector;\n"

  "                     vec3 hemiHalfVectorGround = normalize( lVectorGround + viewPosition );\n"
  "                     float hemiDotNormalHalfGround = 0.5 * dot( normal, hemiHalfVectorGround ) + 0.5;\n"
  "                     float hemiSpecularWeightGround = specularTex.r * max( pow( max( hemiDotNormalHalfGround, 0.0 ), shininess ), 0.0 );\n"

  "                     float dotProductGround = dot( normal, lVectorGround );\n"

  "                     float specularNormalization = ( shininess + 2.0 ) / 8.0;\n"

  "                     vec3 schlickSky = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( lVector, hemiHalfVectorSky ), 0.0 ), 5.0 );\n"
  "                     vec3 schlickGround = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( lVectorGround, hemiHalfVectorGround ), 0.0 ), 5.0 );\n"
  "                     hemiSpecular += hemiColor * specularNormalization * ( schlickSky * hemiSpecularWeightSky * max( dotProduct, 0.0 ) + schlickGround * hemiSpecularWeightGround * max( dotProductGround, 0.0 ) );\n"

  "             }\n"
  "     #endif\n"

  // all lights contribution summation
  "     vec3 totalDiffuse = vec3( 0.0 );\n"
  "     vec3 totalSpecular = vec3( 0.0 );\n"

  "     #if MAX_DIR_LIGHTS > 0\n"
  "             totalDiffuse += dirDiffuse;\n"
  "             totalSpecular += dirSpecular;\n"
  "     #endif\n"

  "     #if MAX_HEMI_LIGHTS > 0\n"
  "             totalDiffuse += hemiDiffuse;\n"
  "             totalSpecular += hemiSpecular;\n"
  "     #endif\n"

  "     #if MAX_POINT_LIGHTS > 0\n"
  "             totalDiffuse += pointDiffuse;\n"
  "             totalSpecular += pointSpecular;\n"
  "     #endif\n"

  "     #if MAX_SPOT_LIGHTS > 0\n"
  "             totalDiffuse += spotDiffuse;\n"
  "             totalSpecular += spotSpecular;\n"
  "     #endif\n"

  "     #ifdef METAL\n"
  "             gl_FragColor.xyz = gl_FragColor.xyz * ( totalDiffuse + ambientLightColor * ambient + totalSpecular );\n"
  "     #else\n"
  "             gl_FragColor.xyz = gl_FragColor.xyz * ( totalDiffuse + ambientLightColor * ambient ) + totalSpecular;\n"
  "     #endif\n"

  "     if ( enableReflection ) {\n"
  "             vec3 vReflect;\n"
  "             vec3 cameraToVertex = normalize( vWorldPosition - cameraPosition );\n"

  "             if ( useRefract ) {\n"
  "                     vReflect = refract( cameraToVertex, normal, refractionRatio );\n"
  "             } else {\n"
  "                     vReflect = reflect( cameraToVertex, normal );\n"
  "             }\n"

  "             vec4 cubeColor = textureCube( tCube, vec3( -vReflect.x, vReflect.yz ) );\n"

  "             #ifdef GAMMA_INPUT\n"
  "                     cubeColor.xyz *= cubeColor.xyz;\n"
  "             #endif\n"

  "             gl_FragColor.xyz = mix( gl_FragColor.xyz, cubeColor.xyz, specularTex.r * reflectivity );\n"
  "     }\n"

  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/linear_to_gamma_fragment.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/fog_fragment.glsl\"\n"

  "}";

static const char *normalmap_vertex_shader =
  "attribute vec4 tangent;\n"
  "uniform vec2 uOffset;\n"
  "uniform vec2 uRepeat;\n"
  "uniform bool enableDisplacement;\n"

  "#ifdef VERTEX_TEXTURES\n"
  "     uniform sampler2D tDisplacement;\n"
  "     uniform float uDisplacementScale;\n"
  "     uniform float uDisplacementBias;\n"
  "#endif\n"

  "varying vec3 vTangent;\n"
  "varying vec3 vBinormal;\n"
  "varying vec3 vNormal;\n"
  "varying vec2 vUv;\n"

  "varying vec3 vWorldPosition;\n"
  "varying vec3 vViewPosition;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/shadowmap_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinbase_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinnormal_vertex.glsl\"\n"

  // normal, tangent and binormal vectors
  "     #ifdef USE_SKINNING\n"
  "             vNormal = normalize( normalMatrix * skinnedNormal.xyz );\n"
  "             vec4 skinnedTangent = skinMatrix * vec4( tangent.xyz, 0.0 );\n"
  "             vTangent = normalize( normalMatrix * skinnedTangent.xyz );\n"
  "     #else\n"
  "             vNormal = normalize( normalMatrix * normal );\n"
  "             vTangent = normalize( normalMatrix * tangent.xyz );\n"
  "     #endif\n"

  "     vBinormal = normalize( cross( vNormal, vTangent ) * tangent.w );\n"
  "     vUv = uv * uRepeat + uOffset;\n"

  // displacement mapping

  "     vec3 displacedPosition;\n"

  "     #ifdef VERTEX_TEXTURES\n"
  "             if ( enableDisplacement ) {\n"
  "                     vec3 dv = texture2D( tDisplacement, uv ).xyz;\n"
  "                     float df = uDisplacementScale * dv.x + uDisplacementBias;\n"
  "                     displacedPosition = position + normalize( normal ) * df;\n"
  "             } else {\n"
  "                     #ifdef USE_SKINNING\n"
  "                             vec4 skinVertex = bindMatrix * vec4( position, 1.0 );\n"

  "                             vec4 skinned = vec4( 0.0 );\n"
  "                             skinned += boneMatX * skinVertex * skinWeight.x;\n"
  "                             skinned += boneMatY * skinVertex * skinWeight.y;\n"
  "                             skinned += boneMatZ * skinVertex * skinWeight.z;\n"
  "                             skinned += boneMatW * skinVertex * skinWeight.w;\n"
  "                             skinned  = bindMatrixInverse * skinned;\n"

  "                             displacedPosition = skinned.xyz;\n"
  "                     #else\n"
  "                             displacedPosition = position;\n"
  "                     #endif\n"
  "             }\n"
  "     #else\n"
  "             #ifdef USE_SKINNING\n"
  "                     vec4 skinVertex = bindMatrix * vec4( position, 1.0 );\n"

  "                     vec4 skinned = vec4( 0.0 );\n"
  "                     skinned += boneMatX * skinVertex * skinWeight.x;\n"
  "                     skinned += boneMatY * skinVertex * skinWeight.y;\n"
  "                     skinned += boneMatZ * skinVertex * skinWeight.z;\n"
  "                     skinned += boneMatW * skinVertex * skinWeight.w;\n"
  "                     skinned  = bindMatrixInverse * skinned;\n"

  "                     displacedPosition = skinned.xyz;\n"
  "             #else\n"
  "                     displacedPosition = position;\n"
  "             #endif\n"
  "     #endif\n"

  //
  "     vec4 mvPosition = modelViewMatrix * vec4( displacedPosition, 1.0 );\n"
  "     vec4 worldPosition = modelMatrix * vec4( displacedPosition, 1.0 );\n"

  "     gl_Position = projectionMatrix * mvPosition;\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"

  //
  "     vWorldPosition = worldPosition.xyz;\n"
  "     vViewPosition = -mvPosition.xyz;\n"

  // shadows
  "     #ifdef USE_SHADOWMAP\n"
  "             for( int i = 0; i < MAX_SHADOWS; i ++ ) {\n"
  "                     vShadowCoord[ i ] = shadowMatrix[ i ] * worldPosition;\n"
  "             }\n"
  "     #endif\n"
  "}";


static float one_matrix3[9] = { 1, 0, 0,
                                0, 1, 0,
                                0, 0, 1};
static const char *background_uniform_libs[] = { NULL };
static GthreeUniformsDefinition background_uniforms[] = {
  {"uvTransform", GTHREE_UNIFORM_TYPE_MATRIX3, &one_matrix3},
  {"t2D", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
};

static const char *background_vertex_shader =
  "#include \"/org/gnome/gthree/shader_chunks/background_vertex.glsl\"\n";

static const char *background_fragment_shader =
  "#include \"/org/gnome/gthree/shader_chunks/background_fragment.glsl\"\n";

/* -------------------------------------------------------------------------
//      Cube map shader
------------------------------------------------------------------------- */

static const char *cube_uniform_libs[] = { NULL };
static GthreeUniformsDefinition cube_uniforms[] = {
  {"tCube", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
  {"tFlip", GTHREE_UNIFORM_TYPE_FLOAT, &fm1},
};

static const char *cube_vertex_shader =
  "varying vec3 vWorldPosition;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"
  "     vWorldPosition = transformDirection( position, modelMatrix );\n"
  "     gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"
  "}";

static const char *cube_fragment_shader =
  "uniform samplerCube tCube;\n"
  "uniform float tFlip;\n"

  "varying vec3 vWorldPosition;\n"

  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "void main() {\n"
  "     gl_FragColor = textureCube( tCube, vec3( tFlip * vWorldPosition.x, vWorldPosition.yz ) );\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "}";

/* Depth encoding into RGBA texture
 *
 * based on SpiderGL shadow map example
 * http://spidergl.org/example.php?id=6
 *
 * originally from
 * http://www.gamedev.net/topic/442138-packing-a-float-into-a-a8r8g8b8-texture-shader/page__whichpage__1%25EF%25BF%25BD
 *
 * see also
 * http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
 */

static const char *depthRGBA_uniform_libs[] = { NULL };
static GthreeUniformsDefinition depthRGBA_uniforms[] = {
};
static const char *depthRGBA_vertex_shader =
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_pars_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_vertex.glsl\"\n"

  "void main() {\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinbase_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/morphtarget_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/skinning_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/default_vertex.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_vertex.glsl\"\n"
  "}";

static const char *depthRGBA_fragment_shader =
  "#include \"/org/gnome/gthree/shader_chunks/common.glsl\"\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_pars_fragment.glsl\"\n"

  "vec4 pack_depth( const in float depth ) {\n"
  "     const vec4 bit_shift = vec4( 256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0 );\n"
  "     const vec4 bit_mask = vec4( 0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0 );\n"
  "     vec4 res = mod( depth * bit_shift * vec4( 255 ), vec4( 256 ) ) / vec4( 255 );\n" // "   vec4 res = fract( depth * bit_shift );\n"
  "     res -= res.xxyz * bit_mask;\n"
  "     return res;\n"
  "}\n"

  "void main() {\n"
  "#include \"/org/gnome/gthree/shader_chunks/logdepthbuf_fragment.glsl\"\n"
  "     #ifdef USE_LOGDEPTHBUF_EXT\n"
  "             gl_FragData[ 0 ] = pack_depth( gl_FragDepthEXT );\n"
  "     #else\n"
  "             gl_FragData[ 0 ] = pack_depth( gl_FragCoord.z );\n"
  "     #endif\n"
  //"gl_FragData[ 0 ] = pack_depth( gl_FragCoord.z / gl_FragCoord.w );\n"
  //"float z = ( ( gl_FragCoord.z / gl_FragCoord.w ) - 3.0 ) / ( 4000.0 - 3.0 );\n"
  //"gl_FragData[ 0 ] = pack_depth( z );\n"
  //"gl_FragData[ 0 ] = vec4( z, z, z, 1.0 );\n"
  "}";

static GthreeShader *basic, *lambert, *phong, *particle_basic, *dashed;
static GthreeShader *depth, *normal, *normalmap, *background, *cube, *depthRGBA;

static void
gthree_shader_init_libs ()
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  basic = gthree_shader_new_from_definitions (basic_uniform_libs,
                                              NULL, 0,
                                              basic_vertex_shader, basic_fragment_shader);
  lambert = gthree_shader_new_from_definitions (lambert_uniform_libs,
                                                lambert_uniforms, G_N_ELEMENTS (lambert_uniforms),
                                                lambert_vertex_shader, lambert_fragment_shader);
  phong = gthree_shader_new_from_definitions (phong_uniform_libs,
                                              phong_uniforms, G_N_ELEMENTS (phong_uniforms),
                                              phong_vertex_shader, phong_fragment_shader);
  particle_basic = gthree_shader_new_from_definitions (particle_basic_uniform_libs,
                                                       NULL, 0,
                                                       particle_basic_vertex_shader, particle_basic_fragment_shader);
  dashed = gthree_shader_new_from_definitions (dashed_uniform_libs,
                                               dashed_uniforms, G_N_ELEMENTS (dashed_uniforms),
                                               dashed_vertex_shader, dashed_fragment_shader);
  depth = gthree_shader_new_from_definitions (depth_uniform_libs,
                                              depth_uniforms, G_N_ELEMENTS (depth_uniforms),
                                              depth_vertex_shader, depth_fragment_shader);
  normal = gthree_shader_new_from_definitions (normal_uniform_libs,
                                               normal_uniforms, G_N_ELEMENTS (normal_uniforms),
                                               normal_vertex_shader, normal_fragment_shader);
  normalmap = gthree_shader_new_from_definitions (normalmap_uniform_libs,
                                                  normalmap_uniforms, G_N_ELEMENTS (normalmap_uniforms),
                                                  normalmap_vertex_shader, normalmap_fragment_shader);
  background = gthree_shader_new_from_definitions (background_uniform_libs,
                                                   background_uniforms, G_N_ELEMENTS (background_uniforms),
                                                   background_vertex_shader, background_fragment_shader);
  cube = gthree_shader_new_from_definitions (cube_uniform_libs,
                                             cube_uniforms, G_N_ELEMENTS (cube_uniforms),
                                             cube_vertex_shader, cube_fragment_shader);
  depthRGBA = gthree_shader_new_from_definitions (depthRGBA_uniform_libs,
                                                  depthRGBA_uniforms, G_N_ELEMENTS (depthRGBA_uniforms),
                                                  depthRGBA_vertex_shader, depthRGBA_fragment_shader);

  initialized = TRUE;
}

GthreeShader *
gthree_get_shader_from_library (const char *name)
{
  gthree_shader_init_libs ();

  if (strcmp (name, "basic") == 0)
    return basic;

  if (strcmp (name, "lambert") == 0)
    return lambert;

  if (strcmp (name, "phong") == 0)
    return phong;

  if (strcmp (name, "particle_basic") == 0)
    return particle_basic;

  if (strcmp (name, "dashed") == 0)
    return dashed;

  if (strcmp (name, "depth") == 0)
    return depth;

  if (strcmp (name, "normal") == 0)
    return normal;

  if (strcmp (name, "normalmap") == 0)
    return normalmap;

  if (strcmp (name, "background") == 0)
    return background;

  if (strcmp (name, "cube") == 0)
    return cube;

  if (strcmp (name, "depthRGBA") == 0)
    return depthRGBA;

  g_warning ("can't find shader library %s\n", name);
  return NULL;
}

GthreeShader *
gthree_clone_shader_from_library (const char *name)
{
  GthreeShader *orig;

  orig = gthree_get_shader_from_library (name);
  if (orig == NULL)
    return NULL;

  return gthree_shader_clone (orig);
}
