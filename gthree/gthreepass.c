#include <math.h>
#include <epoxy/gl.h>

#include "gthreepass.h"
#include "gthreerenderer.h"
#include "gthreemesh.h"
#include "gthreeorthographiccamera.h"
#include "gthreeprimitives.h"
#include "gthreeshadermaterial.h"

G_DEFINE_TYPE (GthreePass, gthree_pass, G_TYPE_OBJECT)

static void
gthree_pass_init (GthreePass *pass)
{
  pass->enabled = TRUE;
  pass->need_swap = TRUE;
  pass->need_source_texture = TRUE;
  pass->does_copy = TRUE;
  pass->clear = FALSE;
  pass->can_render_to_screen = TRUE;
}

static void
gthree_pass_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_pass_parent_class)->finalize (obj);
}

static void
gthree_pass_class_init (GthreePassClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_pass_finalize;
}

void
gthree_pass_set_clear (GthreePass *pass,
                       gboolean clear)
{
  pass->clear = clear;
}

void
gthree_pass_set_enabled (GthreePass *pass,
                         gboolean enabled)
{
  pass->enabled = enabled;
}


void
gthree_pass_resize (GthreePass *pass,
                    int width,
                    int height)
{
  GthreePassClass *class = GTHREE_PASS_GET_CLASS(pass);
  if (class->set_size)
    class->set_size (pass, width, height);
}

void
gthree_pass_render (GthreePass *pass,
                    GthreeRenderer *renderer,
                    GthreeRenderTarget *write_buffer,
                    GthreeRenderTarget *read_buffer,
                    float delta_time,
                    gboolean render_to_screen,
                    gboolean mask_active)
{
  GthreePassClass *class = GTHREE_PASS_GET_CLASS(pass);
  if (class->render)
    class->render (pass, renderer,
                   write_buffer, read_buffer,
                   delta_time, render_to_screen, mask_active);
}

struct _GthreeFullscreenQuadPass {
  GthreePass parent;
  GthreeScene *scene;
  GthreeMesh *mesh;
};

typedef struct {
  GthreePassClass parent_class;
  GthreeCamera *camera;
  GthreeGeometry *geometry;
} GthreeFullscreenQuadPassClass;

G_DEFINE_TYPE (GthreeFullscreenQuadPass, gthree_fullscreen_quad_pass, GTHREE_TYPE_PASS)

static void
gthree_fullscreen_quad_pass_init (GthreeFullscreenQuadPass *pass)
{
}

static void
gthree_fullscreen_quad_pass_finalize (GObject *obj)
{
  GthreeFullscreenQuadPass *fq_pass = GTHREE_FULLSCREEN_QUAD_PASS (obj);
  g_clear_object (&fq_pass->mesh);
  g_clear_object (&fq_pass->scene);
  G_OBJECT_CLASS (gthree_fullscreen_quad_pass_parent_class)->finalize (obj);
}

// Note: This ignores render_to_screen, you need to set up the renderer render target ahead of time
static void
gthree_fullscreen_quad_pass_render (GthreePass *pass,
                                    GthreeRenderer *renderer,
                                    GthreeRenderTarget *write_buffer,
                                    GthreeRenderTarget *read_buffer,
                                    float delta_time,
                                    gboolean render_to_screen,
                                    gboolean mask_active)
{
  GthreeFullscreenQuadPass *fq_pass = GTHREE_FULLSCREEN_QUAD_PASS (pass);
  GthreeFullscreenQuadPassClass *pass_class = GTHREE_FULLSCREEN_QUAD_PASS_GET_CLASS(pass);
  gboolean old_auto_clear = gthree_renderer_get_autoclear (renderer);

  gthree_renderer_set_autoclear (renderer, FALSE);
  gthree_renderer_render (renderer, fq_pass->scene, pass_class->camera);
  gthree_renderer_set_autoclear (renderer, old_auto_clear);
}

