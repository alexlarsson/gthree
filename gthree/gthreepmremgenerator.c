#include <math.h>
#include <epoxy/gl.h>

#include "gthreepmremgeneratorprivate.h"
#include "gthreecubetexture.h"
#include "gthreeprivate.h"
#include "gthreeprogramprivate.h"
#include "gthree-resources.h"

#define PMREM_SHADER_PATH "/org/gnome/gthree/shader_lib/"

#define LOD_MIN 4
#define MIN_TILE_SIZE 16
#define EXTRA_LOD_COUNT 6
#define GGX_SAMPLES 256

typedef struct {
  GthreeRenderer *renderer;

  GBytes *vert_src;
  GBytes *cubemap_frag_src;
  GBytes *ggx_frag_src;

  /* Size-independent (built once) */
  guint vert_shader;
  guint cubemap_program;

  /* Size-dependent (rebuilt when cube_size changes) */
  int cube_size;
  guint ggx_program;
  guint *lod_vaos;
  guint *lod_vbos;
  int total_lods;
} GthreePMREMGeneratorPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreePMREMGenerator, gthree_pmrem_generator, G_TYPE_OBJECT)

static void
cleanup_sized_resources (GthreePMREMGeneratorPrivate *priv)
{
  if (priv->ggx_program)
    {
      glDeleteProgram (priv->ggx_program);
      priv->ggx_program = 0;
    }
  if (priv->lod_vaos)
    {
      glDeleteVertexArrays (priv->total_lods, priv->lod_vaos);
      glDeleteBuffers (priv->total_lods, priv->lod_vbos);
      g_free (priv->lod_vaos);
      g_free (priv->lod_vbos);
      priv->lod_vaos = NULL;
      priv->lod_vbos = NULL;
    }
  priv->cube_size = 0;
  priv->total_lods = 0;
}

static void
gthree_pmrem_generator_finalize (GObject *obj)
{
  GthreePMREMGenerator *gen = GTHREE_PMREM_GENERATOR (obj);
  GthreePMREMGeneratorPrivate *priv = gthree_pmrem_generator_get_instance_private (gen);

  cleanup_sized_resources (priv);

  if (priv->vert_shader)
    glDeleteShader (priv->vert_shader);
  if (priv->cubemap_program)
    glDeleteProgram (priv->cubemap_program);

  g_clear_pointer (&priv->vert_src, g_bytes_unref);
  g_clear_pointer (&priv->cubemap_frag_src, g_bytes_unref);
  g_clear_pointer (&priv->ggx_frag_src, g_bytes_unref);

  G_OBJECT_CLASS (gthree_pmrem_generator_parent_class)->finalize (obj);
}

static void
gthree_pmrem_generator_init (GthreePMREMGenerator *gen)
{
  GthreePMREMGeneratorPrivate *priv = gthree_pmrem_generator_get_instance_private (gen);

  gthree_register_resource ();

  priv->vert_src = g_resources_lookup_data (PMREM_SHADER_PATH "pmrem_cubemap_vert.glsl", 0, NULL);
  priv->cubemap_frag_src = g_resources_lookup_data (PMREM_SHADER_PATH "pmrem_cubemap_frag.glsl", 0, NULL);
  priv->ggx_frag_src = g_resources_lookup_data (PMREM_SHADER_PATH "pmrem_ggx_frag.glsl", 0, NULL);
}

static void
gthree_pmrem_generator_class_init (GthreePMREMGeneratorClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_pmrem_generator_finalize;
}

GthreePMREMGenerator *
gthree_pmrem_generator_new (GthreeRenderer *renderer)
{
  GthreePMREMGenerator *gen;
  GthreePMREMGeneratorPrivate *priv;

  gen = g_object_new (GTHREE_TYPE_PMREM_GENERATOR, NULL);
  priv = gthree_pmrem_generator_get_instance_private (gen);
  priv->renderer = renderer;

  return gen;
}

