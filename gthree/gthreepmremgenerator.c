#include <math.h>
#include <string.h>
#include <epoxy/gl.h>

#include "gthreepmremgenerator.h"
#include "gthreeprivate.h"

#define LOD_MIN 4
#define MIN_TILE_SIZE 16
#define EXTRA_LOD_COUNT 6

typedef struct {
  GthreeRenderer *renderer;
  guint gl_texture;
  int texture_width;
  int texture_height;
} GthreePMREMGeneratorPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreePMREMGenerator, gthree_pmrem_generator, G_TYPE_OBJECT)

static void
gthree_pmrem_generator_finalize (GObject *obj)
{
  GthreePMREMGenerator *gen = GTHREE_PMREM_GENERATOR (obj);
  GthreePMREMGeneratorPrivate *priv = gthree_pmrem_generator_get_instance_private (gen);

  g_clear_object (&priv->renderer);

  if (priv->gl_texture)
    {
      glDeleteTextures (1, &priv->gl_texture);
      priv->gl_texture = 0;
    }

  G_OBJECT_CLASS (gthree_pmrem_generator_parent_class)->finalize (obj);
}

static void
gthree_pmrem_generator_init (GthreePMREMGenerator *gen)
{
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
  priv->renderer = g_object_ref (renderer);

  return gen;
}

void
gthree_pmrem_generator_dispose (GthreePMREMGenerator *generator)
{
  GthreePMREMGeneratorPrivate *priv = gthree_pmrem_generator_get_instance_private (generator);

  if (priv->gl_texture)
    {
      glDeleteTextures (1, &priv->gl_texture);
      priv->gl_texture = 0;
    }
}

static void
pixbuf_get_pixel_float (GdkPixbuf *pixbuf, int x, int y, float *rgba)
{
  int n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  const guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
  const guchar *p = pixels + y * rowstride + x * n_channels;

  rgba[0] = p[0] / 255.0f;
  rgba[1] = p[1] / 255.0f;
  rgba[2] = p[2] / 255.0f;
  rgba[3] = (n_channels == 4) ? p[3] / 255.0f : 1.0f;
}

static void
pixbuf_sample_bilinear (GdkPixbuf *pixbuf, float u, float v, float *rgba)
{
  int w = gdk_pixbuf_get_width (pixbuf);
  int h = gdk_pixbuf_get_height (pixbuf);

  float fx = u * (w - 1);
  float fy = v * (h - 1);
  int x0 = (int)floorf (fx);
  int y0 = (int)floorf (fy);
  int x1 = MIN (x0 + 1, w - 1);
  int y1 = MIN (y0 + 1, h - 1);
  x0 = MAX (x0, 0);
  y0 = MAX (y0, 0);

  float sx = fx - floorf (fx);
  float sy = fy - floorf (fy);

  float p00[4], p10[4], p01[4], p11[4];
  pixbuf_get_pixel_float (pixbuf, x0, y0, p00);
  pixbuf_get_pixel_float (pixbuf, x1, y0, p10);
  pixbuf_get_pixel_float (pixbuf, x0, y1, p01);
  pixbuf_get_pixel_float (pixbuf, x1, y1, p11);

  for (int c = 0; c < 4; c++)
    rgba[c] = (p00[c] * (1 - sx) + p10[c] * sx) * (1 - sy) +
              (p01[c] * (1 - sx) + p11[c] * sx) * sy;
}

/* PMREM face-indexing convention (RH coordinate system).
 * Given a face index and UV in [0,1], return the 3D direction.
 * Must match getDirection() in the vertex shader and
 * getFace()/getUV() in cube_uv_reflection_fragment.glsl */
