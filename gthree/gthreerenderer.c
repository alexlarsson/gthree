#include <math.h>
#include <epoxy/gl.h>

#include "gthreerenderer.h"
#include "gthreeobjectprivate.h"
#include "gthreemesh.h"
#include "gthreeskinnedmesh.h"
#include "gthreelinesegments.h"
#include "gthreeshader.h"
#include "gthreematerial.h"
#include "gthreeprivate.h"
#include "gthreeobjectprivate.h"
#include "gthreecubetexture.h"
#include "gthreeshadermaterial.h"
#include "gthreemeshdepthmaterial.h"
#include "gthreemeshdistancematerial.h"
#include "gthreemeshmaterial.h"
#include "gthreelinebasicmaterial.h"
#include "gthreeprimitives.h"
#include "gthreegroup.h"
#include "gthreeattribute.h"
#include "gthreesprite.h"
#include "gthreepoints.h"
#include "gthreespotlight.h"
#include "gthreepointlight.h"

#define MAX_MORPH_TARGETS 8
#define MAX_MORPH_NORMALS 4

static graphene_vec3_t cube_directions[6];
static graphene_vec3_t cube_ups[6];

/* We keep track of the current renderer, as a proxy for the per-renderer gl context being active */
static GPrivate thread_current_renderer = G_PRIVATE_INIT (NULL);

typedef struct {
  GthreeObject *object;
  GthreeGeometry *geometry;
  GthreeMaterial *material;
  GthreeGeometryGroup *group;
  float z;
} GthreeRenderListItem;

struct _GthreeRenderList {
  float current_z;
  gboolean use_background;
  GArray *items;
  GArray *opaque;
  GArray *transparent;
  GArray *background;
};

typedef struct {
  GList *current_stack;

  int width;
  int height;
  int pixel_ratio;
  guint window_framebuffer;

  gboolean auto_clear;
  gboolean auto_clear_color;
  gboolean auto_clear_depth;
  gboolean auto_clear_stencil;
  graphene_vec3_t clear_color;
  gboolean sort_objects;
  float gamma_factor;
  gboolean physically_correct_lights;
  gboolean shadowmap_enabled;
  gboolean shadowmap_auto_update;
  gboolean shadowmap_needs_update;
  GthreeShadowMapType shadowmap_type;
  GPtrArray *shadowmap_depth_materials;
  GPtrArray *shadowmap_distance_materials;

  GArray *clipping_planes;

  graphene_rect_t viewport;

  /* Render state */
  GthreeProgramCache *program_cache;

  graphene_frustum_t frustum;
  graphene_matrix_t proj_screen_matrix;
  gboolean clipping_enabled;

  guint used_texture_units;

  GthreeLightSetup light_setup;
  GList *lights;

  GList *shadows;

  gboolean old_flip_sided;
  gboolean old_double_sided;
  gboolean old_depth_test;
  gboolean old_depth_write;
  float old_line_width;
  gboolean old_polygon_offset;
  float old_polygon_offset_factor;
  float old_polygon_offset_units;
  GthreeBlendMode old_blending;
  guint old_blend_equation;
  guint old_blend_src;
  guint old_blend_dst;
  guint old_num_global_planes;
  graphene_vec4_t old_clear_color;
  GthreeRenderTarget *current_render_target;
  GthreeProgram *current_program;
  GthreeMaterial *current_material;
  GthreeCamera *current_camera;
  graphene_rect_t current_viewport; // Either ->viewport, or from the render target
  guint current_framebuffer;
  GArray *clipping_state;
  guint num_clipping_planes;

  GthreeGeometry *current_geometry_program_geometry;
  GthreeProgram *current_geometry_program_program;
  gboolean current_geometry_program_wireframe;

  GthreeRenderList *current_render_list;

  guint8 new_attributes[8];
  guint8 enabled_attributes[8];

  float morph_influences[8];

  int max_textures;
  int max_vertex_textures;
  int max_texture_size;
  int max_cubemap_size;
  float max_anisotropy;

  gboolean supports_vertex_textures;
  gboolean supports_bone_textures;

  guint vertex_array_object;

  /* Background */
  GthreeMesh *bg_box_mesh;
  GthreeMesh *bg_plane_mesh;
  GthreeTexture *current_bg_texture;

} GthreeRendererPrivate;

static void gthree_set_default_gl_state (GthreeRenderer *renderer);

static GQuark q_position;
static GQuark q_color;
static GQuark q_uv;
static GQuark q_uv2;
static GQuark q_normal;
static GQuark q_viewMatrix;
static GQuark q_modelMatrix;
static GQuark q_modelViewMatrix;
static GQuark q_normalMatrix;
static GQuark q_projectionMatrix;
static GQuark q_cameraPosition;
static GQuark q_clippingPlanes;
static GQuark q_ambientLightColor;
static GQuark q_directionalLights;
static GQuark q_pointLights;
static GQuark q_spotLights;
static GQuark q_bindMatrix;
static GQuark q_bindMatrixInverse;
static GQuark q_boneMatrices;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeRenderer, gthree_renderer, G_TYPE_OBJECT);

static void clear (gboolean color, gboolean depth, gboolean stencil);
static void set_clear_color (GthreeRenderer *renderer,
                             const graphene_vec3_t *color,
                             float alpha);

static void render_item (GthreeRenderer *renderer,
                         GthreeCamera *camera,
                         gpointer fog,
                         GthreeMaterial *material,
                         GthreeRenderListItem *item);

static void
push_debug_group (const char   *format, ...)
{
#ifdef DEBUG_GROUPS
  gchar *message;
  va_list args;
  int msg_len;

  va_start (args, format);
  message = g_strdup_vprintf (format, args);
  va_end (args);

  msg_len = strlen (message);
  glPushDebugGroupKHR (GL_DEBUG_SOURCE_APPLICATION, 0, msg_len, message);
  g_free (message);
#endif
}

static void
pop_debug_group (void)
{
#ifdef DEBUG_GROUPS
  glPopDebugGroupKHR ();
#endif
}


/* Call this whenever we're sure that the renderer gl context is
   current, this is used by the resource system to track of whether
   the contex is currently valid or not. */
void
gthree_renderer_push_current (GthreeRenderer *renderer)
{
  GthreeRenderer *current = g_private_get (&thread_current_renderer);
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->current_stack = g_list_prepend (priv->current_stack, current);

  g_private_replace (&thread_current_renderer, renderer);
}

void
gthree_renderer_pop_current (GthreeRenderer *renderer)
{
  GthreeRenderer *old_current;
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  g_assert (priv->current_stack != NULL);
  old_current = priv->current_stack->data;
  priv->current_stack = g_list_delete_link (priv->current_stack, priv->current_stack);
  g_private_replace (&thread_current_renderer, old_current);
}

GthreeRenderer *
gthree_renderer_get_current (void)
{
  GthreeRenderer *current;

  current = g_private_get (&thread_current_renderer);

  return current;
}

GthreeRenderer *
gthree_renderer_new ()
{
  return g_object_new (gthree_renderer_get_type (),
                       NULL);
}

static void
gthree_renderer_init (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GLint fbo_id = 0;

  gthree_renderer_push_current (renderer);

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo_id);
  priv->window_framebuffer = fbo_id;
#ifdef DEBUG_LABELS
  glObjectLabel (GL_FRAMEBUFFER, priv->window_framebuffer, strlen ("GthreeArea.FB"), "GthreeArea.FB");
#endif

  priv->program_cache = gthree_program_cache_new ();

  priv->auto_clear = TRUE;
  priv->auto_clear_color = TRUE;
  priv->auto_clear_depth = TRUE;
  priv->auto_clear_stencil = TRUE;
  priv->sort_objects = TRUE;
  priv->width = 1;
  priv->height = 1;
  priv->pixel_ratio = 1;
  priv->gamma_factor = 2.2; // Differs from three.js default 2.0
  priv->physically_correct_lights = FALSE;
  priv->shadowmap_type = GTHREE_SHADOW_MAP_TYPE_PCF;
  priv->shadowmap_enabled = FALSE;
  priv->shadowmap_auto_update = TRUE;
  priv->shadowmap_needs_update = FALSE;

  priv->clipping_planes = g_array_new (FALSE, FALSE, sizeof (graphene_plane_t));
  priv->clipping_state = g_array_new (FALSE, FALSE, sizeof (float));

  priv->current_framebuffer = 0;

  priv->light_setup.directional = g_ptr_array_new ();
  priv->light_setup.directional_shadow_map = g_ptr_array_new ();
  priv->light_setup.directional_shadow_map_matrix = g_array_new (FALSE, FALSE, sizeof (graphene_matrix_t));
  priv->light_setup.point = g_ptr_array_new ();
  priv->light_setup.point_shadow_map = g_ptr_array_new ();
  priv->light_setup.point_shadow_map_matrix = g_array_new (FALSE, FALSE, sizeof (graphene_matrix_t));
  priv->light_setup.spot = g_ptr_array_new ();
  priv->light_setup.spot_shadow_map = g_ptr_array_new ();
  priv->light_setup.spot_shadow_map_matrix = g_array_new (FALSE, FALSE, sizeof (graphene_matrix_t));
  priv->light_setup.shadow = g_ptr_array_new ();

  priv->current_render_list = gthree_render_list_new ();

  priv->old_blending = -1;
  priv->old_blend_equation = -1;
  priv->old_blend_src = -1;
  priv->old_blend_dst = -1;
  priv->old_depth_write = -1;
  priv->old_depth_test = -1;

  gthree_set_default_gl_state (renderer);

  /* We only use one vao, so bind it here */
  glGenVertexArrays (1, &priv->vertex_array_object);
  glBindVertexArray (priv->vertex_array_object);

  // GPU capabilities
  glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS, &priv->max_textures);
  glGetIntegerv (GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &priv->max_vertex_textures);
  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &priv->max_texture_size);
  glGetIntegerv (GL_MAX_CUBE_MAP_TEXTURE_SIZE, &priv->max_cubemap_size);

  priv->max_anisotropy = 0.0f;
  if (epoxy_has_gl_extension("GL_EXT_texture_filter_anisotropic"))
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &priv->max_anisotropy);

  priv->supports_vertex_textures = priv->max_vertex_textures > 0;
  priv->supports_bone_textures =
    priv->supports_vertex_textures &&
    epoxy_has_gl_extension("GL_ARB_texture_float");

  //priv->compressed_texture_formats = _glExtensionCompressedTextureS3TC ? glGetParameter( _gl.COMPRESSED_TEXTURE_FORMATS ) : [];

  gthree_renderer_pop_current (renderer);
}

static void
gthree_renderer_finalize (GObject *obj)
{
  GthreeRenderer *renderer = GTHREE_RENDERER (obj);
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  gthree_renderer_push_current (renderer);

  g_clear_object (&priv->current_render_target);

  if (priv->shadowmap_depth_materials)
    g_ptr_array_unref (priv->shadowmap_depth_materials);
  if (priv->shadowmap_distance_materials)
    g_ptr_array_unref (priv->shadowmap_distance_materials);

  gthree_program_cache_free (priv->program_cache);

  g_array_free (priv->clipping_planes, TRUE);
  g_array_free (priv->clipping_state, TRUE);

  g_list_free (priv->lights);
  g_ptr_array_free (priv->light_setup.directional, TRUE);
  g_ptr_array_free (priv->light_setup.directional_shadow_map, TRUE);
  g_array_free (priv->light_setup.directional_shadow_map_matrix, TRUE);
  g_ptr_array_free (priv->light_setup.point, TRUE);
  g_ptr_array_free (priv->light_setup.point_shadow_map, TRUE);
  g_array_free (priv->light_setup.point_shadow_map_matrix, TRUE);
  g_ptr_array_free (priv->light_setup.spot, TRUE);
  g_ptr_array_free (priv->light_setup.spot_shadow_map, TRUE);
  g_array_free (priv->light_setup.spot_shadow_map_matrix, TRUE);
  g_ptr_array_free (priv->light_setup.shadow, TRUE);

  gthree_render_list_free (priv->current_render_list);

  g_clear_object (&priv->bg_box_mesh);
  g_clear_object (&priv->bg_plane_mesh);
  g_clear_object (&priv->current_bg_texture);

  gthree_renderer_pop_current (renderer);

  G_OBJECT_CLASS (gthree_renderer_parent_class)->finalize (obj);
}

