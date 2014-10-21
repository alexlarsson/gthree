#include <math.h>
#include <epoxy/gl.h>

#include "gthreeshader.h"


typedef struct {
  int free_shaders;
} GthreeShaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeShader, gthree_shader, G_TYPE_OBJECT);

static void gthree_shader_init_libs ();

GthreeShader *
gthree_shader_new ()
{
  GthreeShader *shader;

  shader = g_object_new (gthree_shader_get_type (),
                          NULL);

  return shader;
}

static void
gthree_shader_init (GthreeShader *shader)
{
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  priv->free_shaders = TRUE;
}

static void
gthree_shader_finalize (GObject *obj)
{
  GthreeShader *shader = GTHREE_SHADER (obj);
  GthreeShaderPrivate *priv = gthree_shader_get_instance_private (shader);

  g_clear_object (&shader->uniforms);
  if (priv->free_shaders)
    {
      g_clear_pointer (&shader->vertex_shader, g_free);
      g_clear_pointer (&shader->fragment_shader, g_free);
    }

  G_OBJECT_CLASS (gthree_shader_parent_class)->finalize (obj);
}

static void
gthree_shader_class_init (GthreeShaderClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_shader_finalize;

  gthree_shader_init_libs ();
}

static GthreeShader *basic;

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
  GthreeShader *shader = gthree_shader_new ();
  GthreeUniforms *uniforms;
  int i;

  shader->uniforms = gthree_uniforms_new ();
  for (i = 0; lib_uniforms[i] != NULL; i++)
    {
      uniforms = gthree_get_uniforms_from_library (lib_uniforms[i]);
      gthree_uniforms_merge (shader->uniforms, uniforms);
    }

  if (uniform_defs)
    {
      uniforms = gthree_uniforms_new_from_definitions (uniform_defs, defs_len);
      gthree_uniforms_merge (shader->uniforms, uniforms);
      g_object_unref (uniforms);
    }

  shader->vertex_shader = parse_text_with_includes (vertex_shader);

  shader->fragment_shader = parse_text_with_includes (fragment_shader);

  return shader;
}

static const char *basic_uniform_libs[] = { "common", "fog", "shadowmap", NULL };
static const char *basic_vertex_shader =
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
  "	#ifdef USE_ENVMAP\n"
       "#include \"/org/gnome/gthree/shader_chunks/morphnormal_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/skinnormal_vertex.glsl\"\n"
       "#include \"/org/gnome/gthree/shader_chunks/defaultnormal_vertex.glsl\"\n"
  "	#endif\n"
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
  "	gl_FragColor = vec4( diffuse, opacity );\n"
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


static GthreeShader *basic;

static void
gthree_shader_init_libs ()
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;
  basic = gthree_shader_new_from_definitions (basic_uniform_libs, NULL, 0, basic_vertex_shader, basic_fragment_shader);

  initialized = TRUE;
}

GthreeShader *
gthree_get_shader_from_library (const char *name)
{
  gthree_shader_init_libs ();

  if (strcmp (name, "basic") == 0)
    return basic;

  g_warning ("can't find shader library %s\n", name);
  return NULL;
}

GthreeShader *
gthree_clone_shader_from_library (const char *name)
{
  GthreeShader *shader = gthree_get_shader_from_library (name);
  GthreeShader *clone;
  GthreeShaderPrivate *priv;

  if (shader == NULL)
    return NULL;

  clone = gthree_get_shader_from_library (name);

  clone->uniforms = gthree_uniforms_clone (shader->uniforms);
  clone->vertex_shader = shader->vertex_shader;
  clone->fragment_shader = shader->fragment_shader;

  /* Don't free these, as they point to the library ones */
  priv = gthree_shader_get_instance_private (clone);
  priv->free_shaders = FALSE;

  return clone;
}