static void
get_direction (float u, float v, int face, float *dir)
{
  u = 2.0f * u - 1.0f;
  v = 2.0f * v - 1.0f;
  float len;

  switch (face)
    {
    case 0: /* +X: direction = (1, v, u) => getUV gives (z, y)/|x| */
      dir[0] = 1.0f;  dir[1] = v;  dir[2] = u;
      break;
    case 1: /* +Y: direction = (-u, 1, -v) => getUV gives (-x, -z)/|y| */
      dir[0] = -u;  dir[1] = 1.0f;  dir[2] = -v;
      break;
    case 2: /* +Z: direction = (-u, v, 1) => getUV gives (-x, y)/|z| */
      dir[0] = -u;  dir[1] = v;  dir[2] = 1.0f;
      break;
    case 3: /* -X: direction = (-1, v, -u) => getUV gives (-z, y)/|x| */
      dir[0] = -1.0f;  dir[1] = v;  dir[2] = -u;
      break;
    case 4: /* -Y: direction = (-u, -1, v) => getUV gives (-x, z)/|y| */
      dir[0] = -u;  dir[1] = -1.0f;  dir[2] = v;
      break;
    case 5: /* -Z: direction = (u, v, -1) => getUV gives (x, y)/|z| */
      dir[0] = u;  dir[1] = v;  dir[2] = -1.0f;
      break;
    }

  len = sqrtf (dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
  dir[0] /= len;
  dir[1] /= len;
  dir[2] /= len;
}

/* Map a 3D direction to cubemap face + UV.
 * Must match getFace()/getUV() in cube_uv_reflection_fragment.glsl */
static int
direction_to_face_uv (const float *dir, float *out_u, float *out_v)
{
  float ax = fabsf (dir[0]);
  float ay = fabsf (dir[1]);
  float az = fabsf (dir[2]);
  int face;
  float u, v;

  if (ax > az)
    {
      if (ax > ay)
        face = dir[0] > 0.0f ? 0 : 3;
      else
        face = dir[1] > 0.0f ? 1 : 4;
    }
  else
    {
      if (az > ay)
        face = dir[2] > 0.0f ? 2 : 5;
      else
        face = dir[1] > 0.0f ? 1 : 4;
    }

  switch (face)
    {
    case 0: u = dir[2] / ax;  v = dir[1] / ax;  break;
    case 1: u = -dir[0] / ay;  v = -dir[2] / ay;  break;
    case 2: u = -dir[0] / az;  v = dir[1] / az;  break;
    case 3: u = -dir[2] / ax;  v = dir[1] / ax;  break;
    case 4: u = -dir[0] / ay;  v = dir[2] / ay;  break;
    default:
    case 5: u = dir[0] / az;  v = dir[1] / az;  break;
    }

  *out_u = 0.5f * (u + 1.0f);
  *out_v = 0.5f * (v + 1.0f);
  return face;
}

static void
sample_cubemap (GdkPixbuf **pixbufs, const float *dir, float *rgba)
{
  float u, v;
  int face = direction_to_face_uv (dir, &u, &v);

  /* Flip the v coordinate: pixbufs have (0,0) at top-left but UV has
   * (0,0) at bottom-left in the shader convention */
  v = 1.0f - v;

  pixbuf_sample_bilinear (pixbufs[face], u, v, rgba);
}

static void
write_texel (float *buffer, int tex_width, int x, int y, const float *rgba)
{
  float *p = buffer + (y * tex_width + x) * 4;
  p[0] = rgba[0];
  p[1] = rgba[1];
  p[2] = rgba[2];
  p[3] = rgba[3];
}

/* GGX importance sampling for PMREM filtering */

#define NUM_SAMPLES 128

static float
radical_inverse_vdc (guint32 bits)
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return (float)bits * 2.3283064365386963e-10f;
}

static void
hammersley (int i, int n, float *xi)
{
  xi[0] = (float)i / (float)n;
  xi[1] = radical_inverse_vdc (i);
}

static void
importance_sample_ggx (const float *xi, float roughness, const float *N, float *H)
{
  float a = roughness * roughness;
  float phi = 2.0f * G_PI * xi[0];
  float cos_theta = sqrtf ((1.0f - xi[1]) / (1.0f + (a * a - 1.0f) * xi[1]));
  float sin_theta = sqrtf (1.0f - cos_theta * cos_theta);

  /* Tangent-space half vector */
  float Ht[3] = {
    sin_theta * cosf (phi),
    sin_theta * sinf (phi),
    cos_theta
  };

  /* Build TBN from N */
  float up[3] = { 0, 1, 0 };
  if (fabsf (N[1]) > 0.999f)
    { up[0] = 1; up[1] = 0; up[2] = 0; }

  float T[3], B[3];
  /* T = normalize(cross(up, N)) */
  T[0] = up[1] * N[2] - up[2] * N[1];
  T[1] = up[2] * N[0] - up[0] * N[2];
  T[2] = up[0] * N[1] - up[1] * N[0];
  float tlen = sqrtf (T[0]*T[0] + T[1]*T[1] + T[2]*T[2]);
  T[0] /= tlen; T[1] /= tlen; T[2] /= tlen;

  /* B = cross(N, T) */
  B[0] = N[1] * T[2] - N[2] * T[1];
  B[1] = N[2] * T[0] - N[0] * T[2];
  B[2] = N[0] * T[1] - N[1] * T[0];

  /* Transform to world space: H = T * Ht.x + B * Ht.y + N * Ht.z */
  H[0] = T[0] * Ht[0] + B[0] * Ht[1] + N[0] * Ht[2];
  H[1] = T[1] * Ht[0] + B[1] * Ht[1] + N[1] * Ht[2];
  H[2] = T[2] * Ht[0] + B[2] * Ht[1] + N[2] * Ht[2];
}