static void
gthree_renderer_class_init (GthreeRendererClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_renderer_finalize;

#define INIT_QUARK(name) q_##name = g_quark_from_static_string (#name)
  INIT_QUARK(position);
  INIT_QUARK(color);
  INIT_QUARK(uv);
  INIT_QUARK(uv2);
  INIT_QUARK(normal);
  INIT_QUARK(viewMatrix);
  INIT_QUARK(modelMatrix);
  INIT_QUARK(modelViewMatrix);
  INIT_QUARK(normalMatrix);
  INIT_QUARK(projectionMatrix);
  INIT_QUARK(cameraPosition);
  INIT_QUARK(clippingPlanes);
  INIT_QUARK(ambientLightColor);
  INIT_QUARK(directionalLights);
  INIT_QUARK(pointLights);
  INIT_QUARK(spotLights);
  INIT_QUARK(bindMatrix);
  INIT_QUARK(bindMatrixInverse);
  INIT_QUARK(boneMatrices);

  graphene_vec3_init (&cube_directions[0],  1,  0,  0);
  graphene_vec3_init (&cube_directions[1], -1,  0,  0);
  graphene_vec3_init (&cube_directions[2],  0,  0,  1);
  graphene_vec3_init (&cube_directions[3],  0,  0, -1);
  graphene_vec3_init (&cube_directions[4],  0,  1,  0);
  graphene_vec3_init (&cube_directions[5],  0, -1,  0);

  graphene_vec3_init (&cube_ups[0],  0,  1,  0);
  graphene_vec3_init (&cube_ups[1],  0,  1,  0);
  graphene_vec3_init (&cube_ups[2],  0,  1,  0);
  graphene_vec3_init (&cube_ups[3],  0,  1,  0);
  graphene_vec3_init (&cube_ups[4],  0,  0,  1);
  graphene_vec3_init (&cube_ups[5],  0,  0, -1);

}

void
gthree_renderer_set_viewport (GthreeRenderer *renderer,
                              float           x,
                              float           y,
                              float           width,
                              float           height)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  graphene_rect_init (&priv->viewport, x, y, width, height);
  graphene_rect_init_from_rect (&priv->current_viewport, &priv->viewport);

  glViewport (graphene_rect_get_x (&priv->current_viewport) * priv->pixel_ratio,
              graphene_rect_get_y (&priv->current_viewport) * priv->pixel_ratio,
              graphene_rect_get_width (&priv->current_viewport) * priv->pixel_ratio,
              graphene_rect_get_height (&priv->current_viewport) * priv->pixel_ratio);
}

void
gthree_renderer_set_size (GthreeRenderer *renderer,
                          int             width,
                          int             height)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->width = width;
  priv->height = height;

  gthree_renderer_set_viewport (renderer, 0, 0, width, height);
}

void
gthree_renderer_set_pixel_ratio (GthreeRenderer     *renderer,
                                 int                 pixel_ratio)
{
 GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->pixel_ratio = pixel_ratio;

  gthree_renderer_set_size (renderer, priv->width, priv->height);

}

int
gthree_renderer_get_width (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->width;
}

int
gthree_renderer_get_height (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->height;
}

int
gthree_renderer_get_pixel_ratio (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->pixel_ratio;
}

int
gthree_renderer_get_drawing_buffer_width (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->width * priv->pixel_ratio;
}

int
gthree_renderer_get_drawing_buffer_height (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->height * priv->pixel_ratio;
}

void
gthree_renderer_set_autoclear (GthreeRenderer *renderer,
                               gboolean        auto_clear)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear = !!auto_clear;
}

gboolean
gthree_renderer_get_autoclear (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->auto_clear;
}

void
gthree_renderer_set_autoclear_color (GthreeRenderer *renderer,
                                     gboolean        clear_color)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear_color = !!clear_color;
}

gboolean
gthree_renderer_get_autoclear_color (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->auto_clear_color;
}

void
gthree_renderer_set_autoclear_depth (GthreeRenderer *renderer,
                                     gboolean        clear_depth)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear_depth = !!clear_depth;
}

gboolean
gthree_renderer_get_autoclear_depth (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->auto_clear_depth;
}

void
gthree_renderer_set_autoclear_stencil (GthreeRenderer *renderer,
                                       gboolean        clear_stencil)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear_stencil = !!clear_stencil;
}

gboolean
gthree_renderer_get_autoclear_stencil (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->auto_clear_stencil;
}

void
gthree_renderer_set_clear_color (GthreeRenderer *renderer,
                                 const graphene_vec3_t *color)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->clear_color = *color;
}

const graphene_vec3_t *
gthree_renderer_get_clear_color  (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return &priv->clear_color;
}

void
gthree_renderer_set_gamma_factor (GthreeRenderer *renderer,
                                   float           factor)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->gamma_factor = factor;
}

float
gthree_renderer_get_gamma_factor (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  return priv->gamma_factor;
}

gboolean
gthree_renderer_get_shadow_map_enabled (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->shadowmap_enabled;
}

void
gthree_renderer_set_shadow_map_enabled (GthreeRenderer     *renderer,
                                        gboolean            enabled)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->shadowmap_enabled = enabled;
}

gboolean
gthree_renderer_get_shadow_map_auto_update (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->shadowmap_auto_update;
}

void
gthree_renderer_set_shadow_map_auto_update (GthreeRenderer     *renderer,
                                            gboolean            auto_update)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->shadowmap_auto_update = auto_update;
}

gboolean
gthree_renderer_get_shadow_map_needs_update (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->shadowmap_needs_update;
}

void
gthree_renderer_set_shadow_map_needs_update (GthreeRenderer     *renderer,
                                             gboolean            needs_update)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->shadowmap_needs_update = needs_update;
}

int
gthree_renderer_get_n_clipping_planes (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->clipping_planes->len;
}

GArray *
gthree_renderer_get_clipping_planes (GthreeRenderer     *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->clipping_planes;
}

void
gthree_renderer_set_clipping_planes (GthreeRenderer     *renderer,
                                     GArray             *clipping_planes)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (clipping_planes)
    g_array_ref (clipping_planes);
  else
    clipping_planes = g_array_new (FALSE, FALSE, sizeof (graphene_plane_t));

  g_array_unref (priv->clipping_planes);
  priv->clipping_planes = clipping_planes;
}

const graphene_plane_t *
gthree_renderer_get_clipping_plane (GthreeRenderer     *renderer,
                                    int                 index)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (index < 0 || index >= priv->clipping_planes->len)
    return NULL;

  return &g_array_index (priv->clipping_planes, graphene_plane_t, index);
}

void
gthree_renderer_set_clipping_plane (GthreeRenderer     *renderer,
                                    int                 index,
                                    const graphene_plane_t *plane)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (index < 0 || index >= priv->clipping_planes->len)
    return;

  g_array_index (priv->clipping_planes, graphene_plane_t, index) = *plane;
}

void
gthree_renderer_add_clipping_plane (GthreeRenderer     *renderer,
                                    const graphene_plane_t *plane)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  g_array_append_val (priv->clipping_planes, *plane);
}

void
gthree_renderer_remove_clipping_plane (GthreeRenderer *renderer,
                                       int             index)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (index < 0 || index >= priv->clipping_planes->len)
    return;

  g_array_remove_index (priv->clipping_planes, index);
}

void
gthree_renderer_remove_all_clipping_planes (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  g_array_set_size (priv->clipping_planes, 0);
}

GthreeRenderTarget *
gthree_renderer_get_render_target (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  return priv->current_render_target;
}


static void
update_multisample_render_target (GthreeRenderer *renderer,
                                  GthreeRenderTarget *render_target)
{
  // TODO
}

void
gthree_renderer_set_render_target (GthreeRenderer *renderer,
                                   GthreeRenderTarget *render_target,
                                   int active_cube_target,
                                   int active_mipmap_level)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  gboolean is_cube;
  int framebuffer;
  int pixel_ratio;

  gthree_renderer_push_current (renderer);

  if (render_target)
    g_object_ref (render_target);

  g_clear_object (&priv->current_render_target);
  priv->current_render_target = render_target;

  if (render_target != NULL)
    gthree_render_target_realize (render_target, renderer);

  framebuffer = priv->window_framebuffer;
  is_cube = FALSE;

  if (render_target)
    {
#ifdef TODO
      if ( renderTarget.isWebGLRenderTargetCube )
        {
          var __webglFramebuffer = properties.get( renderTarget ).__webglFramebuffer;
          framebuffer = __webglFramebuffer[ activeCubeFace || 0 ];
          isCube = true;
        }
      else if ( renderTarget.isWebGLMultisampleRenderTarget )
        {
          framebuffer = properties.get( renderTarget ).__webglMultisampledFramebuffer;
        }
      else
#endif
        {
          framebuffer = gthree_render_target_get_gl_framebuffer (render_target);
        }

      graphene_rect_init_from_rect (&priv->current_viewport,
                                    gthree_render_target_get_viewport (render_target));

      pixel_ratio = 1;
#ifdef TODO
      _currentScissor.copy( renderTarget.scissor );
      _currentScissorTest = renderTarget.scissorTest;
#endif
    }
  else
    {
      graphene_rect_init_from_rect (&priv->current_viewport,
                                    &priv->viewport);
#ifdef TODO
      _currentScissor.copy( _scissor ).multiplyScalar( _pixelRatio );
      _currentScissorTest = _scissorTest;
#endif
      pixel_ratio = priv->pixel_ratio;
    }

  /* We always set the framebuffer, because something other than the renderer may have changed it
     inbetween. For instance, a gthree_render_target_download() will do so */
  glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
  priv->current_framebuffer = framebuffer;

  glViewport (graphene_rect_get_x (&priv->current_viewport) * pixel_ratio,
              graphene_rect_get_y (&priv->current_viewport) * pixel_ratio,
              graphene_rect_get_width (&priv->current_viewport) * pixel_ratio,
              graphene_rect_get_height (&priv->current_viewport) * pixel_ratio);
#ifdef TODO
  state.scissor( _currentScissor );
  state.setScissorTest( _currentScissorTest );
#endif

  if (is_cube)
    {
#ifdef TODO
      var textureProperties = properties.get( renderTarget.texture );
      _gl.framebufferTexture2D( _gl.FRAMEBUFFER, _gl.COLOR_ATTACHMENT0,
                                _gl.TEXTURE_CUBE_MAP_POSITIVE_X + ( activeCubeFace || 0 ),
                                textureProperties.__webglTexture,
                                activeMipMapLevel || 0 );
#endif
    }

  gthree_renderer_pop_current (renderer);
}


static void
gthree_set_default_gl_state (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  glClearColor (0, 0, 0, 1);
  graphene_vec4_init (&priv->old_clear_color, 0, 0, 0, 1);
  glClearDepth (1);
  glClearStencil (0);

  glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  glFrontFace (GL_CCW);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);

  glEnable (GL_BLEND);
  glBlendEquation (GL_FUNC_ADD);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport (graphene_rect_get_x (&priv->current_viewport) * priv->pixel_ratio,
              graphene_rect_get_y (&priv->current_viewport) * priv->pixel_ratio,
              graphene_rect_get_width (&priv->current_viewport) * priv->pixel_ratio,
              graphene_rect_get_height (&priv->current_viewport) * priv->pixel_ratio);
};