static void
gthree_fullscreen_quad_pass_class_init (GthreeFullscreenQuadPassClass *klass)
{
  GthreePassClass *pass_class = GTHREE_PASS_CLASS(klass);

  G_OBJECT_CLASS (klass)->finalize = gthree_fullscreen_quad_pass_finalize;
  pass_class->render = gthree_fullscreen_quad_pass_render;

  klass->camera = GTHREE_CAMERA (gthree_orthographic_camera_new (-1, 1, 1, -1, 0, 1));
  klass->geometry = gthree_geometry_new_plane (2, 2, 1, 1);
}

GthreePass *
gthree_fullscreen_quad_pass_new  (GthreeMaterial *material)
{
  GthreeFullscreenQuadPass *pass = g_object_new (GTHREE_TYPE_FULLSCREEN_QUAD_PASS, NULL);
  GthreeFullscreenQuadPassClass *pass_class = GTHREE_FULLSCREEN_QUAD_PASS_GET_CLASS(pass);

  pass->scene = gthree_scene_new ();
  pass->mesh = gthree_mesh_new (pass_class->geometry, material);
  gthree_object_add_child (GTHREE_OBJECT (pass->scene), GTHREE_OBJECT (pass->mesh));

  return GTHREE_PASS (pass);
}

void
gthree_fullscreen_quad_pass_set_material (GthreeFullscreenQuadPass *pass,
                                          GthreeMaterial *material)
{
  gthree_mesh_set_material (pass->mesh, 0, material);
}

struct _GthreeShaderPass {
  GthreePass parent;
  char *texture_id;
  float time;
  GthreeMaterial *material;
  GthreeUniforms *uniforms; // Owned by shader in material
  GthreePass *fs_quad;
};

typedef struct {
  GthreePassClass parent_class;
} GthreeShaderPassClass;

G_DEFINE_TYPE (GthreeShaderPass, gthree_shader_pass, GTHREE_TYPE_PASS)

static void
gthree_shader_pass_init (GthreeShaderPass *pass)
{
}

static void
gthree_shader_pass_finalize (GObject *obj)
{
  GthreeShaderPass *pass = GTHREE_SHADER_PASS (obj);

  g_free (pass->texture_id);
  g_clear_object (&pass->material);
  g_clear_object (&pass->fs_quad);

  G_OBJECT_CLASS (gthree_shader_pass_parent_class)->finalize (obj);
}

static void
gthree_shader_pass_render (GthreePass *pass,
                           GthreeRenderer *renderer,
                           GthreeRenderTarget *write_buffer,
                           GthreeRenderTarget *read_buffer,
                           float delta_time,
                           gboolean render_to_screen,
                           gboolean mask_active)
{
  GthreeShaderPass *shader_pass = GTHREE_SHADER_PASS (pass);

  shader_pass->time += delta_time;

  gthree_uniforms_set_texture (shader_pass->uniforms,
                               shader_pass->texture_id,
                               gthree_render_target_get_texture (read_buffer));

  gthree_uniforms_set_float (shader_pass->uniforms,
                             "time",
                             shader_pass->time);

  if (render_to_screen)
    gthree_renderer_set_render_target (renderer, NULL, 0, 0);
  else
    gthree_renderer_set_render_target (renderer, write_buffer, 0, 0);

  if (pass->clear)
    gthree_renderer_clear (renderer,
                           gthree_renderer_get_autoclear_color (renderer),
                           gthree_renderer_get_autoclear_depth (renderer),
                           gthree_renderer_get_autoclear_stencil (renderer));

  gthree_pass_render (shader_pass->fs_quad, renderer,
                      write_buffer, read_buffer,
                      delta_time, FALSE, mask_active);
}

static void
gthree_shader_pass_class_init (GthreeShaderPassClass *klass)
{
  GthreePassClass *pass_class = GTHREE_PASS_CLASS(klass);

  G_OBJECT_CLASS (klass)->finalize = gthree_shader_pass_finalize;

  pass_class->render = gthree_shader_pass_render;
}

