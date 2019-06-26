#include <math.h>

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
  pass->clear = FALSE;
  pass->render_to_screen = FALSE;
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
                    gboolean mask_active)
{
  GthreePassClass *class = GTHREE_PASS_GET_CLASS(pass);
  if (class->render)
    class->render (pass, renderer,
                   write_buffer, read_buffer,
                   delta_time, mask_active);
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

static void
gthree_fullscreen_quad_pass_render (GthreePass *pass,
                                    GthreeRenderer *renderer,
                                    GthreeRenderTarget *write_buffer,
                                    GthreeRenderTarget *read_buffer,
                                    float delta_time,
                                    gboolean mask_active)
{
  GthreeFullscreenQuadPass *fq_pass = GTHREE_FULLSCREEN_QUAD_PASS (pass);
  GthreeFullscreenQuadPassClass *pass_class = GTHREE_FULLSCREEN_QUAD_PASS_GET_CLASS(pass);

  gthree_renderer_render (renderer, fq_pass->scene, pass_class->camera);
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
  gthree_mesh_set_material (pass->mesh, material);
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

  if (pass->render_to_screen)
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
                      delta_time, mask_active);
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
  g_autoptr(GthreeShader) shader_clone = NULL;
  GthreeShaderMaterial *material = NULL;

  if (texture_id == NULL)
    texture_id = "tDiffuse";

  pass->texture_id = g_strdup (texture_id);

  shader_clone = gthree_shader_clone (shader);

  material = gthree_shader_material_new (shader_clone);

  pass->material = GTHREE_MATERIAL (material);
  pass->uniforms = gthree_shader_get_uniforms (shader_clone);
  pass->fs_quad = gthree_fullscreen_quad_pass_new (pass->material);

  return GTHREE_PASS (pass);
}

struct _GthreeRenderPass {
  GthreePass parent;
  GthreeScene *scene;
  GthreeCamera *camera;
  GthreeMaterial *override_material;
  GdkRGBA clear_color;
  float clear_alpha;
  gboolean clear_depth;
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
  render_pass->clear_depth = FALSE;
}

static void
gthree_render_pass_finalize (GObject *obj)
{
  GthreeRenderPass *pass = GTHREE_RENDER_PASS (obj);

  g_clear_object (&pass->scene);
  g_clear_object (&pass->camera);
  g_clear_object (&pass->override_material);

  G_OBJECT_CLASS (gthree_render_pass_parent_class)->finalize (obj);
}

static void
gthree_render_pass_render (GthreePass *pass,
                           GthreeRenderer *renderer,
                           GthreeRenderTarget *write_buffer,
                           GthreeRenderTarget *read_buffer,
                           float delta_time,
                           gboolean mask_active)
{
  GthreeRenderPass *render_pass = GTHREE_RENDER_PASS (pass);
  gboolean old_auto_clear = gthree_renderer_get_autoclear (renderer);

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

  gthree_renderer_set_render_target (renderer, pass->render_to_screen ? NULL : read_buffer, 0, 0);

  if (render_pass->clear_depth)
    gthree_renderer_clear_depth (renderer);

  if (pass->clear)
    gthree_renderer_clear (renderer,
                           gthree_renderer_get_autoclear_color (renderer),
                           gthree_renderer_get_autoclear_depth (renderer),
                           gthree_renderer_get_autoclear_stencil (renderer));

  gthree_renderer_render (renderer, render_pass->scene, render_pass->camera);

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
gthree_render_pass_set_clear_depth  (GthreeRenderPass *render_pass,
                                     gboolean clear_depth)
{
  render_pass->clear_depth = clear_depth;
}

struct _GthreeClearPass {
  GthreePass parent;
  GdkRGBA color;
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
                          gboolean mask_active)
{
  GthreeClearPass *clear_pass = GTHREE_CLEAR_PASS (pass);
  GdkRGBA old_clear_color;

  if (clear_pass->color_set)
    {
      old_clear_color = *gthree_renderer_get_clear_color (renderer);
      gthree_renderer_set_clear_color (renderer, &clear_pass->color);
    }

  gthree_renderer_set_render_target (renderer, pass->render_to_screen ? NULL : read_buffer, 0, 0);

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
gthree_clear_pass_new (const GdkRGBA *color)
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