void
gthree_renderer_clear (GthreeRenderer *renderer,
                       gboolean color,
                       gboolean depth,
                       gboolean stencil)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  gthree_renderer_push_current (renderer);

  if (color)
    set_clear_color (renderer, &priv->clear_color, 1);

  clear (color, depth, stencil);

  gthree_renderer_pop_current (renderer);
}

void
gthree_renderer_clear_depth (GthreeRenderer *renderer)
{
  gthree_renderer_push_current (renderer);

  clear (FALSE, TRUE, FALSE);

  gthree_renderer_pop_current (renderer);
}

void
gthree_renderer_clear_color (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  gthree_renderer_push_current (renderer);

  set_clear_color (renderer, &priv->clear_color, 1);

  clear (TRUE, FALSE, FALSE);

  gthree_renderer_pop_current (renderer);
}

static void
set_material_faces (GthreeRenderer *renderer,
                    GthreeMaterial *material)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeSide side = gthree_material_get_side (material);
  gboolean double_sided = side == GTHREE_SIDE_DOUBLE;
  gboolean flip_sided = side == GTHREE_SIDE_BACK;

  if (priv->old_double_sided != double_sided )
    {
      if (double_sided)
        glDisable (GL_CULL_FACE);
      else
        glEnable (GL_CULL_FACE);

      priv->old_double_sided = double_sided;
    }

  if (priv->old_flip_sided != flip_sided ) {
    if (flip_sided)
      glFrontFace (GL_CW);
    else
      glFrontFace (GL_CCW);

    priv->old_flip_sided = flip_sided;
  }
}

static void
set_depth_test (GthreeRenderer *renderer,
                gboolean depth_test)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_depth_test != depth_test)
    {
      if (depth_test)
        glEnable (GL_DEPTH_TEST);
      else
        glDisable (GL_DEPTH_TEST);

      priv->old_depth_test = depth_test;
    }
}

static void
set_depth_write (GthreeRenderer *renderer,
                 gboolean depth_write)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_depth_write != depth_write)
    {
      glDepthMask (depth_write);
      priv->old_depth_write = depth_write;
    }
}

static void
set_line_width (GthreeRenderer *renderer,
                float line_width)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_line_width != line_width)
    {
      glLineWidth (line_width);
      priv->old_line_width = line_width;
    }
}

static void
set_polygon_offset (GthreeRenderer *renderer,
                    gboolean polygon_offset,
                    float factor, float units)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_polygon_offset != polygon_offset)
    {
      if (polygon_offset)
        glEnable (GL_POLYGON_OFFSET_FILL);
      else
        glDisable (GL_POLYGON_OFFSET_FILL);

      priv->old_polygon_offset = polygon_offset;
    }

  if (polygon_offset && (priv->old_polygon_offset_factor != factor || priv->old_polygon_offset_units != units ))
    {
      glPolygonOffset (factor, units);
      priv->old_polygon_offset_factor = factor;
      priv->old_polygon_offset_units = units;
    }
}

static void
set_clear_color (GthreeRenderer *renderer,
                 const graphene_vec3_t *color,
                 float alpha)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  graphene_vec4_t c4;

  graphene_vec4_init_from_vec3 (&c4, color, alpha);
  if (!graphene_vec4_equal (&c4, &priv->old_clear_color))
    {
      glClearColor (graphene_vec4_get_x (&c4),
                    graphene_vec4_get_y (&c4),
                    graphene_vec4_get_z (&c4),
                    graphene_vec4_get_w (&c4));
      priv->old_clear_color = c4;
    }
}

static void
set_blending (GthreeRenderer *renderer,
              GthreeBlendMode blending,
              guint blend_equation,
              guint blend_src,
              guint blend_dst)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (blending != priv->old_blending)
    {
      switch (blending)
        {
        default:
        case GTHREE_BLEND_NO:
          glDisable (GL_BLEND);
          break;

        case GTHREE_BLEND_NORMAL:
          glEnable (GL_BLEND);
          glBlendEquationSeparate (GL_FUNC_ADD, GL_FUNC_ADD);
          glBlendFuncSeparate (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          break;

        case GTHREE_BLEND_ADDITIVE:
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_SRC_ALPHA, GL_ONE);
          break;

        case GTHREE_BLEND_SUBTRACTIVE:
          // TODO: Find blendFuncSeparate() combination
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
          break;

        case GTHREE_BLEND_MULTIPLY:
          // TODO: Find blendFuncSeparate() combination
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_ZERO, GL_SRC_COLOR);
          break;

        case GTHREE_BLEND_CUSTOM:
          glEnable (GL_BLEND);
          break;
        }
      priv->old_blending = blending;
    }

  if (blending == GTHREE_BLEND_CUSTOM)
    {
      if (blend_equation != priv->old_blend_equation)
        {
          glBlendEquation (blend_equation);
          priv->old_blend_equation = blend_equation;
        }

      if (blend_src != priv->old_blend_src || blend_dst != priv->old_blend_dst)
        {
          glBlendFunc (blend_src, blend_dst);

          priv->old_blend_src = blend_src;
          priv->old_blend_dst = blend_dst;
        }
    }
  else
    {
      priv->old_blend_equation = -1;
      priv->old_blend_src = -1;
      priv->old_blend_dst = -1;
    }
}

static void
project_object (GthreeRenderer *renderer,
                GthreeScene    *scene,
                GthreeObject   *object,
                GthreeCamera   *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeObject *child;
  GthreeObjectIter iter;
  float z = 0;

  if (!gthree_object_get_visible (object))
    return;

  if (gthree_object_check_layer (object, gthree_object_get_layer_mask (GTHREE_OBJECT (camera))))
    {
      if (GTHREE_IS_GROUP (object))
        {
#if 0
          groupOrder = object.renderOrder;
#endif
        }
      else if (GTHREE_IS_LIGHT (object))
        {
          priv->lights = g_list_append (priv->lights, object);
          if (gthree_object_get_cast_shadow (object))
            priv->shadows = g_list_append (priv->shadows, object);
        }
      else if (GTHREE_IS_MESH (object) || GTHREE_IS_LINE_SEGMENTS (object) || GTHREE_IS_SPRITE (object) || GTHREE_IS_POINTS (object))
        {
          if (GTHREE_IS_SKINNED_MESH (object))
            {
              GthreeSkeleton *skeleton = gthree_skinned_mesh_get_skeleton (GTHREE_SKINNED_MESH (object));
              if (skeleton)
                gthree_skeleton_update (skeleton);
            }

          if (!gthree_object_get_is_frustum_culled (object) || gthree_object_is_in_frustum (object, &priv->frustum))
            {
              gthree_object_update (object, renderer);

              if (priv->sort_objects)
                {
                  graphene_vec4_t vector;

                  /* Get position */
                  graphene_matrix_get_row (gthree_object_get_world_matrix (object), 3, &vector);

                  /* project object position to screen */
                  graphene_matrix_transform_vec4 (&priv->proj_screen_matrix, &vector, &vector);

                  z = graphene_vec4_get_z (&vector) / graphene_vec4_get_w (&vector);
                }

              priv->current_render_list->current_z = z;

              gthree_object_fill_render_list (object, priv->current_render_list);
            }
        }
    }

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    project_object (renderer, scene, child, camera);
}

static void
material_apply_light_setup (GthreeUniforms *m_uniforms,
                            GthreeLightSetup *light_setup,
                            gboolean update_only)
{
  gthree_uniforms_set_vec3 (m_uniforms, "ambientLightColor", &light_setup->ambient);

  gthree_uniforms_set_uarray (m_uniforms, "directionalLights", light_setup->directional, update_only);
  gthree_uniforms_set_uarray (m_uniforms, "pointLights", light_setup->point, update_only);
  gthree_uniforms_set_uarray (m_uniforms, "spotLights", light_setup->spot, update_only);

  gthree_uniforms_set_texture_array (m_uniforms, "directionalShadowMap", light_setup->directional_shadow_map);
  gthree_uniforms_set_matrix4_array (m_uniforms, "directionalShadowMatrix", light_setup->directional_shadow_map_matrix);

  gthree_uniforms_set_texture_array (m_uniforms, "spotShadowMap", light_setup->spot_shadow_map);
  gthree_uniforms_set_matrix4_array (m_uniforms, "spotShadowMatrix", light_setup->spot_shadow_map_matrix);

  gthree_uniforms_set_texture_array (m_uniforms, "pointShadowMap", light_setup->point_shadow_map);
  gthree_uniforms_set_matrix4_array (m_uniforms, "pointShadowMatrix", light_setup->point_shadow_map_matrix);
}

static GthreeProgram *
init_material (GthreeRenderer *renderer,
               GthreeMaterial *material,
               gpointer fog,
               GthreeObject *object)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeProgram *program;
  GthreeShader *shader;
  GthreeProgramParameters parameters = {0};
  GthreeUniforms *m_uniforms;
  int max_bones;
  GthreeMaterialProperties *material_properties = gthree_material_get_properties (material);

  shader = gthree_material_get_shader (material);

  //material.addEventListener( 'dispose', onMaterialDispose );
  //var u, a, identifiers, i, parameters, maxBones, maxShadows, shaderID;


  parameters.precision = GTHREE_PRECISION_HIGH;
  parameters.supports_vertex_textures = priv->supports_vertex_textures;
  // TODO: Get encoding from currentRenderTarget if set
  parameters.output_encoding = GTHREE_ENCODING_FORMAT_GAMMA;
  parameters.physically_correct_lights = priv->physically_correct_lights;

  gthree_material_set_params (material, &parameters);
  parameters.num_dir_lights = priv->light_setup.directional->len;
  parameters.num_point_lights = priv->light_setup.point->len;
  parameters.num_spot_lights = priv->light_setup.spot->len;

  max_bones = 0;
  if (GTHREE_IS_SKINNED_MESH (object))
    {
      GthreeSkeleton *skeleton = gthree_skinned_mesh_get_skeleton (GTHREE_SKINNED_MESH (object));
      if (skeleton)
        max_bones = gthree_skeleton_get_n_bones (skeleton);
      // TODO: Limit max bones to GPU specs
    }

  parameters.max_bones = max_bones;
  parameters.skinning = GTHREE_IS_MESH_MATERIAL (material) && gthree_mesh_material_get_skinning (GTHREE_MESH_MATERIAL (material));

  parameters.morph_targets = GTHREE_IS_MESH_MATERIAL (material) && gthree_mesh_material_get_morph_targets (GTHREE_MESH_MATERIAL (material));
  parameters.morph_normals = GTHREE_IS_MESH_MATERIAL (material) && gthree_mesh_material_get_morph_normals (GTHREE_MESH_MATERIAL (material));

  parameters.num_clipping_planes = priv->num_clipping_planes;

  parameters.shadow_map_enabled = priv->shadowmap_enabled && gthree_object_get_receive_shadow (object) && priv->shadows != NULL;
  parameters.shadow_map_type = priv->shadowmap_type;

#ifdef TODO
  parameters =
    {
    lightMap: !! material.lightMap,
    bumpMap: !! material.bumpMap,
    normalMap: !! material.normalMap,
    specularMap: !! material.specularMap,
    alphaMap: !! material.alphaMap,

    fog: fog,
    useFog: material.fog,
    fogExp: fog instanceof THREE.FogExp2,

    logarithmicDepthBuffer: _logarithmicDepthBuffer,

    useVertexTexture: _supportsBoneTextures && object && object.skeleton && object.skeleton.useVertexTexture,
    };