GthreePass *
gthree_shader_pass_new (GthreeShader *shader, const char *texture_id)
{
  GthreeShaderPass *pass = g_object_new (GTHREE_TYPE_SHADER_PASS, NULL);
  GthreeShaderMaterial *material = NULL;

  if (texture_id == NULL)
    texture_id = "tDiffuse";

  pass->texture_id = g_strdup (texture_id);

  material = gthree_shader_material_new (shader);

  pass->material = GTHREE_MATERIAL (material);
  pass->uniforms = gthree_shader_get_uniforms (shader);
  pass->fs_quad = gthree_fullscreen_quad_pass_new (pass->material);

  return GTHREE_PASS (pass);
}

struct _GthreeRenderPass {
  GthreePass parent;
  GthreeScene *scene;
  GthreeCamera *camera;
  GthreeMaterial *override_material;
  graphene_vec3_t clear_color;
  float clear_alpha;
  gboolean clear_depth;
  GArray *clipping_planes;
};

typedef struct {
  GthreePassClass parent_class;
} GthreeRenderPassClass;

G_DEFINE_TYPE (GthreeRenderPass, gthree_render_pass, GTHREE_TYPE_PASS)

static void
gthree_render_pass_init (GthreeRenderPass *render_pass)
{
  GthreePass *pass = GTHREE_PASS(render_pass);

  pass->clear = TRUE;
  pass->need_swap = FALSE;
  pass->need_source_texture = FALSE;
  pass->does_copy = FALSE;
  render_pass->clear_depth = FALSE;
}

static void
gthree_render_pass_finalize (GObject *obj)
{
  GthreeRenderPass *pass = GTHREE_RENDER_PASS (obj);

  g_clear_object (&pass->scene);
  g_clear_object (&pass->camera);
  g_clear_object (&pass->override_material);

  if (pass->clipping_planes)
    g_array_unref (pass->clipping_planes);

  G_OBJECT_CLASS (gthree_render_pass_parent_class)->finalize (obj);
}

static void
gthree_render_pass_render (GthreePass *pass,
                           GthreeRenderer *renderer,
                           GthreeRenderTarget *write_buffer,
                           GthreeRenderTarget *read_buffer,
                           float delta_time,
                           gboolean render_to_screen,
                           gboolean mask_active)
{
  GthreeRenderPass *render_pass = GTHREE_RENDER_PASS (pass);
  gboolean old_auto_clear = gthree_renderer_get_autoclear (renderer);
  g_autoptr(GArray) old_clipping_planes = NULL;

  gthree_renderer_set_autoclear (renderer, FALSE);

  //TODO: this.scene.overrideMaterial = this.overrideMaterial;

#ifdef TODO
  var oldClearColor, oldClearAlpha;
  if ( this.clearColor ) {
    oldClearColor = renderer.getClearColor().getHex();
    oldClearAlpha = renderer.getClearAlpha();
    renderer.setClearColor( this.clearColor, this.clearAlpha );
  }
#endif

  gthree_renderer_set_render_target (renderer, render_to_screen ? NULL : read_buffer, 0, 0);

  if (render_pass->clipping_planes)
    {
      old_clipping_planes = g_array_ref (gthree_renderer_get_clipping_planes (renderer));
      gthree_renderer_set_clipping_planes (renderer, render_pass->clipping_planes);
    }

  if (render_pass->clear_depth)
    gthree_renderer_clear_depth (renderer);

  if (pass->clear)
    gthree_renderer_clear (renderer,
                           gthree_renderer_get_autoclear_color (renderer),
                           gthree_renderer_get_autoclear_depth (renderer),
                           gthree_renderer_get_autoclear_stencil (renderer));

  gthree_renderer_render (renderer, render_pass->scene, render_pass->camera);

  if (old_clipping_planes)
    gthree_renderer_set_clipping_planes (renderer, old_clipping_planes);

  gthree_renderer_set_autoclear (renderer, old_auto_clear);
}