static guint
link_program (guint vert, guint frag)
{
  guint program = glCreateProgram ();
  GLint status;

  glAttachShader (program, vert);
  glAttachShader (program, frag);

  glBindAttribLocation (program, 0, "position");
  glBindAttribLocation (program, 1, "uv");
  glBindAttribLocation (program, 2, "faceIndex");

  glLinkProgram (program);
  glGetProgramiv (program, GL_LINK_STATUS, &status);
  if (!status)
    {
      char log[1024];
      glGetProgramInfoLog (program, sizeof (log), NULL, log);
      g_warning ("PMREM program link error: %s", log);
      glDeleteProgram (program);
      return 0;
    }

  glDetachShader (program, vert);
  glDetachShader (program, frag);

  return program;
}

static void
ensure_gpu_resources (GthreePMREMGeneratorPrivate *priv,
                      int cube_size, int lod_max, int total_lods,
                      int tex_width, int tex_height,
                      int *size_lods)
{
  if (priv->cube_size == cube_size)
    return;

  if (!priv->vert_src || !priv->cubemap_frag_src || !priv->ggx_frag_src)
    {
      g_warning ("PMREM shader resources not loaded");
      return;
    }

  cleanup_sized_resources (priv);

  priv->cube_size = cube_size;
  priv->total_lods = total_lods;

  gboolean is_gles = !epoxy_is_desktop_gl ();
  const char *frag_header;

  if (is_gles)
    frag_header =
      "#version 300 es\n"
      "precision highp float;\n"
      "precision highp int;\n"
      "#define varying in\n"
      "#define textureCube texture\n"
      "#define texture2D texture\n"
      "#define gl_FragColor pmrem_FragColor\n"
      "out vec4 pmrem_FragColor;\n";
  else
    frag_header =
      "#version 150\n"
      "#define varying in\n"
      "#define textureCube texture\n"
      "#define texture2D texture\n"
      "#define gl_FragColor pmrem_FragColor\n"
      "out vec4 pmrem_FragColor;\n";

  /* Build cubemap program once (size-independent) */
  if (!priv->cubemap_program)
    {
      const char *vert_header;

      if (is_gles)
        vert_header =
          "#version 300 es\n"
          "precision highp float;\n"
          "precision highp int;\n"
          "#define attribute in\n"
          "#define varying out\n"
          "#define textureCube texture\n";
      else
        vert_header =
          "#version 150\n"
          "#define attribute in\n"
          "#define varying out\n"
          "#define textureCube texture\n";

      const char *vert_src_raw = g_bytes_get_data (priv->vert_src, NULL);
      const char *cubemap_frag_src_raw = g_bytes_get_data (priv->cubemap_frag_src, NULL);

      g_autofree char *vert_src = g_strconcat (vert_header, vert_src_raw, NULL);
      g_autofree char *cubemap_frag_src = g_strconcat (frag_header, cubemap_frag_src_raw, NULL);

      priv->vert_shader = gthree_create_shader (GL_VERTEX_SHADER, vert_src);
      guint cubemap_frag = gthree_create_shader (GL_FRAGMENT_SHADER, cubemap_frag_src);
      priv->cubemap_program = link_program (priv->vert_shader, cubemap_frag);
      glDeleteShader (cubemap_frag);
    }

  /* Build GGX program (size-dependent due to texel size and max mip defines) */
  const char *ggx_frag_src_raw = g_bytes_get_data (priv->ggx_frag_src, NULL);

  char texel_w[G_ASCII_DTOSTR_BUF_SIZE], texel_h[G_ASCII_DTOSTR_BUF_SIZE];
  g_ascii_dtostr (texel_w, sizeof (texel_w), 1.0 / tex_width);
  g_ascii_dtostr (texel_h, sizeof (texel_h), 1.0 / tex_height);

  g_autofree char *ggx_frag_src = g_strdup_printf (
    "%s"
    "#define GGX_SAMPLES %d\n"
    "#define CUBEUV_TEXEL_WIDTH %s\n"
    "#define CUBEUV_TEXEL_HEIGHT %s\n"
    "#define CUBEUV_MAX_MIP %d.0\n"
    "%s",
    frag_header,
    GGX_SAMPLES,
    texel_w, texel_h,
    lod_max,
    ggx_frag_src_raw);

  guint ggx_frag = gthree_create_shader (GL_FRAGMENT_SHADER, ggx_frag_src);
  priv->ggx_program = link_program (priv->vert_shader, ggx_frag);
  glDeleteShader (ggx_frag);

  /* Create per-LOD geometry: 6 faces x 2 triangles x 3 vertices = 36 vertices
     Each vertex: position(3) + uv(2) + faceIndex(1) = 6 floats */
  priv->lod_vaos = g_new0 (guint, total_lods);
  priv->lod_vbos = g_new0 (guint, total_lods);
  glGenVertexArrays (total_lods, priv->lod_vaos);
  glGenBuffers (total_lods, priv->lod_vbos);

  for (int lod_idx = 0; lod_idx < total_lods; lod_idx++)
    {
      int face_size = size_lods[lod_idx];
      float texel_size = 1.0f / (face_size - 2);
      float uv_min = -texel_size;
      float uv_max = 1.0f + texel_size;

      float verts[6 * 6 * 6];
      int vi = 0;

      for (int face = 0; face < 6; face++)
        {
          float x = (face % 3) * 2.0f / 3.0f - 1.0f;
          float y = face > 2 ? 0.0f : -1.0f;
          float w = 2.0f / 3.0f;
          float h = 1.0f;
          float fi = (float)face;

          /* Triangle 1 */
          verts[vi++] = x;     verts[vi++] = y;     verts[vi++] = 0;
          verts[vi++] = uv_min; verts[vi++] = uv_min; verts[vi++] = fi;
          verts[vi++] = x + w; verts[vi++] = y;     verts[vi++] = 0;
          verts[vi++] = uv_max; verts[vi++] = uv_min; verts[vi++] = fi;
          verts[vi++] = x + w; verts[vi++] = y + h; verts[vi++] = 0;
          verts[vi++] = uv_max; verts[vi++] = uv_max; verts[vi++] = fi;

          /* Triangle 2 */
          verts[vi++] = x;     verts[vi++] = y;     verts[vi++] = 0;
          verts[vi++] = uv_min; verts[vi++] = uv_min; verts[vi++] = fi;
          verts[vi++] = x + w; verts[vi++] = y + h; verts[vi++] = 0;
          verts[vi++] = uv_max; verts[vi++] = uv_max; verts[vi++] = fi;
          verts[vi++] = x;     verts[vi++] = y + h; verts[vi++] = 0;
          verts[vi++] = uv_min; verts[vi++] = uv_max; verts[vi++] = fi;
        }

      glBindVertexArray (priv->lod_vaos[lod_idx]);
      glBindBuffer (GL_ARRAY_BUFFER, priv->lod_vbos[lod_idx]);
      glBufferData (GL_ARRAY_BUFFER, sizeof (verts), verts, GL_STATIC_DRAW);

      GLint stride = 6 * sizeof (float);
      glEnableVertexAttribArray (0);
      glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
      glEnableVertexAttribArray (1);
      glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
      glEnableVertexAttribArray (2);
      glVertexAttribPointer (2, 1, GL_FLOAT, GL_FALSE, stride, (void *)(5 * sizeof(float)));

      glBindVertexArray (0);
      glBindBuffer (GL_ARRAY_BUFFER, 0);
    }
}