#endif

  program = gthree_program_cache_get (priv->program_cache, shader, &parameters, renderer);
  g_clear_object (&material_properties->program);
  material_properties->program = program;

  // TODO: thee.js uses the lightstate current_hash and other stuff to avoid some stuff here?
  // I think it caches the material uniforms we calculate here and avoid reloading if switching to a new program?

  if (GTHREE_IS_MESH_MATERIAL (material) &&
      gthree_mesh_material_get_morph_targets (GTHREE_MESH_MATERIAL (material)))
    {
      GHashTable *program_attributes = gthree_program_get_attribute_locations (program);
      int num_supported = 0;

      for (int i = 0; i < MAX_MORPH_TARGETS; i++ )
        {
          g_autofree char *attr = g_strdup_printf ("morphTarget%d", i);

          if (g_hash_table_lookup (program_attributes, attr) != NULL)
            num_supported++;
        }
      gthree_mesh_material_set_num_supported_morph_targets (GTHREE_MESH_MATERIAL (material),
                                                            num_supported);
    }

  if (GTHREE_IS_MESH_MATERIAL (material) &&
      gthree_mesh_material_get_morph_normals (GTHREE_MESH_MATERIAL (material)))
    {
      GHashTable *program_attributes = gthree_program_get_attribute_locations (program);
      int num_supported = 0;

      for (int i = 0; i < MAX_MORPH_NORMALS; i++ )
        {
          g_autofree char *attr = g_strdup_printf ("morphNormal%d", i);

          if (g_hash_table_lookup (program_attributes, attr) != NULL)
            num_supported++;
        }

      gthree_mesh_material_set_num_supported_morph_normals (GTHREE_MESH_MATERIAL (material),
                                                            num_supported);
    }

  m_uniforms = gthree_shader_get_uniforms (shader);

  // store the light setup it was created for
  material_properties->light_hash = priv->light_setup.hash;

  if (!GTHREE_IS_SHADER_MATERIAL (material)
#ifdef TODO
      || material.clipping === true
#endif
      )
    {
      GthreeUniform *uni =  gthree_uniforms_lookup (m_uniforms, q_clippingPlanes);
      if (uni == NULL)
        {
          uni = gthree_uniform_newq (q_clippingPlanes, GTHREE_UNIFORM_TYPE_FLOAT4_ARRAY);
          gthree_uniforms_add (m_uniforms, uni); // takes ownership
        }
    }

  material_apply_light_setup (m_uniforms, &priv->light_setup, FALSE);

  gthree_shader_update_uniform_locations_for_program (shader, program);

  return NULL;
}

#if 0
static void
print_matrix4 (float *s)
{
  int i,j;
  for (i = 0; i < 4; i++)
    {
      if (i == 0)
        g_print("[ ");
      else
        g_print("  ");
      for (j = 0; j < 4; j++)
        {
          if (j != 0)
            g_print (", ");
          g_print ("%f", *s++);
        }
      if (i == 3)
        g_print("]\n");
      else
        g_print("\n");
    }
}
#endif

static void
setup_lights (GthreeRenderer *renderer, GthreeCamera *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeLightSetup *setup = &priv->light_setup;
  GList *l;

  graphene_vec3_init (&setup->ambient, 0, 0, 0);

  g_ptr_array_set_size (setup->directional, 0);
  g_ptr_array_set_size (setup->directional_shadow_map, 0);
  g_array_set_size (setup->directional_shadow_map_matrix, 0);
  g_ptr_array_set_size (setup->point, 0);
  g_ptr_array_set_size (setup->point_shadow_map, 0);
  g_array_set_size (setup->point_shadow_map_matrix, 0);
  g_ptr_array_set_size (setup->spot, 0);
  g_ptr_array_set_size (setup->spot_shadow_map, 0);
  g_array_set_size (setup->spot_shadow_map_matrix, 0);

  for (l = priv->lights; l != NULL; l = l->next)
    {
      GthreeLight *light = l->data;

      gthree_light_setup (light, camera, setup);
    }

  setup->hash.num_directional = setup->directional->len;
  setup->hash.num_point = setup->point->len;
  setup->hash.num_spot = setup->spot->len;
  setup->hash.num_shadow = setup->shadow->len;
}

static void *
project_planes (GthreeRenderer *renderer,
                const graphene_plane_t *planes,
                int n_planes,
                GthreeCamera *camera,
                int dst_offset,
                gboolean skip_transform)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (n_planes != 0)
    {
      guint flat_size = dst_offset + n_planes * 4;
      graphene_matrix_t viewNormalMatrix;
      float *dst_array;

      g_array_set_size (priv->clipping_state, flat_size);
      dst_array = (float *)priv->clipping_state->data;

      if (!skip_transform)
        {
          const graphene_matrix_t *viewMatrix = gthree_camera_get_world_inverse_matrix (camera);

          /* Get normal matrix */
          graphene_matrix_inverse (viewMatrix, &viewNormalMatrix);
          graphene_matrix_transpose (&viewNormalMatrix, &viewNormalMatrix);

          for (int i = 0, i4 = dst_offset; i != n_planes; i++, i4 += 4)
            {
              graphene_plane_t transformed_plane;
              graphene_vec3_t normal;

              graphene_plane_transform (&planes[i], viewMatrix, &viewNormalMatrix, &transformed_plane);
              graphene_plane_get_normal (&transformed_plane, &normal);

              dst_array[i4+0] = graphene_vec3_get_x (&normal);
              dst_array[i4+1] = graphene_vec3_get_y (&normal);
              dst_array[i4+2] = graphene_vec3_get_z (&normal);
              dst_array[i4+3] = graphene_plane_get_constant (&transformed_plane);
            }
        }
      else
        {
          for (int i = 0, i4 = dst_offset; i != n_planes; i++, i4 += 4)
            {
              graphene_vec3_t normal;
              float constant;

              graphene_plane_get_normal (&planes[i], &normal);
              constant = graphene_plane_get_constant (&planes[i]);

              dst_array[i4+0] = graphene_vec3_get_x (&normal);
              dst_array[i4+1] = graphene_vec3_get_y (&normal);
              dst_array[i4+2] = graphene_vec3_get_z (&normal);
              dst_array[i4+3] = constant;
            }
        }
    }

  priv->num_clipping_planes = n_planes;

  return NULL;
}

static gboolean
clipping_init (GthreeRenderer *renderer,
               GthreeCamera *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  gboolean enabled;

  enabled =
    priv->clipping_planes->len != 0 ||
    // enable state of previous frame - the clipping code has to
    // run another frame in order to reset the state:
    priv->old_num_global_planes != 0;

  project_planes (renderer,
                  (const graphene_plane_t *)priv->clipping_planes->data,
                  priv->clipping_planes->len,
                  camera,
                  0, FALSE);

  priv->old_num_global_planes = priv->clipping_planes->len;

  return enabled;
}

static void
clipping_begin_shadows (GthreeRenderer *renderer)
{
  // TODO
}

static void
clipping_end_shadows (GthreeRenderer *renderer)
{
  // TODO
}

#define SHADER_MAP_MORPHING_FLAG (1<<0)
#define SHADER_MAP_SKINNING_FLAG (1<<1)

static GthreeMaterial *
getDepthMaterial (GthreeRenderer *renderer,
                  GthreeObject *object,
                  GthreeGeometry *geometry,
                  GthreeMaterial *material,
                  gboolean isPointLight,
                  const graphene_vec3_t *lightPositionWorld,
                  float shadowCameraNear,
                  float shadowCameraFar)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeMaterial *result = NULL;
  GthreeMaterial *customMaterial = NULL;
  GPtrArray *materialVariants = NULL;

  if (priv->shadowmap_depth_materials == NULL)
    {
      priv->shadowmap_depth_materials = g_ptr_array_new_with_free_func (g_object_unref);
      priv->shadowmap_distance_materials = g_ptr_array_new_with_free_func (g_object_unref);

      for (int i = 0; i < 4; i++)
        {
          gboolean useMorphing = (i & SHADER_MAP_MORPHING_FLAG) != 0;
          gboolean useSkinning = (i & SHADER_MAP_SKINNING_FLAG) != 0;

          GthreeMeshDepthMaterial *m = gthree_mesh_depth_material_new ();
          gthree_mesh_depth_material_set_depth_packing_format (m, GTHREE_DEPTH_PACKING_FORMAT_RGBA);

          gthree_mesh_material_set_morph_targets (GTHREE_MESH_MATERIAL (m), useMorphing);
          gthree_mesh_material_set_skinning (GTHREE_MESH_MATERIAL (m), useSkinning);
          g_ptr_array_add (priv->shadowmap_depth_materials, m);

          GthreeMeshDistanceMaterial *m2 = gthree_mesh_distance_material_new ();
          gthree_mesh_material_set_morph_targets (GTHREE_MESH_MATERIAL (m2), useMorphing);
          gthree_mesh_material_set_skinning (GTHREE_MESH_MATERIAL (m2), useSkinning);
          g_ptr_array_add (priv->shadowmap_distance_materials, m2);
        }
    }

  materialVariants = priv->shadowmap_depth_materials;
#ifdef TODO
  var customMaterial = object.customDepthMaterial;
#endif

  if (isPointLight)
    {
      materialVariants = priv->shadowmap_distance_materials;
#ifdef TODO
      customMaterial = object.customDistanceMaterial;
#endif
    }

  if (!customMaterial)
    {
      gboolean useMorphing = FALSE;
      gboolean useSkinning = FALSE;
      int variantIndex = 0;

#ifdef TODO
      if (material.morphTargets && geometry)
        useMorphing = geometry.morphAttributes && geometry.morphAttributes.position && geometry.morphAttributes.position.length > 0;
#endif

#ifdef TODO
      if (GTHREE_IS_SKINNED_MESH (object) && material.skinning === false ) {
        console.warn( 'THREE.WebGLShadowMap: THREE.SkinnedMesh with material.skinning set to false:', object );
      }
#endif

#ifdef TODO
      useSkinning = object.isSkinnedMesh && material.skinning;
#endif

      if (useMorphing)
        variantIndex |= SHADER_MAP_MORPHING_FLAG;
      if (useSkinning)
        variantIndex |= SHADER_MAP_SKINNING_FLAG;

      result = g_ptr_array_index (materialVariants, variantIndex);
    }
  else
    {
      result = customMaterial;
    }

#ifdef TODO
  if ( _renderer.localClippingEnabled &&
       material.clipShadows === true &&
       material.clippingPlanes.length !== 0 ) {
    // in this case we need a unique material instance reflecting the
    // appropriate state

    var keyA = result.uuid, keyB = material.uuid;
    var materialsForVariant = _materialCache[ keyA ];

    if ( materialsForVariant === undefined ) {
      materialsForVariant = {};
      _materialCache[ keyA ] = materialsForVariant;
    }

    var cachedMaterial = materialsForVariant[ keyB ];
    if ( cachedMaterial === undefined ) {
      cachedMaterial = result.clone();
      materialsForVariant[ keyB ] = cachedMaterial;
    }

    result = cachedMaterial;
  }
#endif

  gthree_material_set_is_visible (result, gthree_material_get_is_visible (material));
  if (GTHREE_MESH_MATERIAL (material))
    gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (result),
                                           gthree_mesh_material_get_is_wireframe (GTHREE_MESH_MATERIAL (material)));

#ifdef TODO
  result.side = ( material.shadowSide != null ) ? material.shadowSide : shadowSide[ material.side ];
#endif

#ifdef TODO
  result.clipShadows = material.clipShadows;
  result.clippingPlanes = material.clippingPlanes;
  result.clipIntersection = material.clipIntersection;
#endif

#ifdef TODO
  result.wireframeLinewidth = material.wireframeLinewidth;
  result.linewidth = material.linewidth;
#endif

  if (isPointLight && GTHREE_IS_MESH_DISTANCE_MATERIAL (result))
    {
      gthree_mesh_distance_material_set_reference_point (GTHREE_MESH_DISTANCE_MATERIAL (result),
                                                         lightPositionWorld);
      gthree_mesh_distance_material_set_near_distance (GTHREE_MESH_DISTANCE_MATERIAL (result),
                                                       shadowCameraNear);
      gthree_mesh_distance_material_set_far_distance (GTHREE_MESH_DISTANCE_MATERIAL (result),
                                                      shadowCameraFar);
    }

  return result;
}