static void
gthree_render_pass_class_init (GthreeRenderPassClass *klass)
{
  GthreePassClass *pass_class = GTHREE_PASS_CLASS(klass);

  G_OBJECT_CLASS (klass)->finalize = gthree_render_pass_finalize;

  pass_class->render = gthree_render_pass_render;
}

GthreePass *
gthree_render_pass_new (GthreeScene *scene,
                        GthreeCamera *camera,
                        GthreeMaterial *override_material)
{
  GthreeRenderPass *pass = g_object_new (GTHREE_TYPE_RENDER_PASS, NULL);

  pass->scene = g_object_ref (scene);
  pass->camera = g_object_ref (camera);
  if (override_material)
    pass->override_material = g_object_ref (override_material);

  return GTHREE_PASS (pass);
}

void
gthree_render_pass_set_clipping_planes  (GthreeRenderPass *render_pass,
                                         GArray *clipping_planes)
{
  g_array_ref (clipping_planes);
  if (render_pass->clipping_planes)
    g_array_unref (render_pass->clipping_planes);
  render_pass->clipping_planes = clipping_planes;
}

void
gthree_render_pass_set_clear_depth  (GthreeRenderPass *render_pass,
                                     gboolean clear_depth)
{
  render_pass->clear_depth = clear_depth;
}

struct _GthreeClearPass {
  GthreePass parent;
  graphene_vec3_t color;
  gboolean color_set;
  gboolean clear_depth;
};

typedef struct {
  GthreePassClass parent_class;
} GthreeClearPassClass;

G_DEFINE_TYPE (GthreeClearPass, gthree_clear_pass, GTHREE_TYPE_PASS)

static void
gthree_clear_pass_init (GthreeClearPass *clear_pass)
{
  GthreePass *pass = GTHREE_PASS(clear_pass);

  pass->clear = TRUE;
  pass->need_swap = FALSE;
  pass->need_source_texture = FALSE;
  clear_pass->clear_depth = FALSE;
}

static void
gthree_clear_pass_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_clear_pass_parent_class)->finalize (obj);
}

static void
gthree_clear_pass_render (GthreePass *pass,
                          GthreeRenderer *renderer,
                          GthreeRenderTarget *write_buffer,
                          GthreeRenderTarget *read_buffer,
                          float delta_time,
                          gboolean render_to_screen,
                          gboolean mask_active)
{
  GthreeClearPass *clear_pass = GTHREE_CLEAR_PASS (pass);
  graphene_vec3_t old_clear_color;

  if (clear_pass->color_set)
    {
      old_clear_color = *gthree_renderer_get_clear_color (renderer);
      gthree_renderer_set_clear_color (renderer, &clear_pass->color);
    }

  gthree_renderer_set_render_target (renderer, render_to_screen ? NULL : read_buffer, 0, 0);

  if (clear_pass->clear_depth)
    gthree_renderer_clear_depth (renderer);

  if (pass->clear)
    gthree_renderer_clear (renderer, TRUE, TRUE, TRUE);

  if (clear_pass->color_set)
    gthree_renderer_set_clear_color (renderer, &old_clear_color);
}

static void
gthree_clear_pass_class_init (GthreeClearPassClass *klass)
{
  GthreePassClass *pass_class = GTHREE_PASS_CLASS(klass);

  G_OBJECT_CLASS (klass)->finalize = gthree_clear_pass_finalize;

  pass_class->render = gthree_clear_pass_render;
}

GthreePass *
gthree_clear_pass_new (const graphene_vec3_t *color)
{
  GthreeClearPass *pass = g_object_new (GTHREE_TYPE_CLEAR_PASS, NULL);

  if (color)
    {
      pass->color = *color;
      pass->color_set = TRUE;
    }

  return GTHREE_PASS (pass);
}

void
gthree_clear_pass_set_clear_depth  (GthreeClearPass *clear_pass,
                                     gboolean clear_depth)
{
  clear_pass->clear_depth = clear_depth;
}

struct _GthreeBloomPass {
  GthreePass parent;

  GthreeRenderTarget *render_target_x;
  GthreeRenderTarget *render_target_y;