static void
ggx_filter_color (GdkPixbuf **pixbufs, const float *N, float roughness, float *rgba)
{
  if (roughness < 0.01f)
    {
      float dir[3] = { -N[0], N[1], N[2] };
      sample_cubemap (pixbufs, dir, rgba);
      return;
    }

  float total_weight = 0;
  rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0;

  for (int i = 0; i < NUM_SAMPLES; i++)
    {
      float xi[2];
      hammersley (i, NUM_SAMPLES, xi);

      float H[3];
      importance_sample_ggx (xi, roughness, N, H);

      /* L = reflect(-V, H) where V = N for PMREM (view = normal) */
      float NdotH = N[0]*H[0] + N[1]*H[1] + N[2]*H[2];
      float L[3];
      L[0] = 2.0f * NdotH * H[0] - N[0];
      L[1] = 2.0f * NdotH * H[1] - N[1];
      L[2] = 2.0f * NdotH * H[2] - N[2];

      float NdotL = N[0]*L[0] + N[1]*L[1] + N[2]*L[2];
      if (NdotL > 0)
        {
          float sample_dir[3] = { -L[0], L[1], L[2] };
          float sample_rgba[4];
          sample_cubemap (pixbufs, sample_dir, sample_rgba);
          rgba[0] += sample_rgba[0] * NdotL;
          rgba[1] += sample_rgba[1] * NdotL;
          rgba[2] += sample_rgba[2] * NdotL;
          rgba[3] += sample_rgba[3] * NdotL;
          total_weight += NdotL;
        }
    }

  if (total_weight > 0)
    {
      rgba[0] /= total_weight;
      rgba[1] /= total_weight;
      rgba[2] /= total_weight;
      rgba[3] /= total_weight;
    }
}

/* Roughness for each LOD level. Must match the inverse of roughnessToMip()
   in cube_uv_reflection_fragment.glsl.

   Layout: LOD 0 is the sharpest (sigma=0, roughness=0) at filterInt=0.
   Main LODs (0 to lodMax-LOD_MIN) have decreasing face sizes with increasing
   roughness. Extra LODs (at MIN_TILE_SIZE) continue with filterInt=1,2,...
   and roughness values matching the roughnessToMip inverse at each integer
   filterInt boundary. */
static float
lod_roughness (int lod_idx, int lod_max)
{
  int num_main_lods = lod_max - LOD_MIN + 1;

  if (lod_idx < num_main_lods)
    {
      if (lod_idx == 0)
        return 0.0f;

      /* Intermediate main LODs: roughness increases as face size decreases.
         These map to filterInt=0 with varying faceSize. */
      static const float main_roughness[] = { 0.0f, 0.1f, 0.21f };
      if (lod_idx < (int)G_N_ELEMENTS (main_roughness))
        return main_roughness[lod_idx];
      return 0.21f;
    }
  else
    {
      /* Extra LODs at MIN_TILE_SIZE with filterInt = 1,2,...,6.
         Roughness increases: these are the inverse of roughnessToMip()
         at integer mip values (3, 2, 1, 0, -1, -2). */
      int extra_idx = lod_idx - num_main_lods;
      static const float extra_roughness[] = { 0.305f, 0.4f, 0.533f, 0.667f, 0.8f, 1.0f };
      if (extra_idx < (int)G_N_ELEMENTS (extra_roughness))
        return extra_roughness[extra_idx];
      return 1.0f;
    }
}

/**
 * gthree_pmrem_generator_from_cubemap:
 *
 * Returns: (transfer full):
 */