static void
shadow_map_render_object (GthreeRenderer *renderer,
                          GthreeObject *object,
                          GthreeCamera *camera,
                          graphene_frustum_t *frustum,
                          GthreeCamera *shadow_camera,
                          const graphene_vec3_t *_lightPositionWorld,
                          gboolean is_point_light)
{
  GthreeObject *child;
  GthreeObjectIter iter;

  if (!gthree_object_get_visible (object))
    return;

  if (gthree_object_check_layer (object, gthree_object_get_layer_mask (GTHREE_OBJECT (camera))) &&
      (GTHREE_IS_MESH (object) || GTHREE_IS_LINE_SEGMENTS (object) || GTHREE_IS_POINTS (object)))
    {
      if (gthree_object_get_cast_shadow (object) &&
          (!gthree_object_get_is_frustum_culled (object) || gthree_object_is_in_frustum (object, frustum)))
        {
          GthreeGeometry *geometry = NULL;
          GthreeMaterial *material = NULL;
          gboolean uses_groups = FALSE;

          gthree_object_update_matrix_view (object, gthree_camera_get_world_inverse_matrix (shadow_camera));
          gthree_object_update (object, renderer);

          // TODO: Abstract this out into vfuncs
          if (GTHREE_IS_MESH (object))
            {
              // TODO: Handle multi material
              geometry = gthree_mesh_get_geometry (GTHREE_MESH (object));
              uses_groups = gthree_mesh_get_n_materials (GTHREE_MESH (object)) > 1;
              material = gthree_mesh_get_material (GTHREE_MESH (object), 0);
            }
          else
            {
              g_warning ("Unsupported object type for shadows: %s", g_type_name_from_instance ((gpointer)object));
            }

          if (uses_groups)
            {
#ifdef TODO
              var groups = geometry.groups;

              for ( var k = 0, kl = groups.length; k < kl; k ++ )
                {
                  var group = groups[ k ];
                  var groupMaterial = material[ group.materialIndex ];

                  if ( groupMaterial && groupMaterial.visible )
                    {
                      var depthMaterial = getDepthMaterial( object, groupMaterial, is_point_light, _lightPositionWorld, shadow_camera.near, shadow_camera.far );
                      _renderer.renderBufferDirect( shadow_camera, null, geometry, depthMaterial, object, group );
                    }
                }
#endif
            }
          else if (gthree_material_get_is_visible (material))
            {
              GthreeMaterial *depthMaterial = getDepthMaterial (renderer, object, geometry, material, is_point_light, _lightPositionWorld,
                                                                gthree_camera_get_near (shadow_camera), gthree_camera_get_far (shadow_camera));
              GthreeRenderListItem item = { object, geometry, depthMaterial, NULL, 0.0 };
              render_item (renderer, shadow_camera, FALSE, depthMaterial, &item);
            }
        }
    }

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    shadow_map_render_object (renderer, child, camera, frustum, shadow_camera, _lightPositionWorld, is_point_light);
}


static void
render_shadow_map (GthreeRenderer *renderer,
                   GthreeScene *scene,
                   GthreeCamera *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  g_autoptr(GthreeRenderTarget) current_render_target = NULL;
  GList *l;
  int faceCount;
  graphene_vec3_t c;

  if (!priv->shadowmap_enabled)
    return;

  if (!priv->shadowmap_auto_update && !priv->shadowmap_needs_update)
    return;

  if (priv->shadows == NULL)
    return;

  push_debug_group ("rendering shadow maps");

  g_set_object (&current_render_target,  priv->current_render_target);

  // Set GL state for depth map.
  set_blending (renderer, GTHREE_BLEND_NO, 0, 0, 0);
  set_depth_test (renderer, TRUE);

  set_clear_color (renderer, graphene_vec3_init (&c, 1, 1, 1), 1);

#ifdef TODO
  _state.setScissorTest( false );
#endif

  // render depth map
  for (l = priv->shadows; l != NULL; l = l->next)
    {
      GthreeLight *light = l->data;
      GthreeLightShadow *shadow = gthree_light_get_shadow (light);
      graphene_vec4_t cube2DViewPorts[6];

      if (shadow == NULL)
        {
          g_warning ("light has no shadow.");
          continue;
        }

      GthreeCamera *shadow_camera = gthree_light_shadow_get_camera (shadow);

      int shadow_map_width = MIN (gthree_light_shadow_get_map_width (shadow), priv->max_texture_size);
      int shadow_map_height = MIN (gthree_light_shadow_get_map_height (shadow), priv->max_texture_size);

      push_debug_group ("shadow maps light %p", light);

      if (GTHREE_IS_POINT_LIGHT (light))
        {
          int vpWidth = shadow_map_width;
          int vpHeight = shadow_map_height;

          // These viewports map a cube-map onto a 2D texture with the
          // following orientation:
          //
          //  xzXZ
          //   y Y
          //
          // X - Positive x direction
          // x - Negative x direction
          // Y - Positive y direction
          // y - Negative y direction
          // Z - Positive z direction
          // z - Negative z direction

          // positive X
          graphene_vec4_init (&cube2DViewPorts[0],
                              vpWidth * 2, vpHeight, vpWidth, vpHeight);
          // negative X
          graphene_vec4_init (&cube2DViewPorts[1],
                              0, vpHeight, vpWidth, vpHeight );
          // positive Z
          graphene_vec4_init (&cube2DViewPorts[2],
                              vpWidth * 3, vpHeight, vpWidth, vpHeight );
          // negative Z
          graphene_vec4_init (&cube2DViewPorts[3],
                              vpWidth, vpHeight, vpWidth, vpHeight );
          // positive Y
          graphene_vec4_init (&cube2DViewPorts[4],
                              vpWidth * 3, 0, vpWidth, vpHeight );
          // negative Y
          graphene_vec4_init (&cube2DViewPorts[5],
                              vpWidth, 0, vpWidth, vpHeight );

          shadow_map_width *= 4;
          shadow_map_height *= 2;
        }

      GthreeRenderTarget *shadow_map = gthree_light_shadow_get_map (shadow);

      if (shadow_map == NULL)
        {
          shadow_map = gthree_render_target_new (shadow_map_width, shadow_map_height);

          GthreeTexture *texture = gthree_render_target_get_texture (shadow_map);
          gthree_texture_set_mag_filter (texture, GTHREE_FILTER_NEAREST);
          gthree_texture_set_min_filter (texture, GTHREE_FILTER_NEAREST);

          gthree_light_shadow_set_map (shadow, shadow_map);

#ifdef TODO
          texture.name = light.name + ".shadowMap";
#endif

          gthree_camera_update (shadow_camera);
        }

      if (GTHREE_IS_SPOT_LIGHT_SHADOW (shadow))
        gthree_spot_light_shadow_update (GTHREE_SPOT_LIGHT_SHADOW (shadow), GTHREE_SPOT_LIGHT (light));

      graphene_matrix_t *shadowMatrix = gthree_light_shadow_get_matrix (shadow);

      graphene_vec4_t _lightPositionWorld4;
      graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)),
                               3, &_lightPositionWorld4);
      graphene_vec3_t _lightPositionWorld;
      graphene_vec4_get_xyz (&_lightPositionWorld4, &_lightPositionWorld);

      gthree_object_set_position (GTHREE_OBJECT (shadow_camera), &_lightPositionWorld);

      if (GTHREE_IS_POINT_LIGHT (light))
        {
          faceCount = 6;

          // for point lights we set the shadow matrix to be a translation-only matrix
          // equal to inverse of the light's position
          graphene_point3d_t p;
          graphene_matrix_init_translate (shadowMatrix,
                                          graphene_point3d_init (&p,
                                                                 -graphene_vec3_get_x (&_lightPositionWorld),
                                                                 -graphene_vec3_get_y (&_lightPositionWorld),
                                                                 -graphene_vec3_get_z (&_lightPositionWorld)));
        }
      else
        {
          faceCount = 1;

          g_autoptr(GthreeObject) light_target = NULL;
          g_object_get (light, "target", &light_target, NULL);

          graphene_vec4_t _lookTarget;
          graphene_matrix_get_row (gthree_object_get_world_matrix (light_target),
                                   3, &_lookTarget);

          graphene_point3d_t p;
          gthree_object_look_at (GTHREE_OBJECT (shadow_camera),
                                 graphene_point3d_init (&p,
                                                        graphene_vec4_get_x (&_lookTarget),
                                                        graphene_vec4_get_y (&_lookTarget),
                                                        graphene_vec4_get_z (&_lookTarget)));
          gthree_object_update_matrix_world (GTHREE_OBJECT (shadow_camera), FALSE);
          gthree_camera_update_matrix (shadow_camera);

          // compute shadow matrix
          graphene_matrix_init_scale (shadowMatrix, 0.5, 0.5, 0.5);
          graphene_matrix_translate (shadowMatrix, graphene_point3d_init (&p, 0.5, 0.5, 0.5));

          graphene_matrix_multiply (gthree_camera_get_projection_matrix (shadow_camera),
                                    shadowMatrix,
                                    shadowMatrix);
          graphene_matrix_multiply (gthree_camera_get_world_inverse_matrix (shadow_camera),
                                    shadowMatrix,
                                    shadowMatrix);
        }

      gthree_renderer_set_render_target (renderer, shadow_map, 0, 0);
      gthree_renderer_clear (renderer, TRUE, TRUE, TRUE);

      // render shadow map for each cube face (if omni-directional) or
      // run a single pass if not
      for (int face = 0; face < faceCount; face++)
        {
          if (GTHREE_IS_POINT_LIGHT (light))
            {
              graphene_point3d_t p;
              graphene_vec3_t _lookTarget;
              graphene_vec3_add (gthree_object_get_position (GTHREE_OBJECT (shadow_camera)),
                                 &cube_directions[face], &_lookTarget);

              gthree_object_set_up (GTHREE_OBJECT (shadow_camera), &cube_ups[face]);
              gthree_object_look_at (GTHREE_OBJECT (shadow_camera),
                                     graphene_point3d_init (&p,
                                                            graphene_vec3_get_x (&_lookTarget),
                                                            graphene_vec3_get_y (&_lookTarget),
                                                            graphene_vec3_get_z (&_lookTarget)));

              gthree_object_update_matrix_world (GTHREE_OBJECT (shadow_camera), FALSE);
              gthree_camera_update_matrix (shadow_camera);

              graphene_vec4_t *vpDimensions = &cube2DViewPorts[face];

              glViewport (graphene_vec4_get_x (vpDimensions),
                          graphene_vec4_get_y (vpDimensions),
                          graphene_vec4_get_z (vpDimensions),
                          graphene_vec4_get_w (vpDimensions));
            }

          // update camera matrices and frustum
          graphene_matrix_t _projScreenMatrix;
          graphene_frustum_t frustum;

          gthree_camera_get_proj_screen_matrix (shadow_camera, &_projScreenMatrix);
          graphene_frustum_init_from_matrix (&frustum, &_projScreenMatrix);

          // set object matrices & frustum culling
          shadow_map_render_object (renderer, GTHREE_OBJECT (scene), camera, &frustum, shadow_camera,
                                    &_lightPositionWorld,
                                    GTHREE_IS_POINT_LIGHT (light));
        }

      pop_debug_group ();
    }

  priv->shadowmap_needs_update = FALSE;

  gthree_renderer_set_render_target (renderer, current_render_target, 0, 0);

  pop_debug_group ();
}