  GthreeUniforms *copy_uniforms; // Owned by copy_material
  GthreeShaderMaterial *copy_material;

  GthreeUniforms *convolution_uniforms; // Owned by convolution_material
  GthreeShaderMaterial *convolution_material;

  GthreePass *fs_quad;
};

typedef struct {
  GthreePassClass parent_class;
} GthreeBloomPassClass;

G_DEFINE_TYPE (GthreeBloomPass, gthree_bloom_pass, GTHREE_TYPE_PASS)

static void
gthree_bloom_pass_init (GthreeBloomPass *bloom_pass)
{
  GthreePass *pass = GTHREE_PASS(bloom_pass);

  bloom_pass->fs_quad = gthree_fullscreen_quad_pass_new (NULL);

  pass->need_swap = FALSE;
  pass->need_source_texture = TRUE;
  pass->can_render_to_screen = FALSE;
}

static void
gthree_bloom_pass_finalize (GObject *obj)
{
  GthreeBloomPass *pass = GTHREE_BLOOM_PASS (obj);

  g_clear_object (&pass->render_target_x);
  g_clear_object (&pass->render_target_y);

  g_clear_object (&pass->fs_quad);

  g_clear_object (&pass->copy_material);
  g_clear_object (&pass->convolution_material);

  G_OBJECT_CLASS (gthree_bloom_pass_parent_class)->finalize (obj);
}

static void
gthree_bloom_pass_render (GthreePass *pass,
                          GthreeRenderer *renderer,
                          GthreeRenderTarget *write_buffer,
                          GthreeRenderTarget *read_buffer,
                          float delta_time,
                          gboolean render_to_screen,
                          gboolean mask_active)
{
  GthreeBloomPass *bloom_pass = GTHREE_BLOOM_PASS (pass);
  graphene_vec2_t blurX, blurY;

  graphene_vec2_init (&blurX, 0.001953125, 0.0);
  graphene_vec2_init (&blurY, 0.0, 0.001953125);

  if (mask_active)
    {
#ifdef TODO
      renderer.context.disable( renderer.context.STENCIL_TEST );
#endif
    }

  // Render quad with blured scene into texture (convolution pass 1)
  gthree_fullscreen_quad_pass_set_material (GTHREE_FULLSCREEN_QUAD_PASS (bloom_pass->fs_quad),
                                            GTHREE_MATERIAL (bloom_pass->convolution_material));

  gthree_uniforms_set_texture (bloom_pass->convolution_uniforms,
                               "tDiffuse",
                               gthree_render_target_get_texture (read_buffer));
  gthree_uniforms_set_vec2 (bloom_pass->convolution_uniforms,
                            "uImageIncrement", &blurX);

  gthree_renderer_set_render_target (renderer, bloom_pass->render_target_x, 0, 0);
  gthree_renderer_clear (renderer, TRUE, TRUE, TRUE);
  gthree_pass_render (bloom_pass->fs_quad, renderer,
                      write_buffer, read_buffer,
                      delta_time, FALSE, mask_active);

  // Render quad with blured scene into texture (convolution pass 2)
  gthree_uniforms_set_texture (bloom_pass->convolution_uniforms,
                               "tDiffuse",
                               gthree_render_target_get_texture (bloom_pass->render_target_x));
  gthree_uniforms_set_vec2 (bloom_pass->convolution_uniforms,
                            "uImageIncrement", &blurY);

  gthree_renderer_set_render_target (renderer, bloom_pass->render_target_y, 0, 0);
  gthree_renderer_clear (renderer, TRUE, TRUE, TRUE);
  gthree_pass_render (bloom_pass->fs_quad, renderer,
                      write_buffer, read_buffer,
                      delta_time, FALSE, mask_active);

  // Render original scene with superimposed blur to texture

  gthree_fullscreen_quad_pass_set_material (GTHREE_FULLSCREEN_QUAD_PASS (bloom_pass->fs_quad),
                                            GTHREE_MATERIAL (bloom_pass->copy_material));
  gthree_uniforms_set_texture (bloom_pass->copy_uniforms,
                               "tDiffuse",
                               gthree_render_target_get_texture (bloom_pass->render_target_y));

  if (mask_active)
    {
#ifdef TODO
      renderer.context.enable( renderer.context.STENCIL_TEST );
#endif
    }

  // Always render to read buffer, needs a final copy to screen because
  // we need to overdraw the last rendered thing
  gthree_renderer_set_render_target (renderer, read_buffer, 0, 0);

  if (pass->clear)
    gthree_renderer_clear (renderer,
                           gthree_renderer_get_autoclear_color (renderer),
                           gthree_renderer_get_autoclear_depth (renderer),
                           gthree_renderer_get_autoclear_stencil (renderer));

  gthree_pass_render (bloom_pass->fs_quad, renderer,
                      write_buffer, read_buffer,
                      delta_time, FALSE, mask_active);
}