GthreeTexture *
gthree_pmrem_generator_from_cubemap (GthreePMREMGenerator *generator,
                                     GthreeCubeTexture    *cubemap)
{
  GthreePMREMGeneratorPrivate *priv = gthree_pmrem_generator_get_instance_private (generator);
  GdkPixbuf *pixbufs[6];
  int cube_size, lod_max;
  int tex_width, tex_height;
  float *buffer;
  GthreeTexture *result;
  int total_lods;
  int *size_lods;

  for (int i = 0; i < 6; i++)
    pixbufs[i] = gthree_cube_texture_get_face_pixbuf (cubemap, i);

  cube_size = gdk_pixbuf_get_width (pixbufs[0]);

  lod_max = (int)floor (log2 (cube_size));
  cube_size = 1 << lod_max;

  /* Match three.js: width = 3 * max(cubeSize, 16*7), height = 4 * cubeSize */
  tex_width = 3 * MAX (cube_size, MIN_TILE_SIZE * 7);
  tex_height = 4 * cube_size;

  /* Number of LOD levels: from lodMax down to LOD_MIN, plus EXTRA_LOD_COUNT
   * extra levels at MIN_TILE_SIZE resolution */
  total_lods = lod_max - LOD_MIN + 1 + EXTRA_LOD_COUNT;
  size_lods = g_new (int, total_lods);

  {
    int lod = lod_max;
    for (int i = 0; i < total_lods; i++)
      {
        size_lods[i] = 1 << lod;
        if (lod > LOD_MIN)
          lod--;
      }
  }

  buffer = g_new0 (float, tex_width * tex_height * 4);

  /* For each LOD level, render 6 faces into the CubeUV layout.
   *
   * Layout (matching bilinearCubeUV in the shader):
   *   - Each face tile is faceSize x faceSize pixels, with a 1-pixel border
   *     on each side for seamless bilinear filtering. The face content
   *     occupies (faceSize - 2) x (faceSize - 2) interior pixels.
   *   - Faces 0,1,2 go in the top row; faces 3,4,5 in the bottom row
   *     (offset by +faceSize in Y).
   *   - face columns: x = face_in_row * faceSize
   *   - For LODs with faceSize == MIN_TILE_SIZE (the extra filter levels),
   *     they are offset in X by filterInt * 3 * MIN_TILE_SIZE.
   *   - Y offset: 4 * (cubeSize - faceSize) to stack mips from top. */
  for (int lod_idx = 0; lod_idx < total_lods; lod_idx++)
    {
      int face_size = size_lods[lod_idx];
      int mip_int = lod_max - (lod_idx < (lod_max - LOD_MIN + 1) ? lod_idx : (lod_max - LOD_MIN));
      int filter_int = LOD_MIN - mip_int;
      if (filter_int < 0)
        filter_int = 0;
      mip_int = MAX (mip_int, LOD_MIN);

      int base_x, base_y;
      if (lod_idx > lod_max - LOD_MIN)
        base_x = (lod_idx - (lod_max - LOD_MIN)) * 3 * MIN_TILE_SIZE;
      else
        base_x = 0;
      base_y = 4 * (cube_size - face_size);

      for (int face = 0; face < 6; face++)
        {
          int face_in_row = face % 3;
          int row = face > 2 ? 1 : 0;

          int tile_x = base_x + face_in_row * face_size;
          int tile_y = base_y + row * face_size;

          int content_size = face_size - 2;
          if (content_size < 1)
            content_size = 1;

          for (int py = 0; py < face_size; py++)
            {
              for (int px = 0; px < face_size; px++)
                {
                  int out_x = tile_x + px;
                  int out_y = tile_y + py;

                  if (out_x < 0 || out_x >= tex_width ||
                      out_y < 0 || out_y >= tex_height)
                    continue;

                  /* Map pixel to UV within the face content area.
                   * Interior pixels [1..faceSize-2] map to [0..1].
                   * Border pixels (0 and faceSize-1) map to slightly
                   * outside [0,1], which we clamp. */
                  float u = ((float)(px - 1) + 0.5f) / (float)content_size;
                  float v = ((float)(py - 1) + 0.5f) / (float)content_size;
                  u = CLAMP (u, 0.0f, 1.0f);
                  v = CLAMP (v, 0.0f, 1.0f);

                  float dir[3];
                  get_direction (u, v, face, dir);

                  float roughness = lod_roughness (lod_idx, lod_max);
                  float rgba[4];

                  if (roughness < 0.01f)
                    {
                      dir[0] = -dir[0];
                      sample_cubemap (pixbufs, dir, rgba);
                    }
                  else
                    {
                      ggx_filter_color (pixbufs, dir, roughness, rgba);
                    }
                  write_texel (buffer, tex_width, out_x, out_y, rgba);
                }
            }
        }
    }

  /* Create a GthreeTexture and upload the data via GL */
  result = g_object_new (GTHREE_TYPE_TEXTURE, NULL);
  gthree_texture_set_mapping (result, GTHREE_MAPPING_CUBE_UV_REFLECTION);
  gthree_texture_set_generate_mipmaps (result, FALSE);
  gthree_texture_set_mag_filter (result, GTHREE_FILTER_LINEAR);
  gthree_texture_set_min_filter (result, GTHREE_FILTER_LINEAR);
  gthree_texture_set_flip_y (result, FALSE);
  gthree_texture_set_name (result, "PMREM.cubeUv");

  gthree_renderer_push_current (priv->renderer);

  gthree_texture_realize (result, priv->renderer);
  gthree_texture_bind (result, priv->renderer, -1, GL_TEXTURE_2D);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA16F,
                tex_width, tex_height, 0,
                GL_RGBA, GL_FLOAT, buffer);

  gthree_resource_mark_clean_for (GTHREE_RESOURCE (result), priv->renderer);

  gthree_renderer_pop_current (priv->renderer);

  g_free (buffer);
  g_free (size_lods);

  priv->texture_width = tex_width;
  priv->texture_height = tex_height;

  {
    float *meta = g_new (float, 3);
    meta[0] = 1.0f / tex_width;
    meta[1] = 1.0f / tex_height;
    meta[2] = (float)lod_max;
    g_object_set_data_full (G_OBJECT (result), "cubeuv-meta", meta, g_free);
  }

  return result;
}