// If uniforms are marked as clean, they don't need to be loaded to the GPU.
static void
mark_uniforms_lights_needs_update (GthreeUniforms *uniforms, gboolean needs_update)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup (uniforms, q_ambientLightColor);
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);

  uni = gthree_uniforms_lookup (uniforms, q_directionalLights);
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);

  uni = gthree_uniforms_lookup (uniforms, q_pointLights);
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);

  uni = gthree_uniforms_lookup (uniforms, q_directionalLights);
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);

  uni = gthree_uniforms_lookup (uniforms, q_spotLights);
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);

}

static GthreeProgram *
set_program (GthreeRenderer *renderer,
             GthreeCamera *camera,
             gpointer fog,
             GthreeMaterial *material,
             GthreeObject *object)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  gboolean refreshProgram = false;
  gboolean refreshMaterial = false;
  gboolean refreshLights = false;
  GthreeProgram *program;
  GthreeShader *shader;
  GthreeUniforms *m_uniforms;
  GthreeMaterialProperties *material_properties = gthree_material_get_properties (material);

#ifdef TODO
  // This is only needed for local clipping
  if (priv->clipping_enabled)
    {
      if (camera !== priv->current_camera)
        clipping_set_state (renderer, camera);
    }
#endif

  priv->used_texture_units = 0;

  /* Maybe the light state (e.g. nr of lights, or if this is a shaded
     object) changed since we last initialized the material, even if
     the material itself didn't change */
  priv->light_setup.hash.obj_receive_shadow = gthree_object_get_receive_shadow (object) && priv->shadowmap_enabled;
  if (!gthree_material_get_needs_update (material))
    {
      if (!gthree_light_setup_hash_equal (&material_properties->light_hash, &priv->light_setup.hash))
        gthree_material_set_needs_update (material, TRUE);
    }

  if (gthree_material_get_needs_update (material))
    {
      init_material (renderer, material, fog, object);
      gthree_material_set_needs_update (material, FALSE);
    }

  program = material_properties->program;
  shader = gthree_material_get_shader (material);
  m_uniforms = gthree_shader_get_uniforms (shader);

  if (program != priv->current_program )
    {
      gthree_program_use (program);
      priv->current_program = program;

      refreshProgram = TRUE;
      refreshMaterial = TRUE;
      refreshLights = TRUE;
    }

  if (material != priv->current_material)
    {
      priv->current_material = material;
      refreshMaterial = TRUE;
    }

  if (refreshProgram || camera != priv->current_camera)
    {
      const graphene_matrix_t *projection_matrix = gthree_camera_get_projection_matrix (camera);
      float projection_matrixv[16];
      gint proction_matrix_location = gthree_program_lookup_uniform_location (program, q_projectionMatrix);

      graphene_matrix_to_float (projection_matrix, projection_matrixv);
      glUniformMatrix4fv (proction_matrix_location, 1, FALSE, projection_matrixv);

#ifdef TODO
      if ( _logarithmicDepthBuffer )
        glUniform1f (uniform_locations.logDepthBufFC, 2.0 / ( Math.log( camera.far + 1.0 ) / Math.LN2 ));

#endif
      if (camera != priv->current_camera)
        {
          priv->current_camera = camera;

          // lighting uniforms depend on the camera so enforce an update
          // now, in case this material supports lights - or later, when
          // the next material that does gets activated:
          refreshMaterial = TRUE;	// set to true on material change
          refreshLights = TRUE;		// remains set until update done
        }

      // load material specific uniforms
      // (shader material also gets them for the sake of genericity)

      if (gthree_material_needs_camera_pos (material)
#if TODO
          || material.envMap
#endif
          )
        {
          gint camera_position_location = gthree_program_lookup_uniform_location (program, q_cameraPosition);
          if (camera_position_location >= 0)
            {
              const graphene_matrix_t *camera_matrix_world = gthree_object_get_world_matrix (GTHREE_OBJECT (camera));
              graphene_vec4_t pos;
              graphene_matrix_get_row (camera_matrix_world, 3, &pos);
              glUniform3f (camera_position_location,
                           graphene_vec4_get_x (&pos), graphene_vec4_get_y (&pos), graphene_vec4_get_z (&pos));
            }
        }

      if (gthree_material_needs_view_matrix (material))
        {
          gint view_matrix_location = gthree_program_lookup_uniform_location (program, q_viewMatrix);
          if (view_matrix_location >= 0)
            {
              const graphene_matrix_t *m = gthree_camera_get_world_inverse_matrix (camera);
              float floats[16];
              graphene_matrix_to_float (m, floats);
              glUniformMatrix4fv (view_matrix_location, 1, FALSE, floats);
            }
        }
    }

  // skinning uniforms must be set even if material didn't change
  // auto-setting of texture unit for bone texture must go before other textures
  // not sure why, but otherwise weird things happen

  if (GTHREE_IS_MESH_MATERIAL (material) && gthree_mesh_material_get_skinning (GTHREE_MESH_MATERIAL (material)))
    {
      GthreeSkeleton *skeleton = NULL;
      gint bind_matrix_location = gthree_program_lookup_uniform_location (program, q_bindMatrix);
      gint bind_matrix_inverse_location = gthree_program_lookup_uniform_location (program, q_bindMatrixInverse);

      if (GTHREE_IS_SKINNED_MESH (object))
        {
          const graphene_matrix_t *bind_matrix = gthree_skinned_mesh_get_bind_matrix (GTHREE_SKINNED_MESH (object));
          const graphene_matrix_t *inv_bind_matrix = gthree_skinned_mesh_get_inverse_bind_matrix (GTHREE_SKINNED_MESH (object));
          if (bind_matrix)
            {
              float floats[16];
              graphene_matrix_to_float (bind_matrix, floats);
              glUniformMatrix4fv (bind_matrix_location, 1, FALSE, floats);
            }
          if (inv_bind_matrix)
            {
              float floats[16];
              graphene_matrix_to_float (inv_bind_matrix, floats);
              glUniformMatrix4fv (bind_matrix_inverse_location, 1, FALSE, floats);
            }
          skeleton = gthree_skinned_mesh_get_skeleton (GTHREE_SKINNED_MESH (object));
        }

#ifdef TODO
      if ( _supportsBoneTextures && object.skeleton && object.skeleton.useVertexTexture )
        {
          if ( uniform_locations.boneTexture !== null )
            {
              var textureUnit = getTextureUnit();
              glUniform1i( uniform_locations.boneTexture, textureUnit );
              _this.setTexture( object.skeleton.boneTexture, textureUnit );
            }

          if ( uniform_locations.boneTextureWidth !== null )
            glUniform1i( uniform_locations.boneTextureWidth, object.skeleton.boneTextureWidth );

          if ( uniform_locations.boneTextureHeight !== null )
            glUniform1i( uniform_locations.boneTextureHeight, object.skeleton.boneTextureHeight );

        }
      else
#endif

        if (skeleton)
          {
            /* Unclear why we need [0] here rather than boneMatrices? Do we ever need [1]?? */
            gint bone_matrices_location = gthree_program_lookup_uniform_location (program,
                                                                                  g_quark_from_static_string ("boneMatrices[0]"));
            float *bone_matrices = gthree_skeleton_get_bone_matrices (skeleton);
            if (bone_matrices_location >= 0)
              glUniformMatrix4fv (bone_matrices_location, gthree_skeleton_get_n_bones (skeleton), FALSE, bone_matrices);
          }
    }

  if (priv->clipping_enabled)
    {
      // We always update the clipping planes because they changes outside of the material
      GthreeUniform *uni =  gthree_uniforms_lookup (m_uniforms, q_clippingPlanes);
      if (uni)
        {
          gthree_uniform_set_float4_array (uni, priv->clipping_state);
          gthree_uniform_set_needs_update (uni, TRUE);
        }
    }

  if ( refreshMaterial )
    {
      if (gthree_material_needs_lights (material))
        {
          mark_uniforms_lights_needs_update (m_uniforms, refreshLights);
          if (refreshLights)
            {
              /* We marked the uniforms so they are uploaded, but we also need to sync
               * the actual values from the light uniforms into the material uniforms
               * (these are not the same because the location differs for each instance)
               */
              material_apply_light_setup (m_uniforms, &priv->light_setup, TRUE);
            }
        }

      gthree_material_set_uniforms (material, m_uniforms, camera, renderer);

#if TODO
      // refresh uniforms common to several materials
      if ( fog && material.fog )
        refreshUniformsFog( m_uniforms, fog );
#endif


      // refresh single material specific uniforms

#if TODO
      if ( material instanceof THREE.LineBasicMaterial )
        {
          refreshUniformsLine( m_uniforms, material );
        }
      else if ( material instanceof THREE.LineDashedMaterial )
        {
          refreshUniformsLine( m_uniforms, material );
          refreshUniformsDash( m_uniforms, material );
        }
      else if ( material instanceof THREE.PointCloudMaterial )
        {
          refreshUniformsParticle( m_uniforms, material );
        }

      if ( object.receiveShadow && ! material._shadowPass )
        {
          refreshUniformsShadow( m_uniforms, lights );
        }
#endif

      // load common uniforms
      gthree_uniforms_load (m_uniforms, renderer);
    }
  else
    {
      // We always reload the clipping planes because it may change outside of the material
      GthreeUniform *uni =  gthree_uniforms_lookup (m_uniforms, q_clippingPlanes);
      if (uni)
        gthree_uniform_load (uni, renderer);
    }


  gthree_object_set_direct_uniforms (object, program, renderer);

  return program;
}

static void
init_attributes (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  int i;

  for (i = 0; i < G_N_ELEMENTS(priv->new_attributes); i++)
    priv->new_attributes[i] = 0;
}

static void
enable_attribute (GthreeRenderer *renderer,
                  guint attribute)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->new_attributes[attribute] = 1;
  if (priv->enabled_attributes[attribute] == 0)
    {
      glEnableVertexAttribArray(attribute);
      priv->enabled_attributes[attribute] = 1;
    }
}

static void
disable_unused_attributes (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  int i;

  for (i = 0; i < G_N_ELEMENTS(priv->new_attributes); i++)
    {
      if (priv->enabled_attributes[i] != priv->new_attributes[i])
        {
          glDisableVertexAttribArray(i);
          priv->enabled_attributes[i] = 0;
        }
    }
}

static void
setup_vertex_attributes (GthreeRenderer *renderer,
                         GthreeMaterial *material,
                         GthreeProgram *program,
                         GthreeGeometry *geometry)
{
  GHashTable *program_attributes;
  GHashTableIter iter;
  gpointer key, value;

  init_attributes (renderer);

  program_attributes = gthree_program_get_attribute_locations (program);

  g_hash_table_iter_init (&iter, program_attributes);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      GQuark nameq = GPOINTER_TO_INT (key);
      const char *name = g_quark_to_string (nameq);
      int program_attribute = GPOINTER_TO_INT (value);

      if (program_attribute >= 0)
        {
          GthreeAttribute *geometry_attribute = gthree_geometry_get_attribute (geometry, name);
          if (geometry_attribute != NULL)
            {
              gboolean normalized = gthree_attribute_get_normalized (geometry_attribute);
              int size = gthree_attribute_get_item_size (geometry_attribute);
              int offset = gthree_attribute_get_item_offset (geometry_attribute);
              int stride = gthree_attribute_get_stride (geometry_attribute);

              int buffer = gthree_attribute_get_gl_buffer (geometry_attribute);
              int type = gthree_attribute_get_gl_type (geometry_attribute);
              int bytes_per_element = gthree_attribute_get_gl_bytes_per_element (geometry_attribute);

              /*
                if ( geometryAttribute.isInstancedBufferAttribute )
                {
                state.enableAttributeAndDivisor( programAttribute, geometryAttribute.meshPerAttribute );
                if ( geometry.maxInstancedCount === undefined )
                {
                geometry.maxInstancedCount = geometryAttribute.meshPerAttribute * geometryAttribute.count;
                }
                }
                else
              */
              {
                enable_attribute (renderer, program_attribute);
              }
              glBindBuffer (GL_ARRAY_BUFFER, buffer);
              glVertexAttribPointer (program_attribute, size, type, normalized, stride * bytes_per_element, GINT_TO_POINTER (offset * bytes_per_element));
            }
          else
            {
              gthree_material_load_default_attribute (material, program_attribute, nameq);
            }
        }
    }



  disable_unused_attributes (renderer);
}