static void
gthree_bloom_pass_class_init (GthreeBloomPassClass *klass)
{
  GthreePassClass *pass_class = GTHREE_PASS_CLASS(klass);

  G_OBJECT_CLASS (klass)->finalize = gthree_bloom_pass_finalize;

  pass_class->render = gthree_bloom_pass_render;
}

GthreePass *
gthree_bloom_pass_new (float strength, float sigma, int resolution)
{
  GthreeBloomPass *pass = g_object_new (GTHREE_TYPE_BLOOM_PASS, NULL);
  g_autoptr(GthreeShader) copy_shader = NULL;
  g_autoptr(GthreeShader) convolution_shader = NULL;
  graphene_vec2_t blurX, blurY;
  g_autoptr(GArray) kernel = NULL;
  g_autoptr(GPtrArray) defines = NULL;
  int kernel_size;

  graphene_vec2_init (&blurX, 0.001953125, 0.0);
  graphene_vec2_init (&blurY, 0.0, 0.001953125);

  kernel = gthree_convolution_shader_build_kernel (sigma);
  kernel_size = kernel->len;

  // render targets

  pass->render_target_x = gthree_render_target_new (resolution, resolution, 0);
  gthree_texture_set_name (gthree_render_target_get_texture (pass->render_target_x), "BloomPass.x");

  pass->render_target_y = gthree_render_target_new (resolution, resolution, 0);
  gthree_texture_set_name (gthree_render_target_get_texture (pass->render_target_y), "BloomPass.y");

  // copy material

  copy_shader = gthree_clone_shader_from_library ("copy");

  pass->copy_uniforms = gthree_shader_get_uniforms (copy_shader);
  gthree_uniforms_set_float (pass->copy_uniforms, "opacity", strength);

  pass->copy_material = gthree_shader_material_new (copy_shader);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (pass->copy_material),
                                  GTHREE_BLEND_ADDITIVE,
                                  GL_FUNC_ADD,
                                  GL_SRC_ALPHA,
                                  GL_ONE_MINUS_SRC_ALPHA);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (pass->copy_material), TRUE);

  // convolution material

  convolution_shader = gthree_clone_shader_from_library ("convolution");
  defines = g_ptr_array_new_with_free_func (g_free);
  g_ptr_array_add (defines, g_strdup ("KERNEL_SIZE_FLOAT"));
  g_ptr_array_add (defines, g_strdup_printf ("%d.0", kernel_size));
  g_ptr_array_add (defines, g_strdup ("KERNEL_SIZE_INT"));
  g_ptr_array_add (defines, g_strdup_printf ("%d", kernel_size));
  gthree_shader_set_defines (convolution_shader, defines);

  pass->convolution_uniforms = gthree_shader_get_uniforms (convolution_shader);
  gthree_uniforms_set_vec2 (pass->convolution_uniforms, "uImageIncrement", &blurX);
  gthree_uniforms_set_float_array (pass->convolution_uniforms,
                                   "cKernel", kernel);

  pass->convolution_material = gthree_shader_material_new (convolution_shader);

  return GTHREE_PASS (pass);
}