/**
 * gthree_pmrem_generator_from_cubemap:
 *
 * GPU-accelerated PMREM generation. Renders fullscreen quads through
 * GGX filter shaders into a CubeUV layout texture.
 *
 * Returns: (transfer full): A CubeUV texture
 */
GthreeTexture *
gthree_pmrem_generator_from_cubemap (GthreePMREMGenerator *generator,
                                     GthreeTexture        *cube_texture,
                                     int                   cube_size)
{
  GthreePMREMGeneratorPrivate *priv = gthree_pmrem_generator_get_instance_private (generator);

  int lod_max = (int)floor (log2 (cube_size));
  cube_size = 1 << lod_max;
  int tex_width = 3 * MAX (cube_size, MIN_TILE_SIZE * 7);
  int tex_height = 4 * cube_size;
  int total_lods = lod_max - LOD_MIN + 1 + EXTRA_LOD_COUNT;

  int *size_lods = g_newa (int, total_lods);
  {
    int lod = lod_max;
    for (int i = 0; i < total_lods; i++)
      {
        size_lods[i] = 1 << lod;
        if (lod > LOD_MIN)
          lod--;
      }
  }

  ensure_gpu_resources (priv, cube_size, lod_max, total_lods,
                        tex_width, tex_height, size_lods);

  /* Save GL state */
  GLint old_fbo, old_viewport[4], old_program, old_vao;
  GLint old_active_texture;
  GLfloat old_clear_color[4];
  GLboolean old_depth_test, old_blend, old_scissor_test;
  glGetIntegerv (GL_FRAMEBUFFER_BINDING, &old_fbo);
  glGetIntegerv (GL_VIEWPORT, old_viewport);
  glGetIntegerv (GL_CURRENT_PROGRAM, &old_program);
  glGetIntegerv (GL_VERTEX_ARRAY_BINDING, &old_vao);
  glGetIntegerv (GL_ACTIVE_TEXTURE, &old_active_texture);
  glGetFloatv (GL_COLOR_CLEAR_VALUE, old_clear_color);
  old_depth_test = glIsEnabled (GL_DEPTH_TEST);
  old_blend = glIsEnabled (GL_BLEND);
  old_scissor_test = glIsEnabled (GL_SCISSOR_TEST);

  glDisable (GL_DEPTH_TEST);
  glDisable (GL_BLEND);
  glEnable (GL_SCISSOR_TEST);
  glActiveTexture (GL_TEXTURE0);

  /* Create the two render targets (FBO + texture) */
  guint cubeuv_tex, cubeuv_fbo;
  guint pingpong_tex, pingpong_fbo;

  glGenTextures (1, &cubeuv_tex);
  glBindTexture (GL_TEXTURE_2D, cubeuv_tex);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA16F, tex_width, tex_height, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenFramebuffers (1, &cubeuv_fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, cubeuv_fbo);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cubeuv_tex, 0);

  glViewport (0, 0, tex_width, tex_height);
  glScissor (0, 0, tex_width, tex_height);
  glClearColor (0, 0, 0, 1);
  glClear (GL_COLOR_BUFFER_BIT);

  glGenTextures (1, &pingpong_tex);
  glBindTexture (GL_TEXTURE_2D, pingpong_tex);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA16F, tex_width, tex_height, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenFramebuffers (1, &pingpong_fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, pingpong_fbo);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_tex, 0);
  glClearColor (0, 0, 0, 1);
  glClear (GL_COLOR_BUFFER_BIT);

  /* Ensure cube texture is realized */
  gthree_texture_realize (cube_texture, priv->renderer);
  gthree_texture_load (cube_texture, priv->renderer, 0);

  /* Step 1: Render cubemap into CubeUV LOD 0 */
  {
    int size = size_lods[0];
    int vp_w = 3 * size;
    int vp_h = 2 * size;
    int vp_x = 0;
    int vp_y = 4 * (cube_size - size);

    glBindFramebuffer (GL_FRAMEBUFFER, cubeuv_fbo);
    glViewport (vp_x, vp_y, vp_w, vp_h);
    glScissor (vp_x, vp_y, vp_w, vp_h);

    glUseProgram (priv->cubemap_program);

    GLint loc_env = glGetUniformLocation (priv->cubemap_program, "envMap");
    GLint loc_flip = glGetUniformLocation (priv->cubemap_program, "flipEnvMap");

    glUniform1i (loc_env, 0);
    glUniform1f (loc_flip, GTHREE_IS_CUBE_TEXTURE (cube_texture) ? -1.0f : 1.0f);

    gthree_texture_bind (cube_texture, priv->renderer, 0, GL_TEXTURE_CUBE_MAP);

    glBindVertexArray (priv->lod_vaos[0]);
    glDrawArrays (GL_TRIANGLES, 0, 36);
  }

  /* Step 2: GGX filter for each subsequent LOD level */
  for (int lod_out = 1; lod_out < total_lods; lod_out++)
    {
      int lod_in = lod_out - 1;
      int out_size = size_lods[lod_out];

      float target_roughness = (float)lod_out / (float)(total_lods - 1);
      float source_roughness = (float)lod_in / (float)(total_lods - 1);
      float incremental_roughness = sqrtf (target_roughness * target_roughness - source_roughness * source_roughness);
      float blur_strength = target_roughness * 1.25f;
      float adjusted_roughness = incremental_roughness * blur_strength;

      int x = 3 * out_size * (lod_out > lod_max - LOD_MIN ? lod_out - lod_max + LOD_MIN : 0);
      int y = 4 * (cube_size - out_size);
      int vp_w = 3 * out_size;
      int vp_h = 2 * out_size;

      GLint loc_env = glGetUniformLocation (priv->ggx_program, "envMap");
      GLint loc_rough = glGetUniformLocation (priv->ggx_program, "roughness");
      GLint loc_mip = glGetUniformLocation (priv->ggx_program, "mipInt");

      glUseProgram (priv->ggx_program);
      glUniform1i (loc_env, 0);

      /* Pass 1: Filter from cubeUV into pingPong */
      glBindFramebuffer (GL_FRAMEBUFFER, pingpong_fbo);
      glViewport (x, y, vp_w, vp_h);
      glScissor (x, y, vp_w, vp_h);

      glBindTexture (GL_TEXTURE_2D, cubeuv_tex);
      glUniform1f (loc_rough, adjusted_roughness);
      glUniform1f (loc_mip, (float)(lod_max - lod_in));

      glBindVertexArray (priv->lod_vaos[lod_out]);
      glDrawArrays (GL_TRIANGLES, 0, 36);

      /* Pass 2: Copy from pingPong back to cubeUV */
      glBindFramebuffer (GL_FRAMEBUFFER, cubeuv_fbo);
      glViewport (x, y, vp_w, vp_h);
      glScissor (x, y, vp_w, vp_h);

      glBindTexture (GL_TEXTURE_2D, pingpong_tex);
      glUniform1f (loc_rough, 0.0f);
      glUniform1f (loc_mip, (float)(lod_max - lod_out));

      glDrawArrays (GL_TRIANGLES, 0, 36);
    }

  /* Clean up render targets */
  glDeleteFramebuffers (1, &pingpong_fbo);
  glDeleteTextures (1, &pingpong_tex);
  glDeleteFramebuffers (1, &cubeuv_fbo);

  /* Restore GL state */
  glBindVertexArray (old_vao);
  glUseProgram (old_program);
  glBindFramebuffer (GL_FRAMEBUFFER, old_fbo);
  glViewport (old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
  glActiveTexture (old_active_texture);
  glClearColor (old_clear_color[0], old_clear_color[1], old_clear_color[2], old_clear_color[3]);
  if (old_depth_test) glEnable (GL_DEPTH_TEST); else glDisable (GL_DEPTH_TEST);
  if (old_blend) glEnable (GL_BLEND); else glDisable (GL_BLEND);
  if (old_scissor_test) glEnable (GL_SCISSOR_TEST); else glDisable (GL_SCISSOR_TEST);

  /* Wrap the GL texture in a GthreeTexture */
  GthreeTexture *result = g_object_new (GTHREE_TYPE_TEXTURE, NULL);
  gthree_texture_set_mapping (result, GTHREE_MAPPING_CUBE_UV_REFLECTION);
  gthree_texture_set_generate_mipmaps (result, FALSE);
  gthree_texture_set_mag_filter (result, GTHREE_FILTER_LINEAR);
  gthree_texture_set_min_filter (result, GTHREE_FILTER_LINEAR);
  gthree_texture_set_flip_y (result, FALSE);
  gthree_texture_set_name (result, "PMREM.cubeUv");

  gthree_texture_set_gl_texture (result, priv->renderer, cubeuv_tex);
  gthree_resource_mark_clean_for (GTHREE_RESOURCE (result), priv->renderer);

  {
    float *meta = g_new (float, 3);
    meta[0] = 1.0f / tex_width;
    meta[1] = 1.0f / tex_height;
    meta[2] = (float)lod_max;
    g_object_set_data_full (G_OBJECT (result), "cubeuv-meta", meta, g_free);
  }

  return result;
}