//	var influencesList = {};
//	var morphInfluences = new Float32Array( 8 );

typedef struct {
  int index;
  float value;
} InfluenceInfo;

static int
influence_info_cmp (InfluenceInfo *a, InfluenceInfo *b)
{
  float aa = fabs (a->value);
  float bb = fabs (b->value);

  if (aa == bb)
    return 0;

  if (bb > aa)
    return -1;

  return 1;
}

static void
update_morphtargets (GthreeRenderer *renderer,
                     GthreeMesh *mesh,
                     GthreeGeometry *geometry,
                     GthreeMeshMaterial *material,
                     GthreeProgram *program)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GArray *object_influences = gthree_mesh_get_morph_targets (mesh);
  int length = object_influences->len;
  GArray *influences;
  int i;
  GPtrArray *morphTargets = NULL;
  GPtrArray *morphNormals = NULL;


  if (geometry->influences == NULL)
    {
      geometry->influences = g_array_new (FALSE, FALSE, sizeof (InfluenceInfo));
      for (i = 0; i < length; i++)
        {
          InfluenceInfo info = {i, 0};
          g_array_append_val (geometry->influences, info);
        }
    }
  influences = geometry->influences;

  if (gthree_mesh_material_get_morph_targets (material))
    morphTargets = gthree_geometry_get_morph_attributes (geometry, "position");

  if (gthree_mesh_material_get_morph_normals (material))
    morphNormals = gthree_geometry_get_morph_attributes (geometry, "normal");

  // Remove current morphAttributes
  for (i = 0; i < length; i++)
    {
      InfluenceInfo *info = &g_array_index (influences, InfluenceInfo, i);
      if (info->value != 0)
        {
          if (morphTargets)
            {
              g_autofree char *name = g_strdup_printf ("morphTarget%d", i);
              gthree_geometry_remove_attribute (geometry, name);
            }

          if (morphNormals)
            {
              g_autofree char *name = g_strdup_printf ("morphNormal%d", i);
              gthree_geometry_remove_attribute (geometry, name);
            }
        }
    }

  // Collect influences
  for (i = 0; i < length; i++)
    {
      InfluenceInfo *info = &g_array_index (influences, InfluenceInfo, i);
      info->index = i;
      info->value = g_array_index (object_influences, float, i);
    }

  g_array_sort (influences, (GCompareFunc)influence_info_cmp);

  // Add morphAttributes
  for (i = 0; i < 8; i++)
    {
      if (i < length)
        {
          InfluenceInfo *info = &g_array_index (influences, InfluenceInfo, i);

          if (info->value != 0)
            {
              if (morphTargets)
                {
                  g_autofree char *name = g_strdup_printf ("morphTarget%d", i);
                  GthreeAttribute *attribute = g_ptr_array_index (morphTargets, info->index);
                  gthree_geometry_add_attribute (geometry, name, attribute);
                }

              if (morphNormals)
                {
                  g_autofree char *name = g_strdup_printf ("morphNormal%d", i);
                  GthreeAttribute *attribute = g_ptr_array_index (morphTargets, info->index);
                  gthree_geometry_add_attribute (geometry, name, attribute);
                }

              priv->morph_influences[i] = info->value;
              continue;
            }

        }

      priv->morph_influences[i] = 0;
    }

  gint morph_target_influences_location =
    gthree_program_lookup_uniform_location_from_string (program, "morphTargetInfluences[0]");

  if (morph_target_influences_location >= 0)
    glUniform1fv (morph_target_influences_location, 8, priv->morph_influences);
  else
    g_warning ("No morphTargetInfluences uniform");
}

static void
render_item (GthreeRenderer *renderer,
             GthreeCamera *camera,
             gpointer fog,
             GthreeMaterial *material,
             GthreeRenderListItem *item)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeGeometry *geometry = item->geometry;
  GthreeGeometryGroup *group = item->group;
  GthreeObject *object = item->object;
  GthreeProgram *program;
  GthreeAttribute *position, *index;
  gboolean update_buffers = FALSE;
  gboolean wireframe = FALSE;
  int data_count;
  int range_factor, range_start, range_count, group_start, group_count, draw_start, draw_end, draw_count;
  int draw_mode = GL_TRIANGLES;

  if (!gthree_material_get_is_visible (material))
    return;

  if (GTHREE_IS_MESH_MATERIAL (material) &&
      gthree_mesh_material_get_is_wireframe (GTHREE_MESH_MATERIAL (material)))
    wireframe = TRUE;

  program = set_program (renderer, camera, fog, material, object);

  if (geometry != priv->current_geometry_program_geometry ||
      program != priv->current_geometry_program_program ||
      wireframe != priv->current_geometry_program_wireframe)
    {
      priv->current_geometry_program_geometry = geometry;
      priv->current_geometry_program_program = program;
      priv->current_geometry_program_wireframe = wireframe;
      update_buffers = true;
    }

  if (GTHREE_IS_MESH (object) &&
      gthree_mesh_has_morph_targets (GTHREE_MESH (object)) &&
      GTHREE_IS_MESH_MATERIAL (material))
    {
      update_morphtargets (renderer, GTHREE_MESH (object), geometry, GTHREE_MESH_MATERIAL (material), program);
      update_buffers = TRUE;
    }

  index = gthree_geometry_get_index (geometry);
  position = gthree_geometry_get_position (geometry);
  range_factor = 1;

  if (wireframe)
    {
      index = gthree_geometry_get_wireframe_index (geometry);
      gthree_attribute_update (index, renderer, GL_ELEMENT_ARRAY_BUFFER);
      range_factor = 2;
    }

  if (update_buffers)
    {
      setup_vertex_attributes (renderer, material, program, geometry);
      if (index != NULL)
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, gthree_attribute_get_gl_buffer (index));
    }

  data_count = -1;

  if (index != NULL)
    data_count = gthree_attribute_get_count (index);
  else if (position != NULL)
    data_count = gthree_attribute_get_count (position);

  range_start = gthree_geometry_get_draw_range_start (geometry) * range_factor;
  range_count = gthree_geometry_get_draw_range_count (geometry) * range_factor;

  group_start = group != NULL ? group->start * range_factor : 0;
  group_count = group != NULL ? group->count * range_factor : -1;

  /* Handle unlimited ranges (-1 * maybe range_factor) */
  if (group_count < 0)
    group_count = data_count;
  if (range_count < 0)
    range_count = data_count;

  draw_start = MAX (range_start, group_start);
  draw_end = MIN (data_count, MIN (range_start + range_count, group_start + group_count)) - 1;
  draw_count = MAX( 0, draw_end - draw_start + 1);

  if ( draw_count == 0 )
    return;

  // render mesh
  if (GTHREE_IS_MESH (object))
    {
      if (wireframe)
        {
          // wireframe
          set_line_width (renderer, gthree_mesh_material_get_wireframe_line_width (GTHREE_MESH_MATERIAL (material)));
          draw_mode = GL_LINES;
        }
      else
        {
          switch (gthree_mesh_get_draw_mode (GTHREE_MESH (object)))
            {
            default:
            case GTHREE_DRAW_MODE_TRIANGLES:
              draw_mode = GL_TRIANGLES;
              break;
            case GTHREE_DRAW_MODE_TRIANGLE_STRIP:
              draw_mode = GL_TRIANGLE_STRIP;
              break;
            case GTHREE_DRAW_MODE_TRIANGLE_FAN:
              draw_mode = GL_TRIANGLE_FAN;
              break;
            }
        }
    }
  else if (GTHREE_IS_LINE_SEGMENTS (object))
    {
      float width = 1.0;
      if (GTHREE_IS_LINE_BASIC_MATERIAL (material))
        width = gthree_line_basic_material_get_line_width (GTHREE_LINE_BASIC_MATERIAL (material));
      set_line_width (renderer, width);
      draw_mode = GL_LINES;
    }
  else if (GTHREE_IS_SPRITE (object))
    {
      draw_mode = GL_TRIANGLES;
    }
  else if (GTHREE_IS_POINTS (object))
    {
      draw_mode = GL_POINTS;
    }

  if (index)
    {
      int index_type = gthree_attribute_get_gl_type (index);
      int index_bytes_per_element = gthree_attribute_get_gl_bytes_per_element (index);
      int index_offset = gthree_attribute_get_item_offset (index);

      glDrawElements (draw_mode, draw_count, index_type, GINT_TO_POINTER ((index_offset + draw_start) * index_bytes_per_element));
    }
  else
    {
      glDrawArrays (draw_mode, draw_start, draw_count);
    }
}

static void
render_objects (GthreeRenderer *renderer,
                GthreeScene    *scene,
                GArray *render_list_indexes,
                GthreeCamera *camera,
                gpointer fog,
                gboolean use_blending,
                GthreeMaterial *override_material)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeMaterial *material;
  int i;

  for (i = 0; i < render_list_indexes->len; i++)
    {
      int render_list_index = g_array_index (render_list_indexes, int, i);
      GthreeRenderListItem *item = &g_array_index (priv->current_render_list->items, GthreeRenderListItem, render_list_index);

      gthree_object_call_before_render_callback (item->object, scene, camera);

      gthree_object_update_matrix_view (item->object, gthree_camera_get_world_inverse_matrix (camera));

      if (override_material)
        material = override_material;
      else
        material = item->material;

      if (material == NULL)
        continue;

      if (use_blending)
        {
          guint equation, src_factor, dst_factor;
          GthreeBlendMode mode = gthree_material_get_blend_mode (material, &equation, &src_factor, &dst_factor);

          set_blending (renderer, mode, equation, src_factor, dst_factor);
        }

      set_depth_test (renderer, gthree_material_get_depth_test (material));
      set_depth_write (renderer, gthree_material_get_depth_write (material));

      {
        gboolean polygon_offset;
        float factor, units;

        polygon_offset = gthree_material_get_polygon_offset (material, &factor, &units);
        set_polygon_offset (renderer, polygon_offset, factor, units);
      }
      set_material_faces (renderer, material);

      render_item (renderer, camera, fog, material, item);
    }
}

static void
clear (gboolean color, gboolean depth, gboolean stencil)
{
  guint bits = 0;
  if (color)
    bits |= GL_COLOR_BUFFER_BIT;
  if (depth)
    bits |= GL_DEPTH_BUFFER_BIT;
  if (stencil)
    bits |= GL_STENCIL_BUFFER_BIT;

  glClear (bits);
}

static void
before_render_bg_cube (GthreeObject                *object,
                       GthreeScene                 *scene,
                       GthreeCamera                *camera)
{
  const graphene_matrix_t *camera_world_matrix;
  graphene_matrix_t m;
  graphene_point3d_t camera_offset;

  camera_world_matrix = gthree_object_get_world_matrix (GTHREE_OBJECT (camera));

  camera_offset.x = graphene_matrix_get_x_translation (camera_world_matrix);
  camera_offset.y = graphene_matrix_get_y_translation (camera_world_matrix);
  camera_offset.z = graphene_matrix_get_z_translation (camera_world_matrix);

  graphene_matrix_init_translate (&m, &camera_offset);

  gthree_object_set_world_matrix (object, &m);
}

static void
gthree_renderer_render_background (GthreeRenderer *renderer,
                                   GthreeScene    *scene)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  const graphene_vec3_t *bg_color = gthree_scene_get_background_color (scene);
  GthreeTexture *bg_texture = gthree_scene_get_background_texture (scene);
  gboolean force_clear = FALSE;
  GthreeMesh *bg_mesh = NULL;
  const graphene_vec3_t *clear_color = NULL;

  if (bg_color == NULL)
    clear_color = &priv->clear_color;
  else
    {
      clear_color = bg_color;
      force_clear = TRUE;
    }

  if (priv->auto_clear || force_clear)
    {
      set_clear_color (renderer, clear_color, 1);
      clear (priv->auto_clear_color, priv->auto_clear_depth, priv->auto_clear_stencil);
    }

  if (bg_texture && GTHREE_IS_CUBE_TEXTURE (bg_texture))
    {
      if (priv->bg_box_mesh == NULL)
        {
          GthreeGeometry *geometry;
          GthreeShaderMaterial *shader_material;
          GthreeShader *shader;
          GthreeMesh *box_mesh;

          shader = gthree_clone_shader_from_library ("cube");
          g_assert (shader != NULL);

          shader_material = gthree_shader_material_new (shader);
          gthree_material_set_depth_test (GTHREE_MATERIAL (shader_material), FALSE);
          gthree_material_set_depth_write (GTHREE_MATERIAL (shader_material), FALSE);
          gthree_material_set_side (GTHREE_MATERIAL (shader_material), GTHREE_SIDE_BACK);

          geometry = gthree_geometry_new_box (10, 10, 10, 1, 1, 1);

          box_mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (shader_material));

          gthree_object_set_matrix_auto_update (GTHREE_OBJECT (box_mesh), FALSE);
          gthree_object_set_before_render_callback (GTHREE_OBJECT (box_mesh), before_render_bg_cube);

          priv->bg_box_mesh = box_mesh;
        }

      if (priv->current_bg_texture != bg_texture)
        {
          GthreeMaterial *material = gthree_mesh_get_material (priv->bg_box_mesh, 0);
          GthreeShader *shader = gthree_material_get_shader (material);
          GthreeUniforms *uniforms = gthree_shader_get_uniforms (shader);
          GthreeUniform *uni = gthree_uniforms_lookup_from_string (uniforms, "tCube");
          g_assert (uni != NULL);

          gthree_uniform_set_texture (uni, bg_texture);
          g_clear_object (&priv->current_bg_texture);
          priv->current_bg_texture = g_object_ref (bg_texture);

          gthree_material_set_needs_update (material, TRUE);
        }

      bg_mesh = priv->bg_box_mesh;
    }
  else if (bg_texture && GTHREE_IS_TEXTURE (bg_texture))
    {
      if (priv->bg_plane_mesh == NULL)
        {
          GthreeGeometry *geometry;
          GthreeShaderMaterial *shader_material;
          GthreeShader *shader;
          GthreeMesh *plane_mesh;

          shader = gthree_clone_shader_from_library ("background");
          g_assert (shader != NULL);

          shader_material = gthree_shader_material_new (shader);
          gthree_material_set_depth_test (GTHREE_MATERIAL (shader_material), FALSE);
          gthree_material_set_depth_write (GTHREE_MATERIAL (shader_material), FALSE);
          gthree_material_set_side (GTHREE_MATERIAL (shader_material), GTHREE_SIDE_FRONT);

          geometry = NULL;
          geometry = gthree_geometry_new_plane (2, 2, 1, 1);

          plane_mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (shader_material));

          priv->bg_plane_mesh = plane_mesh;
        }

      if (priv->current_bg_texture != bg_texture)
        {
          GthreeMaterial *material = gthree_mesh_get_material (priv->bg_plane_mesh, 0);
          GthreeShader *shader = gthree_material_get_shader (material);
          GthreeUniforms *uniforms = gthree_shader_get_uniforms (shader);
          GthreeUniform *uni = gthree_uniforms_lookup_from_string (uniforms, "t2D");
          g_assert (uni != NULL);

          gthree_uniform_set_texture (uni, bg_texture);
          g_clear_object (&priv->current_bg_texture);
          priv->current_bg_texture = g_object_ref (bg_texture);

          /* TODO: Handle uvTransform for texture */

          gthree_material_set_needs_update (material, TRUE);
        }

      bg_mesh = priv->bg_plane_mesh;
    }

  if (bg_mesh != NULL)
    {
      gthree_object_update (GTHREE_OBJECT (bg_mesh), renderer);

      priv->current_render_list->use_background = TRUE;
      priv->current_render_list->current_z = 0;
      gthree_object_fill_render_list (GTHREE_OBJECT (bg_mesh), priv->current_render_list);
      priv->current_render_list->use_background = FALSE;
    }
}

void
gthree_renderer_render (GthreeRenderer *renderer,
                        GthreeScene    *scene,
                        GthreeCamera   *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeMaterial *override_material;
  gpointer fog;

  gthree_renderer_push_current (renderer);

  push_debug_group ("gthree render to %p", priv->current_render_target);

  g_list_free (priv->lights);
  priv->lights = NULL;

  g_list_free (priv->shadows);
  priv->shadows = NULL;

  fog = NULL;

  priv->current_material = NULL;
  priv->current_camera = NULL;
  priv->current_geometry_program_geometry = NULL;
  priv->current_geometry_program_program = NULL;
  priv->current_geometry_program_wireframe = FALSE;

  /* update scene graph */

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), FALSE);

  /* update camera matrices and frustum */

  if (gthree_object_get_parent (GTHREE_OBJECT (camera)) == NULL)
    gthree_object_update_matrix_world (GTHREE_OBJECT (camera), FALSE);

  gthree_camera_update_matrix (camera);

  gthree_camera_get_proj_screen_matrix (camera, &priv->proj_screen_matrix);
  graphene_frustum_init_from_matrix (&priv->frustum, &priv->proj_screen_matrix);

  priv->clipping_enabled = clipping_init (renderer, camera);

  /* Flush lazily deleted resources to avoid leaking until widget unrealize */
  gthree_resources_flush_deletes (renderer);

  gthree_render_list_init (priv->current_render_list);

  project_object (renderer, scene, GTHREE_OBJECT (scene), camera);

  if (priv->sort_objects)
    gthree_render_list_sort (priv->current_render_list);

  if (priv->clipping_enabled )
    clipping_begin_shadows (renderer);

  render_shadow_map (renderer, scene, camera);

  setup_lights (renderer, camera);

  if (priv->clipping_enabled)
    clipping_end_shadows (renderer);

  gthree_renderer_set_render_target (renderer, priv->current_render_target, 0, 0);

  gthree_renderer_render_background (renderer, scene);

  /* set matrices for regular objects (frustum culled) */

  override_material = gthree_scene_get_override_material (scene);
  if (override_material)
    {
      gboolean polygon_offset;
      float factor, units;
      guint equation, src_factor, dst_factor;
      GthreeBlendMode mode = gthree_material_get_blend_mode (override_material, &equation, &src_factor, &dst_factor);

      set_blending (renderer, mode, equation, src_factor, dst_factor);

      set_depth_test (renderer, gthree_material_get_depth_test (override_material));
      set_depth_write (renderer, gthree_material_get_depth_write (override_material));
      polygon_offset = gthree_material_get_polygon_offset (override_material, &factor, &units);
      set_polygon_offset (renderer, polygon_offset, factor, units);

      render_objects (renderer, scene, priv->current_render_list->background, camera, fog, TRUE, override_material );
      render_objects (renderer, scene, priv->current_render_list->opaque, camera, fog, TRUE, override_material );
      render_objects (renderer, scene, priv->current_render_list->transparent, camera, fog, TRUE, override_material );
    }
  else
    {
      set_blending (renderer, GTHREE_BLEND_NO, 0, 0, 0);

      render_objects (renderer, scene, priv->current_render_list->background, camera, fog, FALSE, NULL);

      // opaque pass (front-to-back order)
      render_objects (renderer, scene, priv->current_render_list->opaque, camera, fog, FALSE, NULL);

      // transparent pass (back-to-front order)
      render_objects (renderer, scene, priv->current_render_list->transparent, camera, fog, TRUE, NULL);
    }

  if (priv->current_render_target != NULL)
    {
      // Generate mipmap if we're using any kind of mipmap filtering
      gthree_render_target_update_mipmap (priv->current_render_target, renderer);
      // resolve multisample renderbuffers to a single-sample texture if necessary
      update_multisample_render_target (renderer, priv->current_render_target);
    }

  pop_debug_group ();

  gthree_renderer_pop_current (renderer);
}

guint
gthree_renderer_allocate_texture_unit (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  guint texture_unit = priv->used_texture_units;

  if (texture_unit >= priv->max_textures )
    g_warning ("Trying to use %dtexture units while this GPU supports only %d",  texture_unit,priv->max_textures);

  priv->used_texture_units += 1;

  return texture_unit;
}


GthreeRenderList *
gthree_render_list_new ()
{
  GthreeRenderList *list = g_new0 (GthreeRenderList, 1);

  list->items = g_array_new (FALSE, FALSE, sizeof (GthreeRenderListItem));
  list->opaque = g_array_new (FALSE, FALSE, sizeof (int));
  list->transparent = g_array_new (FALSE, FALSE, sizeof (int));
  list->background = g_array_new (FALSE, FALSE, sizeof (int));

  return list;
}

void
gthree_render_list_free (GthreeRenderList *list)
{
  g_array_unref (list->items);
  g_array_unref (list->opaque);
  g_array_unref (list->transparent);
  g_array_unref (list->background);
  g_free (list);
}

void
gthree_render_list_init (GthreeRenderList *list)
{
  list->current_z = 0;
  list->use_background = FALSE;
  g_array_set_size (list->items, 0);
  g_array_set_size (list->opaque, 0);
  g_array_set_size (list->transparent, 0);
  g_array_set_size (list->background, 0);
}

static gint
render_list_painter_sort_stable (gconstpointer _a, gconstpointer _b, gpointer user_data)
{
  GthreeRenderList *list = user_data;
  int ai = *(int *)_a;
  int bi = *(int *)_b;
  GthreeRenderListItem *a = &g_array_index (list->items, GthreeRenderListItem, ai);
  GthreeRenderListItem *b = &g_array_index (list->items, GthreeRenderListItem, bi);

  if (a->z != b->z)
    {
      if (a->z > b->z)
        return 1;
      else
        return -1;
    }
  else if (a->object != b->object)
    {
      if ((gsize)a->object > (gsize)b->object)
        return 1;
      else
        return -1;
    }

  return 0;
}

static gint
render_list_reverse_painter_sort_stable (gconstpointer _a, gconstpointer _b, gpointer user_data)
{
  GthreeRenderList *list = user_data;
  int ai = *(int *)_a;
  int bi = *(int *)_b;
  GthreeRenderListItem *a = &g_array_index (list->items, GthreeRenderListItem, ai);
  GthreeRenderListItem *b = &g_array_index (list->items, GthreeRenderListItem, bi);

  if (a->z != b->z)
    {
      if (b->z > a->z)
        return 1;
      else
        return -1;
    }
  else if (a->object != b->object)
    {
      if ((gsize)a->object > (gsize)b->object)
        return 1;
      else
        return -1;
    }

  return 0;
}

void
gthree_render_list_sort (GthreeRenderList *list)
{
  g_array_sort_with_data (list->opaque, render_list_painter_sort_stable, list);
  g_array_sort_with_data (list->transparent, render_list_reverse_painter_sort_stable, list);
}

void
gthree_render_list_push (GthreeRenderList *list,
                         GthreeObject *object,
                         GthreeGeometry *geometry,
                         GthreeMaterial *material,
                         GthreeGeometryGroup *group)
{
  GthreeRenderListItem item = { object, geometry, material, group, list->current_z };
  int index = list->items->len;

  g_array_append_val (list->items, item);

  if (list->use_background)
    g_array_append_val (list->background, index);
  else if (gthree_material_get_is_transparent (material))
    g_array_append_val (list->transparent, index);
  else
    g_array_append_val (list->opaque, index);
}
