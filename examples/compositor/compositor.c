/*
 * Gthree 3D Wayland Compositor Example
 *
 * Renders Wayland client windows as paper sheets on a 3D desk scene.
 * Wayland compositor logic inspired by Casilda (by Juan Pablo Ugarte).
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#define _POSIX_C_SOURCE 200809L
#ifndef WLR_USE_UNSTABLE
#define WLR_USE_UNSTABLE
#endif

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <gthree/gthree.h>
#include <gthree/gthreearea.h>

#include <drm_fourcc.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/interface.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_xdg_activation_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_cursor_shape_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_shm.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/render/dmabuf.h>
#include <xkbcommon/xkbcommon.h>
#include <linux/input-event-codes.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/wayland/gdkwayland.h>
#endif
#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#ifdef HAVE_X11_XCB
#include <X11/Xlib-xcb.h>
#include <xkbcommon/xkbcommon-x11.h>
#endif
#endif

#include "wayland-source.h"

#define DEFAULT_WIDTH 1400.0f
#define DEFAULT_HEIGHT 1800.0f

#define PAPER_SCALE 0.4f
#define FLOOR_SCALE 20.0f
#define DESK_SCALE 600.0f
#define FALL_SPEED 400.0f
#define FALL_START_Y 1200.0f
#define SETTLE_DURATION 0.3f
#define GRAB_LIFT_HEIGHT 100.0f
#define BONE_COUNT 16

#define CAM_DISTANCE_MIN 30.0f
#define CAM_DISTANCE_MAX 20000.0f
#define CAM_PHI_MIN 0.1f
#define CAM_PHI_MAX (G_PI * 0.48f)
#define CAM_ORBIT_SPEED 0.005f
#define CAM_PAN_SPEED 1.0f
#define CAM_ZOOM_SPEED 80.0f

#define HEIGHTMAP_SIZE 512

/* SHM formats we support */
static const uint32_t shm_formats[] = {
  DRM_FORMAT_ARGB8888,
  DRM_FORMAT_XRGB8888,
  DRM_FORMAT_ABGR8888,
  DRM_FORMAT_XBGR8888,
  DRM_FORMAT_RGB888,
  DRM_FORMAT_BGR888,
};

typedef enum { PAPER_FALLING, PAPER_SETTLING, PAPER_RESTING, PAPER_GRABBED } PaperState;

typedef struct CompositorToplevel CompositorToplevel;

typedef struct
{
  GtkWidget *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;

  /* Wayland */
  struct wl_display *wl_display;
  GSource *wl_source;
  char *socket;

  /* wlroots objects */
  struct wlr_backend backend;
  struct wlr_backend_impl backend_impl;
  struct wlr_output output;
  struct wlr_output_impl output_impl;
  struct wl_listener output_bind;

  struct wlr_keyboard keyboard;
  struct wlr_pointer pointer;
  struct wlr_seat *seat;

  /* Protocol listeners */
  struct wl_listener new_xdg_toplevel;
  struct wl_listener new_xdg_popup;
  struct wl_listener request_activate;
  struct wl_listener request_set_shape;
  struct wl_listener request_set_selection;

  /* Toplevels */
  GList *toplevels;

  /* Desk scene */
  GthreeObject *desk_model;
  GthreeObject *floor_model;
  GthreeLoader *desk_loader;
  GthreeLoader *floor_loader;
  float desk_x_min, desk_x_max, desk_z_min, desk_z_max;

  /* Soldier */
  GthreeObject *soldier_model;
  GthreeLoader *soldier_loader;
  GthreeAnimationMixer *soldier_mixer;

  /* Heightmap */
  guchar *heightmap_data;
  float hm_x_min, hm_x_max, hm_z_min, hm_z_max;
  float hm_y_min, hm_y_max;
  float hm_cam_y, hm_near, hm_far;

  /* Camera */
  float cam_theta, cam_phi, cam_distance;
  graphene_vec3_t cam_target;

  /* Input state */
  double cursor_x, cursor_y;
  CompositorToplevel *focused_toplevel;

  gboolean meta_held;
  gboolean camera_dragging;
  int camera_drag_button;
  double drag_last_x, drag_last_y;

  CompositorToplevel *grabbed_toplevel;
  float grab_rest_y;
  float grab_offset_x, grab_offset_z;

  gint64 last_frame_time;

  gboolean no_dmabuf;
  gboolean debug;
} CompositorState;

struct CompositorToplevel
{
  CompositorState *state;
  struct wlr_xdg_toplevel *xdg_toplevel;

  GthreeGroup *group;
  GthreeMesh *mesh;
  GthreeTexture *texture;
  GthreeMeshBasicMaterial *material;

  float quad_width;
  float quad_height;
  float pixel_width;
  float pixel_height;

  /* Paper physics */
  PaperState paper_state;
  float fall_time;
  float fall_start_y;
  float settle_time;
  float rest_x, rest_z, rest_rotation_y;
  float bend_phase;

  /* Skinning */
  GthreeSkeleton *skeleton;
  GthreeBone *bones[BONE_COUNT];
  GthreeBone *root_bone;

  GList *popups;
  GList *subsurfaces;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
  struct wl_listener new_subsurface;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;
  struct wl_listener request_fullscreen;
};

typedef struct
{
  CompositorState *state;
  struct wlr_xdg_popup *xdg_popup;
  CompositorToplevel *toplevel;

  GthreeMesh *mesh;
  GthreeTexture *texture;
  GthreeMeshBasicMaterial *material;
  float quad_width;
  float quad_height;

  GList *subsurfaces;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
  struct wl_listener new_subsurface;
} CompositorPopup;

typedef struct
{
  CompositorState *state;
  struct wlr_subsurface *wlr_subsurface;
  CompositorToplevel *toplevel;
  CompositorPopup *popup;

  GthreeMesh *mesh;
  GthreeTexture *texture;
  GthreeMeshBasicMaterial *material;
  float quad_width;
  float quad_height;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
} CompositorSubsurface;

static CompositorState global_state;

static void on_new_subsurface (struct wl_listener *listener, void *data);
static void on_popup_new_subsurface (struct wl_listener *listener, void *data);
static void on_subsurface_map (struct wl_listener *listener, void *data);
static void on_subsurface_unmap (struct wl_listener *listener, void *data);
static void on_subsurface_commit (struct wl_listener *listener, void *data);
static void on_subsurface_destroy (struct wl_listener *listener, void *data);
static gboolean import_buffer_to_texture (CompositorState *state, struct wlr_buffer *buffer, GthreeTexture *texture);
static gboolean ray_plane_intersect (CompositorState *state, float plane_y, float *out_x, float *out_z);
static void render_heightmap (CompositorState *state);
static GthreeMesh *create_subsurface_mesh (CompositorSubsurface *sub, GthreeGeometry *geom);
static void position_subsurface_mesh (CompositorSubsurface *sub);

/* --- DRM format to GthreeMemoryFormat mapping --- */

static GthreeMemoryFormat
drm_to_gthree_format (uint32_t fourcc)
{
  switch (fourcc)
    {
    case DRM_FORMAT_ARGB8888:
      return GTHREE_MEMORY_FORMAT_B8G8R8A8_PREMULTIPLIED;
    case DRM_FORMAT_XRGB8888:
      return GTHREE_MEMORY_FORMAT_B8G8R8A8;
    case DRM_FORMAT_ABGR8888:
      return GTHREE_MEMORY_FORMAT_R8G8B8A8_PREMULTIPLIED;
    case DRM_FORMAT_XBGR8888:
      return GTHREE_MEMORY_FORMAT_R8G8B8A8;
    case DRM_FORMAT_RGB888:
      return GTHREE_MEMORY_FORMAT_R8G8B8;
    case DRM_FORMAT_BGR888:
      return GTHREE_MEMORY_FORMAT_B8G8R8A8; /* approximate */
    default:
      return GTHREE_MEMORY_FORMAT_R8G8B8A8;
    }
}

/* --- Camera --- */

static void
update_camera_from_spherical (CompositorState *state)
{
  float tx = graphene_vec3_get_x (&state->cam_target);
  float ty = graphene_vec3_get_y (&state->cam_target);
  float tz = graphene_vec3_get_z (&state->cam_target);

  float cx = tx + state->cam_distance * sinf (state->cam_phi) * sinf (state->cam_theta);
  float cy = ty + state->cam_distance * cosf (state->cam_phi);
  float cz = tz + state->cam_distance * sinf (state->cam_phi) * cosf (state->cam_theta);

  gthree_object_set_position_xyz (GTHREE_OBJECT (state->camera), cx, cy, cz);
  gthree_object_look_at_xyz (GTHREE_OBJECT (state->camera), tx, ty, tz);
}

/* --- Heightmap --- */

static float
get_heightmap_pixel (CompositorState *state, int px, int py)
{
  int idx = (py * HEIGHTMAP_SIZE + px) * 4;
  float r = state->heightmap_data[idx]     / 255.0f;
  float g = state->heightmap_data[idx + 1] / 255.0f;
  float b = state->heightmap_data[idx + 2] / 255.0f;
  float a = state->heightmap_data[idx + 3] / 255.0f;
  const float unpack_downscale = 255.0f / 256.0f;
  float depth_val = r * unpack_downscale
                  + g * (unpack_downscale / 256.0f)
                  + b * (unpack_downscale / 65536.0f)
                  + a * (1.0f / 16777216.0f);
  float dist = state->hm_near + depth_val * (state->hm_far - state->hm_near);
  return state->hm_cam_y - dist;
}

static float
get_max_height_under_rect (CompositorState *state,
                           float center_x, float center_z,
                           float half_w, float half_h,
                           float rotation_deg)
{
  if (!state->heightmap_data)
    return 0;

  float rot = rotation_deg * (G_PI / 180.0f);
  float cs = cosf (rot), sn = sinf (rot);

  /* Compute the four corners in world space */
  float lx[4] = {-half_w,  half_w, half_w, -half_w};
  float lz[4] = {-half_h, -half_h, half_h,  half_h};
  float wx[4], wz[4];
  float min_wx = G_MAXFLOAT, max_wx = -G_MAXFLOAT;
  float min_wz = G_MAXFLOAT, max_wz = -G_MAXFLOAT;
  for (int i = 0; i < 4; i++)
    {
      wx[i] = center_x + lx[i] * cs - lz[i] * sn;
      wz[i] = center_z + lx[i] * sn + lz[i] * cs;
      if (wx[i] < min_wx) min_wx = wx[i];
      if (wx[i] > max_wx) max_wx = wx[i];
      if (wz[i] < min_wz) min_wz = wz[i];
      if (wz[i] > max_wz) max_wz = wz[i];
    }

  /* Convert world AABB to heightmap pixel range */
  float inv_w = 1.0f / (state->hm_x_max - state->hm_x_min);
  float inv_h = 1.0f / (state->hm_z_max - state->hm_z_min);
  int px0 = (int)((min_wx - state->hm_x_min) * inv_w * (HEIGHTMAP_SIZE - 1));
  int px1 = (int)((max_wx - state->hm_x_min) * inv_w * (HEIGHTMAP_SIZE - 1));
  int py0 = (int)((min_wz - state->hm_z_min) * inv_h * (HEIGHTMAP_SIZE - 1));
  int py1 = (int)((max_wz - state->hm_z_min) * inv_h * (HEIGHTMAP_SIZE - 1));
  px0 = CLAMP (px0, 0, HEIGHTMAP_SIZE - 1);
  px1 = CLAMP (px1, 0, HEIGHTMAP_SIZE - 1);
  py0 = CLAMP (py0, 0, HEIGHTMAP_SIZE - 1);
  py1 = CLAMP (py1, 0, HEIGHTMAP_SIZE - 1);

  /* Inverse rotation to transform world points back to local space */
  float max_h = 0;
  for (int py = py0; py <= py1; py++)
    for (int px = px0; px <= px1; px++)
      {
        float world_x = state->hm_x_min + (float)px / (HEIGHTMAP_SIZE - 1) * (state->hm_x_max - state->hm_x_min);
        float world_z = state->hm_z_min + (float)py / (HEIGHTMAP_SIZE - 1) * (state->hm_z_max - state->hm_z_min);

        /* Rotate into paper-local space to test containment */
        float dx = world_x - center_x;
        float dz = world_z - center_z;
        float local_x = dx * cs + dz * sn;
        float local_z = -dx * sn + dz * cs;

        if (local_x >= -half_w && local_x <= half_w &&
            local_z >= -half_h && local_z <= half_h)
          {
            float h = get_heightmap_pixel (state, px, py);
            if (h > max_h) max_h = h;
          }
      }

  return max_h;
}

static gboolean
oriented_rects_overlap (float ax, float az, float ahw, float ahh, float arot_deg,
                        float bx, float bz, float bhw, float bhh, float brot_deg)
{
  float ar = arot_deg * (G_PI / 180.0f);
  float br = brot_deg * (G_PI / 180.0f);
  float acs = cosf (ar), asn = sinf (ar);
  float bcs = cosf (br), bsn = sinf (br);

  /* Corners of both rects in world space */
  float lx[4] = {-1, 1, 1, -1};
  float lz[4] = {-1, -1, 1, 1};
  float ca[4][2], cb[4][2];
  for (int i = 0; i < 4; i++)
    {
      ca[i][0] = ax + lx[i] * ahw * acs - lz[i] * ahh * asn;
      ca[i][1] = az + lx[i] * ahw * asn + lz[i] * ahh * acs;
      cb[i][0] = bx + lx[i] * bhw * bcs - lz[i] * bhh * bsn;
      cb[i][1] = bz + lx[i] * bhw * bsn + lz[i] * bhh * bcs;
    }

  /* SAT with 4 edge normals (2 per rect) */
  float axes[4][2] = {
    { acs, asn }, { -asn, acs },
    { bcs, bsn }, { -bsn, bcs },
  };

  for (int a = 0; a < 4; a++)
    {
      float nx = axes[a][0], nz = axes[a][1];
      float a_min = G_MAXFLOAT, a_max = -G_MAXFLOAT;
      float b_min = G_MAXFLOAT, b_max = -G_MAXFLOAT;
      for (int i = 0; i < 4; i++)
        {
          float pa = ca[i][0] * nx + ca[i][1] * nz;
          float pb = cb[i][0] * nx + cb[i][1] * nz;
          if (pa < a_min) a_min = pa;
          if (pa > a_max) a_max = pa;
          if (pb < b_min) b_min = pb;
          if (pb > b_max) b_max = pb;
        }
      if (a_max < b_min || b_max < a_min)
        return FALSE;
    }

  return TRUE;
}

static float
get_target_height_for_rect (CompositorState *state,
                            CompositorToplevel *exclude,
                            float cx, float cz,
                            float paper_y,
                            float hw, float hh,
                            float rotation_y)
{
  float max_h = get_max_height_under_rect (state, cx, cz, hw, hh, rotation_y);

  for (GList *l = state->toplevels; l; l = l->next)
    {
      CompositorToplevel *other = l->data;
      if (other == exclude || !other->group)
        continue;
      if (other->paper_state != PAPER_RESTING &&
          other->paper_state != PAPER_SETTLING)
        continue;

      const graphene_vec3_t *pos =
        gthree_object_get_position (GTHREE_OBJECT (other->group));
      float other_y = graphene_vec3_get_y (pos);
      if (other_y >= paper_y)
        continue;

      if (oriented_rects_overlap (cx, cz, hw, hh, rotation_y,
                                  other->rest_x, other->rest_z,
                                  other->quad_width * 0.5f,
                                  other->quad_height * 0.5f,
                                  other->rest_rotation_y))
        {
          float h = other_y + 1.0f;
          if (h > max_h)
            max_h = h;
        }
    }

  return max_h + 1.0f;
}

static void
assign_rest_position (CompositorState *state, CompositorToplevel *ct)
{
  float margin = 20.0f;
  float x_min = state->desk_x_min + margin;
  float x_max = state->desk_x_max - margin;
  float z_min = state->desk_z_min + margin;
  float z_max = state->desk_z_max - margin;

  ct->rest_x = g_random_double_range (x_min, x_max);
  ct->rest_z = g_random_double_range (z_min, z_max);
  ct->rest_rotation_y = g_random_double_range (-15.0f, 15.0f);
  ct->bend_phase = g_random_double_range (0, 2 * G_PI);
}

/* --- Focus management --- */

static void
focus_toplevel (CompositorState *state, CompositorToplevel *toplevel)
{
  if (state->focused_toplevel == toplevel)
    return;

  if (state->focused_toplevel)
    wlr_xdg_toplevel_set_activated (state->focused_toplevel->xdg_toplevel, false);

  state->focused_toplevel = toplevel;

  if (toplevel)
    {
      wlr_xdg_toplevel_set_activated (toplevel->xdg_toplevel, true);
      wlr_seat_keyboard_notify_enter (state->seat,
                                      toplevel->xdg_toplevel->base->surface,
                                      state->keyboard.keycodes,
                                      state->keyboard.num_keycodes,
                                      &state->keyboard.modifiers);
    }
}

static void
flip_plane_uvs_y (GthreeGeometry *geom)
{
  GthreeAttribute *uv = gthree_geometry_get_attribute (geom, "uv");
  int count = gthree_attribute_get_count (uv);
  for (int i = 0; i < count; i++)
    {
      graphene_vec2_t v;
      gthree_attribute_get_vec2 (uv, i, &v);
      gthree_attribute_set_y (uv, i, 1.0f - graphene_vec2_get_y (&v));
    }
}

static void
set_uniform_scale (GthreeObject *object, float s)
{
  graphene_vec3_t scale;
  graphene_vec3_init (&scale, s, s, s);
  gthree_object_set_scale (object, &scale);
}

static void
create_surface_material (GthreeMeshBasicMaterial **out_material,
                         GthreeTexture           **out_texture,
                         gboolean                  transparent)
{
  GthreeMeshBasicMaterial *material = gthree_mesh_basic_material_new ();
  gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);
  if (transparent)
    gthree_material_set_is_transparent (GTHREE_MATERIAL (material), TRUE);

  GthreeTexture *texture =
    gthree_texture_new_from_bytes (g_bytes_new (NULL, 0), 1, 1, 4,
                                   GTHREE_MEMORY_FORMAT_R8G8B8A8, 0);
  gthree_mesh_basic_material_set_map (material, texture);

  *out_material = material;
  *out_texture = texture;
}

/* --- Wayland protocol handlers --- */

static void
surface_notify_scale (struct wlr_surface *surface, float scale)
{
  wlr_fractional_scale_v1_notify_scale (surface, scale);
  wlr_surface_set_preferred_buffer_scale (surface, (int32_t)ceilf (scale));
}

static gboolean
buffer_has_alpha (struct wlr_buffer *buffer)
{
  uint32_t fmt = 0;
  struct wlr_dmabuf_attributes dmabuf;

  if (wlr_buffer_get_dmabuf (buffer, &dmabuf))
    fmt = dmabuf.format;
  else
    {
      void *tmp;
      size_t stride;
      if (wlr_buffer_begin_data_ptr_access (buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ,
                                             &tmp, &fmt, &stride))
        wlr_buffer_end_data_ptr_access (buffer);
    }

  return fmt != DRM_FORMAT_XRGB8888 && fmt != DRM_FORMAT_XBGR8888 &&
         fmt != DRM_FORMAT_RGB888 && fmt != DRM_FORMAT_BGR888;
}

static void
setup_skin_weights_for_child (GthreeGeometry *geom,
                              float           local_x,
                              float           parent_pixel_width)
{
  GthreeAttribute *position = gthree_geometry_get_position (geom);
  int n_vert = gthree_attribute_get_count (position);

  GthreeAttribute *skin_indices = gthree_attribute_new ("skinIndex",
                                                         GTHREE_ATTRIBUTE_TYPE_UINT16,
                                                         n_vert, 4, FALSE);
  GthreeAttribute *skin_weights = gthree_attribute_new ("skinWeight",
                                                         GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                                         n_vert, 4, FALSE);

  float segment = parent_pixel_width / BONE_COUNT;

  for (int i = 0; i < n_vert; i++)
    {
      graphene_point3d_t vertex;
      gthree_attribute_get_point3d (position, i, &vertex);
      float toplevel_x = vertex.x + local_x + parent_pixel_width / 2.0f;
      int bone_idx = (int)(toplevel_x / segment);
      float weight = toplevel_x / segment - bone_idx;
      if (bone_idx < 0)
        {
          bone_idx = 0;
          weight = 0;
        }
      if (bone_idx >= BONE_COUNT)
        {
          bone_idx = BONE_COUNT - 1;
          weight = 1.0f;
        }

      guint16 *idx = gthree_attribute_peek_uint16_at (skin_indices, i);
      idx[0] = bone_idx;
      idx[1] = MIN (bone_idx + 1, BONE_COUNT - 1);
      idx[2] = 0;
      idx[3] = 0;

      float *wt = gthree_attribute_peek_float_at (skin_weights, i);
      wt[0] = 1.0f - weight;
      wt[1] = weight;
      wt[2] = 0;
      wt[3] = 0;
    }

  gthree_geometry_add_attribute (geom, "skinIndex", skin_indices);
  gthree_geometry_add_attribute (geom, "skinWeight", skin_weights);
}

static void
build_toplevel_skinned_mesh (CompositorToplevel *ct)
{
  g_autoptr (GthreeGeometry) geom =
    gthree_geometry_new_plane (ct->pixel_width, ct->pixel_height, BONE_COUNT, 6);
  flip_plane_uvs_y (geom);
  setup_skin_weights_for_child (geom, 0, ct->pixel_width);

  float bone_spacing = ct->pixel_width / BONE_COUNT;
  for (int i = 0; i < BONE_COUNT; i++)
    {
      ct->bones[i] = gthree_bone_new ();
      if (i == 0)
        gthree_object_set_position_xyz (GTHREE_OBJECT (ct->bones[i]),
                                        -ct->pixel_width / 2.0f, 0, 0);
      else
        gthree_object_set_position_xyz (GTHREE_OBJECT (ct->bones[i]),
                                        bone_spacing, 0, 0);

      if (i > 0)
        gthree_object_add_child (GTHREE_OBJECT (ct->bones[i - 1]),
                                 GTHREE_OBJECT (ct->bones[i]));
    }
  ct->root_bone = ct->bones[0];

  GthreeSkinnedMesh *skinned = gthree_skinned_mesh_new (geom, GTHREE_MATERIAL (ct->material));
  ct->mesh = GTHREE_MESH (skinned);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (ct->mesh), TRUE);

  gthree_object_add_child (GTHREE_OBJECT (ct->mesh), GTHREE_OBJECT (ct->root_bone));

  ct->skeleton = gthree_skeleton_new (ct->bones, BONE_COUNT, NULL);
  gthree_skinned_mesh_bind (skinned, ct->skeleton, NULL);

  gthree_object_add_child (GTHREE_OBJECT (ct->group), GTHREE_OBJECT (ct->mesh));
}

static void
destroy_toplevel_skinned_mesh (CompositorToplevel *ct)
{
  if (ct->mesh)
    {
      gthree_object_remove_child (GTHREE_OBJECT (ct->group), GTHREE_OBJECT (ct->mesh));
      g_clear_object (&ct->mesh);
    }
  g_clear_object (&ct->skeleton);
}

static void
on_toplevel_map (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorToplevel *ct = wl_container_of (listener, ct, map);
  CompositorState *state = ct->state;

  struct wlr_surface *surface = ct->xdg_toplevel->base->surface;
  struct wlr_buffer *buffer = surface->current.buffer;
  ct->pixel_width = buffer ? (float)buffer->width : DEFAULT_WIDTH;
  ct->pixel_height = buffer ? (float)buffer->height : DEFAULT_HEIGHT;
  ct->quad_width = ct->pixel_width * PAPER_SCALE;
  ct->quad_height = ct->pixel_height * PAPER_SCALE;

  create_surface_material (&ct->material, &ct->texture,
                           buffer && buffer_has_alpha (buffer));
  gthree_mesh_material_set_skinning (GTHREE_MESH_MATERIAL (ct->material), TRUE);

  ct->group = gthree_group_new ();
  set_uniform_scale (GTHREE_OBJECT (ct->group), PAPER_SCALE);
  graphene_euler_t rot;
  graphene_euler_init (&rot, -90, 0, 0);
  gthree_object_set_rotation (GTHREE_OBJECT (ct->group), &rot);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (ct->group), TRUE);

  build_toplevel_skinned_mesh (ct);

  assign_rest_position (state, ct);

  ct->paper_state = PAPER_FALLING;
  ct->fall_time = 0;
  ct->fall_start_y = FALL_START_Y;
  gthree_object_set_position_xyz (GTHREE_OBJECT (ct->group),
                                  ct->rest_x, ct->fall_start_y, ct->rest_z);
  gthree_object_add_child (GTHREE_OBJECT (state->scene), GTHREE_OBJECT (ct->group));

  state->toplevels = g_list_append (state->toplevels, ct);

  if (buffer)
    import_buffer_to_texture (state, buffer, ct->texture);

  focus_toplevel (state, ct);
  gtk_widget_queue_draw (state->area);

  if (state->debug)
    g_print ("TOPLEVEL map: size=%.0fx%.0f paper=%.0fx%.0f buffer=%p\n",
             ct->pixel_width, ct->pixel_height,
             ct->quad_width, ct->quad_height, (void *)buffer);
}

static void
on_toplevel_unmap (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorToplevel *ct = wl_container_of (listener, ct, unmap);
  CompositorState *state = ct->state;

  if (state->grabbed_toplevel == ct)
    state->grabbed_toplevel = NULL;

  if (state->focused_toplevel == ct)
    {
      state->focused_toplevel = NULL;
      GList *first = state->toplevels;
      if (first && first->data != ct)
        focus_toplevel (state, first->data);
    }

  state->toplevels = g_list_remove (state->toplevels, ct);

  if (ct->group)
    {
      destroy_toplevel_skinned_mesh (ct);
      gthree_object_remove_child (GTHREE_OBJECT (state->scene), GTHREE_OBJECT (ct->group));
      g_clear_object (&ct->group);
    }
  g_clear_object (&ct->texture);
  g_clear_object (&ct->material);

  gtk_widget_queue_draw (state->area);
}

static gboolean
import_buffer_to_texture (CompositorState *state, struct wlr_buffer *buffer, GthreeTexture *texture)
{
  struct wlr_dmabuf_attributes dmabuf_attrs;
  if (!state->no_dmabuf && wlr_buffer_get_dmabuf (buffer, &dmabuf_attrs))
    {
      gtk_gl_area_make_current (GTK_GL_AREA (state->area));
      GthreeRenderer *renderer = gthree_area_get_renderer (GTHREE_AREA (state->area));
      g_autoptr (GError) error = NULL;

      if (gthree_texture_set_from_dmabuf (texture, renderer,
                                          dmabuf_attrs.fd[0],
                                          dmabuf_attrs.format,
                                          dmabuf_attrs.modifier,
                                          dmabuf_attrs.width, dmabuf_attrs.height,
                                          dmabuf_attrs.offset[0],
                                          dmabuf_attrs.stride[0],
                                          &error))
        return TRUE;

      if (state->debug)
        g_print ("BUFFER dmabuf import failed: %s, trying SHM\n", error->message);
    }

  void *buf_data;
  uint32_t fmt;
  size_t stride;
  if (!wlr_buffer_begin_data_ptr_access (buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ,
                                          &buf_data, &fmt, &stride))
    {
      if (state->debug)
        g_print ("BUFFER SHM access failed for %dx%d buffer\n",
                 buffer->width, buffer->height);
      return FALSE;
    }

  GthreeMemoryFormat gfmt = drm_to_gthree_format (fmt);
  g_autoptr (GBytes) bytes = g_bytes_new (buf_data, stride * buffer->height);
  gthree_texture_set_from_bytes (texture, bytes, buffer->width, buffer->height, stride, gfmt);
  gthree_texture_set_needs_update (texture);
  wlr_buffer_end_data_ptr_access (buffer);
  return TRUE;
}

static void
on_toplevel_commit (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorToplevel *ct = wl_container_of (listener, ct, commit);

  if (ct->xdg_toplevel->base->initial_commit)
    {
      wlr_xdg_toplevel_set_size (ct->xdg_toplevel, 0, 0);
      return;
    }

  struct wlr_surface *surface = ct->xdg_toplevel->base->surface;
  struct wlr_buffer *buffer = surface->current.buffer;
  if (!buffer || !ct->texture)
    {
      if (ct->state->debug)
        g_print ("TOPLEVEL commit: skipped (buffer=%p texture=%p)\n",
                 (void *)buffer, (void *)ct->texture);
      return;
    }

  if (!import_buffer_to_texture (ct->state, buffer, ct->texture))
    {
      if (ct->state->debug)
        g_print ("TOPLEVEL commit: import failed for %dx%d buffer\n",
                 buffer->width, buffer->height);
      return;
    }

  float new_pw = (float)buffer->width;
  float new_ph = (float)buffer->height;
  if (new_pw != ct->pixel_width || new_ph != ct->pixel_height)
    {
      ct->pixel_width = new_pw;
      ct->pixel_height = new_ph;
      ct->quad_width = new_pw * PAPER_SCALE;
      ct->quad_height = new_ph * PAPER_SCALE;

      destroy_toplevel_skinned_mesh (ct);
      build_toplevel_skinned_mesh (ct);

      for (GList *s = ct->subsurfaces; s; s = s->next)
        {
          CompositorSubsurface *sub = s->data;
          if (!sub->mesh)
            continue;

          gthree_object_remove_child (GTHREE_OBJECT (ct->group), GTHREE_OBJECT (sub->mesh));
          g_clear_object (&sub->mesh);

          g_autoptr (GthreeGeometry) sgeom =
            gthree_geometry_new_plane (sub->quad_width, sub->quad_height, 1, 1);
          flip_plane_uvs_y (sgeom);
          sub->mesh = create_subsurface_mesh (sub, sgeom);
          gthree_object_add_child (GTHREE_OBJECT (ct->group), GTHREE_OBJECT (sub->mesh));
          position_subsurface_mesh (sub);
        }
    }

  gtk_widget_queue_draw (ct->state->area);
}

static void
on_toplevel_destroy (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorToplevel *ct = wl_container_of (listener, ct, destroy);

  wl_list_remove (&ct->map.link);
  wl_list_remove (&ct->unmap.link);
  wl_list_remove (&ct->commit.link);
  wl_list_remove (&ct->destroy.link);
  wl_list_remove (&ct->new_subsurface.link);
  wl_list_remove (&ct->request_move.link);
  wl_list_remove (&ct->request_resize.link);
  wl_list_remove (&ct->request_maximize.link);
  wl_list_remove (&ct->request_fullscreen.link);

  g_clear_object (&ct->mesh);
  g_clear_object (&ct->group);
  g_clear_object (&ct->skeleton);
  g_clear_object (&ct->texture);
  g_clear_object (&ct->material);

  for (GList *l = ct->subsurfaces; l; l = l->next)
    ((CompositorSubsurface *)l->data)->toplevel = NULL;
  g_list_free (ct->subsurfaces);

  for (GList *l = ct->popups; l; l = l->next)
    ((CompositorPopup *)l->data)->toplevel = NULL;
  g_list_free (ct->popups);

  g_free (ct);
}

static void
on_toplevel_request_noop (G_GNUC_UNUSED struct wl_listener *listener,
                          G_GNUC_UNUSED void *data)
{
}

static void
on_new_xdg_toplevel (struct wl_listener *listener, void *data)
{
  CompositorState *state = wl_container_of (listener, state, new_xdg_toplevel);
  struct wlr_xdg_toplevel *xdg_toplevel = data;

  CompositorToplevel *ct = g_new0 (CompositorToplevel, 1);
  ct->state = state;
  ct->xdg_toplevel = xdg_toplevel;
  xdg_toplevel->base->data = ct;

  ct->map.notify = on_toplevel_map;
  wl_signal_add (&xdg_toplevel->base->surface->events.map, &ct->map);
  ct->unmap.notify = on_toplevel_unmap;
  wl_signal_add (&xdg_toplevel->base->surface->events.unmap, &ct->unmap);
  ct->commit.notify = on_toplevel_commit;
  wl_signal_add (&xdg_toplevel->base->surface->events.commit, &ct->commit);
  ct->destroy.notify = on_toplevel_destroy;
  wl_signal_add (&xdg_toplevel->events.destroy, &ct->destroy);

  ct->new_subsurface.notify = on_new_subsurface;
  wl_signal_add (&xdg_toplevel->base->surface->events.new_subsurface, &ct->new_subsurface);

  ct->request_move.notify = on_toplevel_request_noop;
  wl_signal_add (&xdg_toplevel->events.request_move, &ct->request_move);
  ct->request_resize.notify = on_toplevel_request_noop;
  wl_signal_add (&xdg_toplevel->events.request_resize, &ct->request_resize);
  ct->request_maximize.notify = on_toplevel_request_noop;
  wl_signal_add (&xdg_toplevel->events.request_maximize, &ct->request_maximize);
  ct->request_fullscreen.notify = on_toplevel_request_noop;
  wl_signal_add (&xdg_toplevel->events.request_fullscreen, &ct->request_fullscreen);

  surface_notify_scale (xdg_toplevel->base->surface, 1.0f);

  if (state->debug)
    g_print ("TOPLEVEL new: %s\n", xdg_toplevel->title ? xdg_toplevel->title : "(untitled)");
}

/* --- Popup handling --- */

static void
position_popup_mesh (CompositorPopup *popup)
{
  if (!popup->mesh || !popup->toplevel || !popup->toplevel->group)
    return;

  CompositorToplevel *ct = popup->toplevel;
  struct wlr_box geo = popup->xdg_popup->current.geometry;
  struct wlr_box tgeo = ct->xdg_toplevel->base->geometry;

  float buf_x = tgeo.x + geo.x + geo.width / 2.0f;
  float buf_y = tgeo.y + geo.y + geo.height / 2.0f;
  float local_x = buf_x - (ct->pixel_width / 2.0f);
  float local_y = (ct->pixel_height / 2.0f) - buf_y;

  gthree_object_set_position_xyz (GTHREE_OBJECT (popup->mesh), local_x, local_y, 5.0f);

  if (popup->state->debug)
    g_print ("POPUP position: geo=(%d,%d) tgeo=(%d,%d %dx%d) quad=%.0fx%.0f "
             "local=(%.0f,%.0f)\n",
             geo.x, geo.y, tgeo.x, tgeo.y, tgeo.width, tgeo.height,
             ct->quad_width, ct->quad_height, local_x, local_y);
}

static void
on_popup_map (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorPopup *popup = wl_container_of (listener, popup, map);
  if (!popup->toplevel)
    return;

  CompositorState *state = popup->state;
  struct wlr_box geo = popup->xdg_popup->current.geometry;

  popup->quad_width = geo.width > 0 ? (float)geo.width : DEFAULT_WIDTH;
  popup->quad_height = geo.height > 0 ? (float)geo.height : DEFAULT_HEIGHT;

  g_autoptr (GthreeGeometry) geom =
    gthree_geometry_new_plane (popup->quad_width, popup->quad_height, 1, 1);
  flip_plane_uvs_y (geom);
  create_surface_material (&popup->material, &popup->texture, TRUE);

  popup->mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (popup->material));
  gthree_object_add_child (GTHREE_OBJECT (popup->toplevel->group), GTHREE_OBJECT (popup->mesh));

  struct wlr_surface *surface = popup->xdg_popup->base->surface;
  struct wlr_buffer *buffer = surface->current.buffer;
  if (buffer)
    import_buffer_to_texture (state, buffer, popup->texture);

  position_popup_mesh (popup);
  gtk_widget_queue_draw (state->area);

  if (state->debug)
    {
      struct wlr_box dgeo = popup->xdg_popup->current.geometry;
      g_print ("POPUP map: size=%.0fx%.0f geo=(%d,%d %dx%d) buffer=%p\n",
               popup->quad_width, popup->quad_height,
               dgeo.x, dgeo.y, dgeo.width, dgeo.height,
               (void *)buffer);
    }
}

static void
on_popup_unmap (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorPopup *popup = wl_container_of (listener, popup, unmap);

  if (popup->state->debug)
    g_print ("POPUP unmap\n");

  if (popup->mesh)
    {
      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (popup->mesh));
      if (parent)
        gthree_object_remove_child (parent, GTHREE_OBJECT (popup->mesh));
      g_clear_object (&popup->mesh);
    }
  g_clear_object (&popup->texture);
  g_clear_object (&popup->material);

  gtk_widget_queue_draw (popup->state->area);
}

static void
on_popup_commit (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorPopup *popup = wl_container_of (listener, popup, commit);

  if (popup->xdg_popup->base->initial_commit)
    {
      CompositorToplevel *ct = popup->toplevel;
      int tw = ct ? (int)ct->pixel_width : (int)DEFAULT_WIDTH;
      int th = ct ? (int)ct->pixel_height : (int)DEFAULT_HEIGHT;
      struct wlr_box box = { .x = 0, .y = 0, .width = tw, .height = th };
      wlr_xdg_popup_unconstrain_from_box (popup->xdg_popup, &box);

      if (popup->state->debug)
        {
          struct wlr_box geo = popup->xdg_popup->scheduled.geometry;
          g_print ("POPUP initial_commit: constraint_box=(%d,%d %dx%d) "
                   "scheduled_geo=(%d,%d %dx%d)\n",
                   box.x, box.y, box.width, box.height,
                   geo.x, geo.y, geo.width, geo.height);
        }
      return;
    }

  struct wlr_surface *surface = popup->xdg_popup->base->surface;
  struct wlr_buffer *buffer = surface->current.buffer;
  if (!buffer || !popup->texture)
    {
      if (popup->state->debug)
        g_print ("POPUP commit: skipped (buffer=%p texture=%p)\n",
                 (void *)buffer, (void *)popup->texture);
      return;
    }

  if (popup->state->debug)
    {
      void *peek_data;
      uint32_t peek_fmt;
      size_t peek_stride;
      if (wlr_buffer_begin_data_ptr_access (buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ,
                                             &peek_data, &peek_fmt, &peek_stride))
        {
          uint32_t *pixels = peek_data;
          g_print ("POPUP buffer peek: fmt=0x%x stride=%zu first_pixels=[%08x %08x %08x %08x]\n",
                   peek_fmt, peek_stride, pixels[0], pixels[1], pixels[2], pixels[3]);
          wlr_buffer_end_data_ptr_access (buffer);
        }
      else
        g_print ("POPUP buffer peek: cannot access data\n");
    }

  if (!import_buffer_to_texture (popup->state, buffer, popup->texture))
    return;

  float new_w = (float)buffer->width;
  float new_h = (float)buffer->height;
  if (popup->mesh && (new_w != popup->quad_width || new_h != popup->quad_height))
    {
      popup->quad_width = new_w;
      popup->quad_height = new_h;

      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (popup->mesh));
      gthree_object_remove_child (parent, GTHREE_OBJECT (popup->mesh));
      g_clear_object (&popup->mesh);

      g_autoptr (GthreeGeometry) geom = gthree_geometry_new_plane (new_w, new_h, 1, 1);
      flip_plane_uvs_y (geom);
      popup->mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (popup->material));
      gthree_object_add_child (parent, GTHREE_OBJECT (popup->mesh));
    }

  position_popup_mesh (popup);
  gtk_widget_queue_draw (popup->state->area);

  if (popup->state->debug)
    {
      struct wlr_box geo = popup->xdg_popup->current.geometry;
      g_print ("POPUP commit: buffer=%dx%d quad=%.0fx%.0f geo=(%d,%d %dx%d)\n",
               buffer->width, buffer->height,
               popup->quad_width, popup->quad_height,
               geo.x, geo.y, geo.width, geo.height);
    }
}

static void
on_popup_destroy (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorPopup *popup = wl_container_of (listener, popup, destroy);

  if (popup->state->debug)
    g_print ("POPUP destroy\n");

  if (popup->toplevel)
    popup->toplevel->popups = g_list_remove (popup->toplevel->popups, popup);

  if (popup->mesh)
    {
      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (popup->mesh));
      if (parent)
        gthree_object_remove_child (parent, GTHREE_OBJECT (popup->mesh));
      g_clear_object (&popup->mesh);
    }
  g_clear_object (&popup->texture);
  g_clear_object (&popup->material);

  wl_list_remove (&popup->map.link);
  wl_list_remove (&popup->unmap.link);
  wl_list_remove (&popup->commit.link);
  wl_list_remove (&popup->destroy.link);
  wl_list_remove (&popup->new_subsurface.link);

  for (GList *l = popup->subsurfaces; l; l = l->next)
    ((CompositorSubsurface *)l->data)->popup = NULL;
  g_list_free (popup->subsurfaces);

  g_free (popup);
}

static CompositorToplevel *
find_toplevel_for_popup (CompositorState *state, struct wlr_xdg_surface *surface)
{
  while (surface)
    {
      if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
        return surface->data;
      if (surface->role == WLR_XDG_SURFACE_ROLE_POPUP && surface->popup->parent)
        surface = wlr_xdg_surface_try_from_wlr_surface (surface->popup->parent);
      else
        break;
    }
  return NULL;
}

static void
on_new_xdg_popup (G_GNUC_UNUSED struct wl_listener *listener, void *data)
{
  CompositorState *state = wl_container_of (listener, state, new_xdg_popup);
  struct wlr_xdg_popup *xdg_popup = data;

  CompositorPopup *popup = g_new0 (CompositorPopup, 1);
  popup->state = state;
  popup->xdg_popup = xdg_popup;
  xdg_popup->base->data = popup;

  popup->toplevel = find_toplevel_for_popup (state, xdg_popup->base);

  if (popup->toplevel)
    popup->toplevel->popups = g_list_prepend (popup->toplevel->popups, popup);

  popup->map.notify = on_popup_map;
  wl_signal_add (&xdg_popup->base->surface->events.map, &popup->map);
  popup->unmap.notify = on_popup_unmap;
  wl_signal_add (&xdg_popup->base->surface->events.unmap, &popup->unmap);
  popup->commit.notify = on_popup_commit;
  wl_signal_add (&xdg_popup->base->surface->events.commit, &popup->commit);
  popup->destroy.notify = on_popup_destroy;
  wl_signal_add (&xdg_popup->events.destroy, &popup->destroy);

  popup->new_subsurface.notify = on_popup_new_subsurface;
  wl_signal_add (&xdg_popup->base->surface->events.new_subsurface, &popup->new_subsurface);

  surface_notify_scale (xdg_popup->base->surface, 1.0f);

  if (state->debug)
    {
      struct wlr_xdg_positioner_rules *rules = &xdg_popup->scheduled.rules;
      g_print ("POPUP new: toplevel=%p anchor_rect=(%d,%d %dx%d) size=(%dx%d) "
               "anchor=%d gravity=%d constraint=%d\n",
               (void *)popup->toplevel,
               rules->anchor_rect.x, rules->anchor_rect.y,
               rules->anchor_rect.width, rules->anchor_rect.height,
               rules->size.width, rules->size.height,
               rules->anchor, rules->gravity, rules->constraint_adjustment);
    }
}

static void
on_popup_new_subsurface (struct wl_listener *listener, void *data)
{
  CompositorPopup *popup = wl_container_of (listener, popup, new_subsurface);
  struct wlr_subsurface *wlr_subsurface = data;

  CompositorSubsurface *sub = g_new0 (CompositorSubsurface, 1);
  sub->state = popup->state;
  sub->wlr_subsurface = wlr_subsurface;
  sub->popup = popup;

  popup->subsurfaces = g_list_prepend (popup->subsurfaces, sub);

  sub->map.notify = on_subsurface_map;
  wl_signal_add (&wlr_subsurface->surface->events.map, &sub->map);
  sub->unmap.notify = on_subsurface_unmap;
  wl_signal_add (&wlr_subsurface->surface->events.unmap, &sub->unmap);
  sub->commit.notify = on_subsurface_commit;
  wl_signal_add (&wlr_subsurface->surface->events.commit, &sub->commit);
  sub->destroy.notify = on_subsurface_destroy;
  wl_signal_add (&wlr_subsurface->events.destroy, &sub->destroy);

  surface_notify_scale (wlr_subsurface->surface, 1.0f);

  if (popup->state->debug)
    g_print ("POPUP SUBSURFACE new: popup=%p subsurface=%p\n",
             (void *)popup, (void *)wlr_subsurface);
}

/* --- Subsurface handling --- */

static void
update_subsurface_polygon_offsets (CompositorToplevel *ct)
{
  int i = 1;
  for (GList *l = ct->subsurfaces; l; l = l->next)
    {
      CompositorSubsurface *sub = l->data;
      if (sub->material)
        gthree_material_set_polygon_offset (GTHREE_MATERIAL (sub->material), TRUE, -i, -i);
      i++;
    }
}

static GthreeObject *
subsurface_get_parent_object (CompositorSubsurface *sub)
{
  if (sub->toplevel)
    return GTHREE_OBJECT (sub->toplevel->group);
  if (sub->popup)
    return GTHREE_OBJECT (sub->popup->mesh);
  return NULL;
}

static gboolean
subsurface_get_parent_size (CompositorSubsurface *sub, float *w, float *h)
{
  if (sub->toplevel && sub->toplevel->group)
    {
      *w = sub->toplevel->pixel_width;
      *h = sub->toplevel->pixel_height;
      return TRUE;
    }
  if (sub->popup && sub->popup->mesh)
    {
      *w = sub->popup->quad_width;
      *h = sub->popup->quad_height;
      return TRUE;
    }
  return FALSE;
}

static GthreeMesh *
create_subsurface_mesh (CompositorSubsurface *sub,
                        GthreeGeometry       *geom)
{
  CompositorToplevel *ct = sub->toplevel;

  if (ct && ct->skeleton)
    {
      int sx = sub->wlr_subsurface->current.x;
      float local_x = (sx + sub->quad_width / 2.0f) - (ct->pixel_width / 2.0f);

      g_autoptr (GthreeGeometry) skinned_geom =
        gthree_geometry_new_plane (sub->quad_width, sub->quad_height, 8, 1);
      flip_plane_uvs_y (skinned_geom);

      GthreeAttribute *position = gthree_geometry_get_position (skinned_geom);
      int n_vert = gthree_attribute_get_count (position);
      for (int i = 0; i < n_vert; i++)
        {
          graphene_point3d_t v;
          gthree_attribute_get_point3d (position, i, &v);
          v.x += local_x;
          gthree_attribute_set_xyz (position, i, v.x, v.y, v.z);
        }

      setup_skin_weights_for_child (skinned_geom, 0, ct->pixel_width);
      gthree_mesh_material_set_skinning (GTHREE_MESH_MATERIAL (sub->material), TRUE);

      GthreeSkinnedMesh *skinned = gthree_skinned_mesh_new (skinned_geom, GTHREE_MATERIAL (sub->material));
      const graphene_matrix_t *bind = gthree_skinned_mesh_get_bind_matrix (GTHREE_SKINNED_MESH (ct->mesh));
      gthree_skinned_mesh_bind (skinned, ct->skeleton, bind);
      return GTHREE_MESH (skinned);
    }

  return gthree_mesh_new (geom, GTHREE_MATERIAL (sub->material));
}

static void
position_subsurface_mesh (CompositorSubsurface *sub)
{
  float parent_w, parent_h;
  if (!sub->mesh || !subsurface_get_parent_size (sub, &parent_w, &parent_h))
    return;

  int sx = sub->wlr_subsurface->current.x;
  int sy = sub->wlr_subsurface->current.y;

  float local_x = (sx + sub->quad_width / 2.0f) - (parent_w / 2.0f);
  float local_y = (parent_h / 2.0f) - (sy + sub->quad_height / 2.0f);

  if (sub->toplevel && sub->toplevel->skeleton)
    gthree_object_set_position_xyz (GTHREE_OBJECT (sub->mesh), 0, local_y, 2.5f);
  else
    gthree_object_set_position_xyz (GTHREE_OBJECT (sub->mesh), local_x, local_y, 2.5f);
}

static void
on_subsurface_map (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorSubsurface *sub = wl_container_of (listener, sub, map);

  if (sub->state->debug)
    g_print ("SUBSURFACE map\n");
}

static void
on_subsurface_unmap (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorSubsurface *sub = wl_container_of (listener, sub, unmap);

  if (sub->state->debug)
    g_print ("SUBSURFACE unmap\n");

  if (sub->mesh)
    {
      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (sub->mesh));
      if (parent)
        gthree_object_remove_child (parent, GTHREE_OBJECT (sub->mesh));
      g_clear_object (&sub->mesh);
    }
  g_clear_object (&sub->texture);
  g_clear_object (&sub->material);

  gtk_widget_queue_draw (sub->state->area);
}

static void
on_subsurface_commit (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorSubsurface *sub = wl_container_of (listener, sub, commit);
  CompositorState *state = sub->state;

  struct wlr_surface *surface = sub->wlr_subsurface->surface;
  struct wlr_buffer *buffer = surface->current.buffer;
  if (!buffer)
    return;

  GthreeObject *parent_obj = subsurface_get_parent_object (sub);
  if (!parent_obj)
    return;

  if (!sub->texture)
    {
      float w = (float)buffer->width;
      float h = (float)buffer->height;
      sub->quad_width = w;
      sub->quad_height = h;

      create_surface_material (&sub->material, &sub->texture, buffer_has_alpha (buffer));

      g_autoptr (GthreeGeometry) geom = gthree_geometry_new_plane (w, h, 1, 1);
      flip_plane_uvs_y (geom);
      sub->mesh = create_subsurface_mesh (sub, geom);
      gthree_object_add_child (parent_obj, GTHREE_OBJECT (sub->mesh));

      if (sub->toplevel)
        update_subsurface_polygon_offsets (sub->toplevel);

      if (state->debug)
        g_print ("SUBSURFACE created mesh: size=%.0fx%.0f pos=(%d,%d)\n",
                 w, h, sub->wlr_subsurface->current.x, sub->wlr_subsurface->current.y);
    }

  if (!import_buffer_to_texture (state, buffer, sub->texture))
    return;

  float new_w = (float)buffer->width;
  float new_h = (float)buffer->height;
  if (sub->mesh && (new_w != sub->quad_width || new_h != sub->quad_height))
    {
      sub->quad_width = new_w;
      sub->quad_height = new_h;

      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (sub->mesh));
      gthree_object_remove_child (parent, GTHREE_OBJECT (sub->mesh));
      g_clear_object (&sub->mesh);

      g_autoptr (GthreeGeometry) geom = gthree_geometry_new_plane (new_w, new_h, 1, 1);
      flip_plane_uvs_y (geom);
      sub->mesh = create_subsurface_mesh (sub, geom);
      gthree_object_add_child (parent, GTHREE_OBJECT (sub->mesh));
    }

  position_subsurface_mesh (sub);
  gtk_widget_queue_draw (state->area);
}

static void
on_subsurface_destroy (struct wl_listener *listener, G_GNUC_UNUSED void *data)
{
  CompositorSubsurface *sub = wl_container_of (listener, sub, destroy);

  if (sub->state->debug)
    g_print ("SUBSURFACE destroy\n");

  CompositorToplevel *toplevel = sub->toplevel;
  if (toplevel)
    toplevel->subsurfaces = g_list_remove (toplevel->subsurfaces, sub);
  if (sub->popup)
    sub->popup->subsurfaces = g_list_remove (sub->popup->subsurfaces, sub);

  if (sub->mesh)
    {
      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (sub->mesh));
      if (parent)
        gthree_object_remove_child (parent, GTHREE_OBJECT (sub->mesh));
      g_clear_object (&sub->mesh);
    }
  g_clear_object (&sub->texture);
  g_clear_object (&sub->material);

  wl_list_remove (&sub->map.link);
  wl_list_remove (&sub->unmap.link);
  wl_list_remove (&sub->commit.link);
  wl_list_remove (&sub->destroy.link);
  g_free (sub);

  if (toplevel)
    update_subsurface_polygon_offsets (toplevel);
}

static void
on_new_subsurface (struct wl_listener *listener, void *data)
{
  CompositorToplevel *ct = wl_container_of (listener, ct, new_subsurface);
  struct wlr_subsurface *wlr_subsurface = data;

  CompositorSubsurface *sub = g_new0 (CompositorSubsurface, 1);
  sub->state = ct->state;
  sub->wlr_subsurface = wlr_subsurface;
  sub->toplevel = ct;

  ct->subsurfaces = g_list_append (ct->subsurfaces, sub);

  sub->map.notify = on_subsurface_map;
  wl_signal_add (&wlr_subsurface->surface->events.map, &sub->map);
  sub->unmap.notify = on_subsurface_unmap;
  wl_signal_add (&wlr_subsurface->surface->events.unmap, &sub->unmap);
  sub->commit.notify = on_subsurface_commit;
  wl_signal_add (&wlr_subsurface->surface->events.commit, &sub->commit);
  sub->destroy.notify = on_subsurface_destroy;
  wl_signal_add (&wlr_subsurface->events.destroy, &sub->destroy);

  surface_notify_scale (wlr_subsurface->surface, 1.0f);

  if (ct->state->debug)
    g_print ("SUBSURFACE new: toplevel=%p\n", (void *)ct);
}

static void
on_request_activate (struct wl_listener *listener, void *data)
{
  CompositorState *state = wl_container_of (listener, state, request_activate);
  struct wlr_xdg_activation_v1_request_activate_event *event = data;
  struct wlr_xdg_toplevel *xdg_toplevel = wlr_xdg_toplevel_try_from_wlr_surface (event->surface);
  if (!xdg_toplevel)
    return;

  CompositorToplevel *ct = xdg_toplevel->base->data;
  if (ct)
    focus_toplevel (state, ct);
}

static void
on_request_set_shape (G_GNUC_UNUSED struct wl_listener *listener,
                      G_GNUC_UNUSED void *data)
{
}

static void
on_request_set_selection (struct wl_listener *listener, void *data)
{
  CompositorState *state = wl_container_of (listener, state, request_set_selection);
  struct wlr_seat_request_set_selection_event *event = data;
  wlr_seat_set_selection (state->seat, event->source, event->serial);
}

/* --- Backend / Output stubs --- */

static bool
backend_start (G_GNUC_UNUSED struct wlr_backend *b)
{
  return true;
}

static void
backend_destroy (struct wlr_backend *b)
{
  CompositorState *state = wl_container_of (b, state, backend);
  wlr_backend_finish (&state->backend);
  wlr_output_destroy (&state->output);
}

static bool
output_commit (G_GNUC_UNUSED struct wlr_output *o,
               G_GNUC_UNUSED const struct wlr_output_state *s)
{
  return true;
}

static void
output_destroy (G_GNUC_UNUSED struct wlr_output *o)
{
}

static void
output_handle_bind (G_GNUC_UNUSED struct wl_listener *listener, void *data)
{
  const struct wlr_output_event_bind *e = data;
  struct wlr_output *o = e->output;
  wl_output_send_geometry (e->resource, 0, 0, o->phys_width, o->phys_height,
                           o->subpixel, o->make, o->model, o->transform);
}

static void
set_output_resolution (CompositorState *state, int w, int h)
{
  struct wlr_output_state os;
  wlr_output_state_init (&os);
  wlr_output_state_set_enabled (&os, true);
  wlr_output_state_set_custom_mode (&os, w, h, 0);
  wlr_output_state_set_scale (&os, 1.0f);
  wlr_output_commit_state (&state->output, &os);
  wlr_output_state_finish (&os);
}

/* --- Keyboard --- */

static struct xkb_keymap *
get_xkb_keymap (GdkDevice *gkeyboard)
{
  struct xkb_keymap *keymap = NULL;

#ifdef GDK_WINDOWING_WAYLAND
  if (GDK_IS_WAYLAND_DEVICE (gkeyboard))
    {
      keymap = gdk_wayland_device_get_xkb_keymap (gkeyboard);
      xkb_keymap_ref (keymap);
    }
#endif

#if defined(GDK_WINDOWING_X11) && defined(HAVE_X11_XCB)
  if (GDK_IS_X11_DEVICE_XI2 (gkeyboard))
    {
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      struct xkb_context *ctx = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
      Display *dpy = gdk_x11_display_get_xdisplay (GDK_X11_DISPLAY (gdk_device_get_display (gkeyboard)));
      keymap = xkb_x11_keymap_new_from_device (ctx, XGetXCBConnection (dpy),
                                               gdk_x11_device_get_id (gkeyboard),
                                               XKB_KEYMAP_COMPILE_NO_FLAGS);
      xkb_context_unref (ctx);
      G_GNUC_END_IGNORE_DEPRECATIONS
    }
#endif

  if (keymap == NULL)
    {
      struct xkb_context *ctx = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
      keymap = xkb_keymap_new_from_names (ctx, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
      xkb_context_unref (ctx);
    }

  return keymap;
}

static void
on_gdk_keyboard_notify (GObject *object, GParamSpec *pspec, CompositorState *state)
{
  GdkDevice *gkeyboard = GDK_DEVICE (object);

  if (g_strcmp0 (pspec->name, "layout-names") == 0)
    {
      struct xkb_keymap *keymap = get_xkb_keymap (gkeyboard);
      wlr_keyboard_set_keymap (&state->keyboard, keymap);
      xkb_keymap_unref (keymap);
    }
  else if (g_strcmp0 (pspec->name, "active-layout-index") == 0)
    {
      state->keyboard.modifiers.group = gdk_device_get_active_layout_index (gkeyboard);
      wlr_seat_keyboard_notify_modifiers (state->seat, &state->keyboard.modifiers);
    }
}

/* --- DRM node detection (for DMABUF feedback) --- */

static dev_t
find_used_drm_node (void)
{
  dev_t result = 0;
  char target[PATH_MAX] = {0};
  struct dirent *dp;
  struct stat st;

  DIR *dir = opendir ("/proc/self/fd");
  if (!dir)
    return 0;

  int dir_fd = dirfd (dir);
  while ((dp = readdir (dir)) != NULL)
    {
      if (dp->d_name[0] == '.')
        continue;
      int len = readlinkat (dir_fd, dp->d_name, target, PATH_MAX - 1);
      if (len < 0)
        continue;
      target[len] = '\0';
      if (strncmp (target, "/dev/dri/", 9) != 0)
        continue;
      if (fstatat (dir_fd, dp->d_name, &st, 0) == 0)
        {
          result = st.st_rdev;
          break;
        }
    }
  closedir (dir);
  return result;
}

static void
configure_dmabuf (struct wl_display *wl_display)
{
  GdkDmabufFormats *gdk_formats = gdk_display_get_dmabuf_formats (gdk_display_get_default ());
  struct wlr_linux_dmabuf_feedback_v1 feedback = {0};
  struct wlr_linux_dmabuf_feedback_v1_tranche *tranche =
    wlr_linux_dmabuf_feedback_add_tranche (&feedback);
  size_t n_formats = gdk_dmabuf_formats_get_n_formats (gdk_formats);

  for (size_t i = 0; i < n_formats; i++)
    {
      uint32_t fourcc;
      uint64_t modifier;
      gdk_dmabuf_formats_get_format (gdk_formats, i, &fourcc, &modifier);
      wlr_drm_format_set_add (&tranche->formats, fourcc, modifier);
    }

  feedback.main_device = tranche->target_device = find_used_drm_node ();

  wlr_linux_dmabuf_v1_create (wl_display, 5, &feedback);
  wlr_linux_dmabuf_feedback_v1_finish (&feedback);
}

/* --- Wayland init --- */

static void
compositor_wlr_init (CompositorState *state)
{
  state->wl_display = wl_display_create ();

  wlr_shm_create (state->wl_display, 2, shm_formats, G_N_ELEMENTS (shm_formats));
  if (!state->no_dmabuf)
    configure_dmabuf (state->wl_display);

  wlr_compositor_create (state->wl_display, 6, NULL);
  wlr_subcompositor_create (state->wl_display);
  wlr_data_device_manager_create (state->wl_display);
  wlr_viewporter_create (state->wl_display);
  wlr_fractional_scale_manager_v1_create (state->wl_display, 1);

  /* XDG shell */
  struct wlr_xdg_shell *xdg_shell = wlr_xdg_shell_create (state->wl_display, 6);
  state->new_xdg_toplevel.notify = on_new_xdg_toplevel;
  wl_signal_add (&xdg_shell->events.new_toplevel, &state->new_xdg_toplevel);
  state->new_xdg_popup.notify = on_new_xdg_popup;
  wl_signal_add (&xdg_shell->events.new_popup, &state->new_xdg_popup);

  /* XDG activation */
  struct wlr_xdg_activation_v1 *activation = wlr_xdg_activation_v1_create (state->wl_display);
  state->request_activate.notify = on_request_activate;
  wl_signal_add (&activation->events.request_activate, &state->request_activate);

  /* Cursor shape */
  struct wlr_cursor_shape_manager_v1 *csm = wlr_cursor_shape_manager_v1_create (state->wl_display, 2);
  state->request_set_shape.notify = on_request_set_shape;
  wl_signal_add (&csm->events.request_set_shape, &state->request_set_shape);

  /* Seat */
  state->seat = wlr_seat_create (state->wl_display, "seat0");
  state->request_set_selection.notify = on_request_set_selection;
  wl_signal_add (&state->seat->events.request_set_selection, &state->request_set_selection);
  wlr_seat_set_capabilities (state->seat, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);

  /* Backend */
  state->backend_impl.start = backend_start;
  state->backend_impl.destroy = backend_destroy;
  wlr_backend_init (&state->backend, &state->backend_impl);

  /* Output */
  state->output_impl.commit = output_commit;
  state->output_impl.destroy = output_destroy;

  struct wlr_output_state os;
  wlr_output_state_init (&os);
  wlr_output_state_set_custom_mode (&os, (int)DEFAULT_WIDTH, (int)DEFAULT_HEIGHT, 0);

  wlr_output_init (&state->output, &state->backend, &state->output_impl,
                    wl_display_get_event_loop (state->wl_display), &os);

  state->output.make = g_strdup ("Gthree");
  state->output.model = g_strdup ("Compositor");

  state->output_bind.notify = output_handle_bind;
  wl_signal_add (&state->output.events.bind, &state->output_bind);

  wlr_output_set_name (&state->output, "GthreeCompositor");
  wlr_output_set_description (&state->output, "Gthree 3D Wayland Compositor");

  struct wlr_output_layout *layout = wlr_output_layout_create (state->wl_display);
  wlr_output_layout_add_auto (layout, &state->output);
  wlr_xdg_output_manager_v1_create (state->wl_display, layout);
  wlr_output_create_global (&state->output, state->wl_display);

  wlr_output_state_finish (&os);

  set_output_resolution (state, (int)DEFAULT_WIDTH, (int)DEFAULT_HEIGHT);

  /* Keyboard */
  wlr_keyboard_init (&state->keyboard, NULL, "gthree-keyboard");
  wlr_pointer_init (&state->pointer, NULL, "gthree-pointer");
  wlr_seat_set_keyboard (state->seat, &state->keyboard);

  /* Socket */
  state->socket = g_strdup_printf ("gthree-compositor-%d", getpid ());
  if (wl_display_add_socket (state->wl_display, state->socket))
    {
      g_warning ("Failed to add socket %s", state->socket);
      g_clear_pointer (&state->socket, g_free);
    }
  else
    g_print ("Compositor listening on WAYLAND_DISPLAY=%s\n", state->socket);

  /* GLib event loop integration */
  state->wl_source = wayland_source_new (state->wl_display);
  g_source_attach (state->wl_source, NULL);
}

/* --- Input event handlers --- */

static gboolean
resolve_hit_input (CompositorState *state, GthreeRayIntersection *hit,
                   CompositorToplevel **out_toplevel,
                   struct wlr_surface **out_surface,
                   double *out_sx, double *out_sy)
{
  GthreeObject *object = hit->object;
  float u = graphene_vec2_get_x (&hit->uv);
  float v = graphene_vec2_get_y (&hit->uv);

  for (GList *l = state->toplevels; l; l = l->next)
    {
      CompositorToplevel *ct = l->data;
      double toplevel_sx = -1, toplevel_sy = -1;

      if (ct->mesh && GTHREE_OBJECT (ct->mesh) == object)
        {
          toplevel_sx = u * ct->pixel_width;
          toplevel_sy = v * ct->pixel_height;
        }
      else
        {
          for (GList *s = ct->subsurfaces; s; s = s->next)
            {
              CompositorSubsurface *sub = s->data;
              if (sub->mesh && GTHREE_OBJECT (sub->mesh) == object)
                {
                  toplevel_sx = sub->wlr_subsurface->current.x + u * sub->quad_width;
                  toplevel_sy = sub->wlr_subsurface->current.y + v * sub->quad_height;
                  break;
                }
            }
        }

      if (toplevel_sx >= 0)
        {
          double sub_sx, sub_sy;
          struct wlr_surface *target =
            wlr_surface_surface_at (ct->xdg_toplevel->base->surface,
                                    toplevel_sx, toplevel_sy, &sub_sx, &sub_sy);
          if (target)
            {
              *out_toplevel = ct;
              *out_surface = target;
              *out_sx = sub_sx;
              *out_sy = sub_sy;
              return TRUE;
            }
        }

      for (GList *p = ct->popups; p; p = p->next)
        {
          CompositorPopup *popup = p->data;
          if (popup->mesh && GTHREE_OBJECT (popup->mesh) == object)
            {
              double popup_sx = u * popup->quad_width;
              double popup_sy = v * popup->quad_height;
              double sub_sx, sub_sy;
              struct wlr_surface *target =
                wlr_surface_surface_at (popup->xdg_popup->base->surface,
                                        popup_sx, popup_sy, &sub_sx, &sub_sy);
              if (target)
                {
                  *out_toplevel = ct;
                  *out_surface = target;
                  *out_sx = sub_sx;
                  *out_sy = sub_sy;
                  return TRUE;
                }
            }
        }
    }
  return FALSE;
}

static GthreeRaycaster *
create_raycaster_from_cursor (CompositorState *state)
{
  int w = gtk_widget_get_width (state->area);
  int h = gtk_widget_get_height (state->area);
  if (w == 0 || h == 0)
    return NULL;

  float nx = ((float)state->cursor_x / w) * 2 - 1;
  float ny = -((float)state->cursor_y / h) * 2 + 1;

  gthree_object_update_matrix_world (GTHREE_OBJECT (state->scene), FALSE);

  GthreeRaycaster *raycaster = gthree_raycaster_new ();
  gthree_raycaster_set_from_camera (raycaster, GTHREE_CAMERA (state->camera), nx, ny);
  return raycaster;
}

static void
handle_raycast_input (CompositorState *state, uint32_t time_msec)
{
  if (!state->area)
    return;

  g_autoptr (GthreeRaycaster) raycaster = create_raycaster_from_cursor (state);
  if (!raycaster)
    return;

  g_autoptr (GPtrArray) hits = gthree_raycaster_intersect_object (
    raycaster, GTHREE_OBJECT (state->scene), TRUE, NULL);

  for (guint i = 0; i < hits->len; i++)
    {
      GthreeRayIntersection *hit = g_ptr_array_index (hits, i);
      CompositorToplevel *toplevel;
      struct wlr_surface *surface;
      double sx, sy;

      if (resolve_hit_input (state, hit, &toplevel, &surface, &sx, &sy))
        {
          if (state->focused_toplevel != toplevel)
            focus_toplevel (state, toplevel);

          if (state->seat->pointer_state.focused_surface != surface)
            wlr_seat_pointer_notify_enter (state->seat, surface, sx, sy);

          wlr_seat_pointer_notify_motion (state->seat, time_msec, sx, sy);
          wlr_seat_pointer_notify_frame (state->seat);
          return;
        }
    }

  wlr_seat_pointer_clear_focus (state->seat);
}

static void
on_motion (GtkEventControllerMotion *controller,
           double x, double y,
           gpointer user_data)
{
  CompositorState *state = user_data;
  state->cursor_x = x;
  state->cursor_y = y;

  if (state->grabbed_toplevel && state->meta_held)
    {
      float hit_x, hit_z;
      if (ray_plane_intersect (state, state->grab_rest_y, &hit_x, &hit_z))
        {
          float target_x = hit_x + state->grab_offset_x;
          float target_z = hit_z + state->grab_offset_z;
          CompositorToplevel *ct = state->grabbed_toplevel;
          const graphene_vec3_t *cur_pos =
            gthree_object_get_position (GTHREE_OBJECT (ct->group));
          float min_y = get_target_height_for_rect (state, ct,
                               target_x, target_z,
                               graphene_vec3_get_y (cur_pos),
                               ct->quad_width * 0.5f, ct->quad_height * 0.5f,
                               ct->rest_rotation_y) + GRAB_LIFT_HEIGHT;
          float cur_y = graphene_vec3_get_y (cur_pos);
          float target_y = cur_y > min_y ? cur_y : min_y;
          gthree_object_set_position_xyz (GTHREE_OBJECT (state->grabbed_toplevel->group),
                                          target_x, target_y, target_z);
          gtk_widget_queue_draw (state->area);
        }
      return;
    }

  if (state->camera_dragging)
    {
      double dx = x - state->drag_last_x;
      double dy = y - state->drag_last_y;
      state->drag_last_x = x;
      state->drag_last_y = y;

      if (state->camera_drag_button == 1)
        {
          state->cam_theta -= (float)dx * CAM_ORBIT_SPEED;
          state->cam_phi -= (float)dy * CAM_ORBIT_SPEED;
          state->cam_phi = CLAMP (state->cam_phi, CAM_PHI_MIN, CAM_PHI_MAX);
        }
      else if (state->camera_drag_button == 3)
        {
          float theta = state->cam_theta;
          float phi = state->cam_phi;

          /* Camera right (horizontal) */
          float rx = cosf (theta);
          float rz = -sinf (theta);

          /* Camera up = forward x right */
          float ux = cosf (phi) * sinf (theta);
          float uy = -sinf (phi);
          float uz = cosf (phi) * cosf (theta);

          float tx = graphene_vec3_get_x (&state->cam_target);
          float ty = graphene_vec3_get_y (&state->cam_target);
          float tz = graphene_vec3_get_z (&state->cam_target);

          tx -= (float)dx * CAM_PAN_SPEED * rx + (float)dy * CAM_PAN_SPEED * ux;
          ty -= (float)dy * CAM_PAN_SPEED * uy;
          tz -= (float)dx * CAM_PAN_SPEED * rz + (float)dy * CAM_PAN_SPEED * uz;
          graphene_vec3_init (&state->cam_target, tx, ty, tz);
        }

      update_camera_from_spherical (state);
      gtk_widget_queue_draw (state->area);
    }
}

static void
on_motion_leave (G_GNUC_UNUSED GtkEventControllerMotion *controller,
                 gpointer user_data)
{
  CompositorState *state = user_data;
  wlr_seat_pointer_clear_focus (state->seat);
}

static uint32_t
gdk_to_linux_button (guint button)
{
  switch (button)
    {
    case 1: return BTN_LEFT;
    case 2: return BTN_MIDDLE;
    case 3: return BTN_RIGHT;
    default: return BTN_LEFT + button - 1;
    }
}

static CompositorToplevel *
raycast_find_toplevel (CompositorState *state)
{
  g_autoptr (GthreeRaycaster) raycaster = create_raycaster_from_cursor (state);
  if (!raycaster)
    return NULL;

  g_autoptr (GPtrArray) hits = gthree_raycaster_intersect_object (
    raycaster, GTHREE_OBJECT (state->scene), TRUE, NULL);

  for (guint i = 0; i < hits->len; i++)
    {
      GthreeRayIntersection *hit = g_ptr_array_index (hits, i);
      CompositorToplevel *toplevel;
      struct wlr_surface *surface;
      double sx, sy;

      if (resolve_hit_input (state, hit, &toplevel, &surface, &sx, &sy))
        return toplevel;
    }
  return NULL;
}

static gboolean
ray_plane_intersect (CompositorState *state, float plane_y,
                     float *out_x, float *out_z)
{
  g_autoptr (GthreeRaycaster) raycaster = create_raycaster_from_cursor (state);
  if (!raycaster)
    return FALSE;

  const graphene_ray_t *ray = gthree_raycaster_get_ray (raycaster);
  graphene_point3d_t origin;
  graphene_vec3_t direction;
  graphene_ray_get_origin (ray, &origin);
  graphene_ray_get_direction (ray, &direction);

  float dy = graphene_vec3_get_y (&direction);
  if (fabsf (dy) < 1e-6f)
    return FALSE;

  float t = (plane_y - origin.y) / dy;
  if (t < 0)
    return FALSE;

  *out_x = origin.x + t * graphene_vec3_get_x (&direction);
  *out_z = origin.z + t * graphene_vec3_get_z (&direction);
  return TRUE;
}

static void
on_click_pressed (GtkGestureClick *gesture,
                  G_GNUC_UNUSED int n_press,
                  double x, double y,
                  gpointer user_data)
{
  CompositorState *state = user_data;
  guint button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));
  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (gesture));
  uint32_t time = gdk_event_get_time (event);

  gtk_widget_grab_focus (state->area);

  if (state->meta_held)
    {
      if (button == 1)
        {
          CompositorToplevel *ct = raycast_find_toplevel (state);
          if (ct)
            {
              state->grabbed_toplevel = ct;
              const graphene_vec3_t *pos =
                gthree_object_get_position (GTHREE_OBJECT (ct->group));
              float orig_y = graphene_vec3_get_y (pos);
              state->grab_rest_y = orig_y;

              float hit_x, hit_z;
              state->grab_offset_x = 0;
              state->grab_offset_z = 0;
              if (ray_plane_intersect (state, orig_y, &hit_x, &hit_z))
                {
                  state->grab_offset_x = graphene_vec3_get_x (pos) - hit_x;
                  state->grab_offset_z = graphene_vec3_get_z (pos) - hit_z;
                }

              ct->paper_state = PAPER_GRABBED;
              ct->fall_time = 0;
              float grab_y = orig_y + GRAB_LIFT_HEIGHT;
              gthree_object_set_position_xyz (GTHREE_OBJECT (ct->group),
                                              graphene_vec3_get_x (pos),
                                              grab_y,
                                              graphene_vec3_get_z (pos));
              gtk_widget_queue_draw (state->area);
            }
          else
            {
              state->camera_dragging = TRUE;
              state->camera_drag_button = 1;
              state->drag_last_x = x;
              state->drag_last_y = y;
            }
        }
      else if (button == 3)
        {
          state->camera_dragging = TRUE;
          state->camera_drag_button = 3;
          state->drag_last_x = x;
          state->drag_last_y = y;
        }
      return;
    }

  handle_raycast_input (state, time);

  wlr_seat_pointer_notify_button (state->seat, time,
                                  gdk_to_linux_button (button),
                                  WL_POINTER_BUTTON_STATE_PRESSED);
  wlr_seat_pointer_notify_frame (state->seat);
}

static void
on_click_released (GtkGestureClick *gesture,
                   G_GNUC_UNUSED int n_press,
                   G_GNUC_UNUSED double x,
                   G_GNUC_UNUSED double y,
                   gpointer user_data)
{
  CompositorState *state = user_data;
  guint button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));
  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (gesture));
  uint32_t time = gdk_event_get_time (event);

  if (state->camera_dragging && (int)button == state->camera_drag_button)
    {
      state->camera_dragging = FALSE;
      return;
    }

  if (state->grabbed_toplevel && button == 1)
    {
      CompositorToplevel *ct = state->grabbed_toplevel;
      state->grabbed_toplevel = NULL;

      const graphene_vec3_t *pos =
        gthree_object_get_position (GTHREE_OBJECT (ct->group));
      ct->rest_x = graphene_vec3_get_x (pos);
      ct->rest_z = graphene_vec3_get_z (pos);
      ct->paper_state = PAPER_FALLING;
      ct->fall_time = 0;
      ct->fall_start_y = graphene_vec3_get_y (pos);
      gtk_widget_queue_draw (state->area);
      return;
    }

  if (state->meta_held)
    return;

  wlr_seat_pointer_notify_button (state->seat, time,
                                  gdk_to_linux_button (button),
                                  WL_POINTER_BUTTON_STATE_RELEASED);
  wlr_seat_pointer_notify_frame (state->seat);
}

static gboolean
on_scroll (GtkEventControllerScroll *controller,
           double dx, double dy,
           gpointer user_data)
{
  CompositorState *state = user_data;

  if (state->grabbed_toplevel && state->meta_held)
    {
      CompositorToplevel *ct = state->grabbed_toplevel;
      ct->rest_rotation_y += (float)dy * 5.0f;
      graphene_euler_t mesh_rot;
      graphene_euler_init (&mesh_rot, -90, ct->rest_rotation_y, 0);
      gthree_object_set_rotation (GTHREE_OBJECT (ct->group), &mesh_rot);
      gtk_widget_queue_draw (state->area);
      return TRUE;
    }

  if (state->meta_held)
    {
      state->cam_distance += (float)dy * CAM_ZOOM_SPEED;
      state->cam_distance = CLAMP (state->cam_distance, CAM_DISTANCE_MIN, CAM_DISTANCE_MAX);
      update_camera_from_spherical (state);
      gtk_widget_queue_draw (state->area);
      return TRUE;
    }

  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));
  uint32_t time = gdk_event_get_time (event);

  if (dy != 0)
    {
      wlr_seat_pointer_notify_axis (state->seat, time,
                                    WL_POINTER_AXIS_VERTICAL_SCROLL,
                                    dy * 15.0, (int32_t)(dy * 120),
                                    WL_POINTER_AXIS_SOURCE_WHEEL,
                                    WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    }
  if (dx != 0)
    {
      wlr_seat_pointer_notify_axis (state->seat, time,
                                    WL_POINTER_AXIS_HORIZONTAL_SCROLL,
                                    dx * 15.0, (int32_t)(dx * 120),
                                    WL_POINTER_AXIS_SOURCE_WHEEL,
                                    WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    }
  wlr_seat_pointer_notify_frame (state->seat);

  return TRUE;
}

static gboolean
is_meta_key (guint keyval)
{
  return keyval == GDK_KEY_Alt_L || keyval == GDK_KEY_Alt_R;
}

static gboolean
on_key_pressed (GtkEventControllerKey *controller,
                guint keyval, guint keycode,
                GdkModifierType mods,
                gpointer user_data)
{
  CompositorState *state = user_data;

  if (is_meta_key (keyval))
    {
      state->meta_held = TRUE;
      return TRUE;
    }

  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));
  uint32_t time = gdk_event_get_time (event);
  wlr_seat_keyboard_notify_key (state->seat, time, keycode - 8, WL_KEYBOARD_KEY_STATE_PRESSED);
  return TRUE;
}

static void
on_key_released (GtkEventControllerKey *controller,
                 guint keyval, guint keycode,
                 GdkModifierType mods,
                 gpointer user_data)
{
  CompositorState *state = user_data;

  if (is_meta_key (keyval))
    {
      state->meta_held = FALSE;
      return;
    }

  GdkEvent *event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));
  uint32_t time = gdk_event_get_time (event);
  wlr_seat_keyboard_notify_key (state->seat, time, keycode - 8, WL_KEYBOARD_KEY_STATE_RELEASED);
}

static void
on_key_modifiers (G_GNUC_UNUSED GtkEventControllerKey *controller,
                  GdkModifierType mods,
                  gpointer user_data)
{
  CompositorState *state = user_data;

  struct wlr_keyboard_modifiers wlr_mods = { 0 };

  if (mods & GDK_SHIFT_MASK)   wlr_mods.depressed |= (1 << 0);
  if (mods & GDK_LOCK_MASK)    wlr_mods.depressed |= (1 << 1);
  if (mods & GDK_CONTROL_MASK) wlr_mods.depressed |= (1 << 2);
  if (mods & GDK_ALT_MASK)     wlr_mods.depressed |= (1 << 3);

  wlr_mods.group = state->keyboard.modifiers.group;
  wlr_seat_keyboard_notify_modifiers (state->seat, &wlr_mods);
}

/* --- Smoothstep --- */

static float
smoothstep (float edge0, float edge1, float x)
{
  float t = CLAMP ((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

static void
set_bone_curl (CompositorToplevel *ct, float curl_height)
{
  float bone_spacing = ct->pixel_width / BONE_COUNT;
  float prev_h = 0;
  for (int i = 0; i < BONE_COUNT; i++)
    {
      float factor = 2.0f * i / (BONE_COUNT - 1) - 1.0f;
      float h = factor * factor * curl_height;
      float dz = h - prev_h;
      prev_h = h;
      if (i == 0)
        gthree_object_set_position_xyz (GTHREE_OBJECT (ct->bones[i]),
                                        -ct->pixel_width / 2.0f, 0, dz);
      else
        gthree_object_set_position_xyz (GTHREE_OBJECT (ct->bones[i]),
                                        bone_spacing, 0, dz);
    }
}

static gint
compare_toplevel_by_y (gconstpointer a, gconstpointer b)
{
  const CompositorToplevel *ta = a;
  const CompositorToplevel *tb = b;
  if (!ta->group || !tb->group)
    return (ta->group != NULL) - (tb->group != NULL);
  const graphene_vec3_t *pa = gthree_object_get_position (GTHREE_OBJECT (ta->group));
  const graphene_vec3_t *pb = gthree_object_get_position (GTHREE_OBJECT (tb->group));
  float ya = graphene_vec3_get_y (pa);
  float yb = graphene_vec3_get_y (pb);

  if (ya < yb) return -1;
  if (ya > yb) return 1;
  return 0;
}

/* --- Tick / frame callback --- */

static gboolean
tick_cb (GtkWidget *widget,
         GdkFrameClock *frame_clock,
         gpointer user_data)
{
  CompositorState *state = user_data;

  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  float dt = 0;
  if (state->last_frame_time != 0)
    dt = (frame_time - state->last_frame_time) / 1000000.0f;
  state->last_frame_time = frame_time;
  dt = CLAMP (dt, 0, 0.1f);

  gboolean needs_redraw = FALSE;

  if (state->soldier_mixer && dt > 0)
    {
      gthree_animation_mixer_update (state->soldier_mixer, dt);
      needs_redraw = TRUE;
    }

  /* Animate papers */
  for (GList *l = state->toplevels; l; l = l->next)
    {
      CompositorToplevel *ct = l->data;
      if (!ct->group)
        continue;

      if (ct->paper_state == PAPER_FALLING)
        {
          ct->fall_time += dt;

          float target_y = get_target_height_for_rect (state, ct,
                              ct->rest_x, ct->rest_z,
                              ct->fall_start_y,
                              ct->quad_width * 0.5f, ct->quad_height * 0.5f,
                              ct->rest_rotation_y);
          float fall_distance = fabsf (ct->fall_start_y - target_y);
          float fall_duration = fall_distance / FALL_SPEED;
          if (fall_duration < 0.1f) fall_duration = 0.1f;
          float t = ct->fall_time / fall_duration;
          float y = ct->fall_start_y + (target_y - ct->fall_start_y) * smoothstep (0, 1, t);

          gthree_object_set_position_xyz (GTHREE_OBJECT (ct->group),
                                          ct->rest_x, y, ct->rest_z);
          set_bone_curl (ct, ct->pixel_width * 0.04f);

          /* Yaw rotation */
          graphene_euler_t mesh_rot;
          float yaw_flutter = sinf (ct->fall_time * 3.0f + ct->bend_phase) * 5.0f;
          graphene_euler_init (&mesh_rot, -90,
                               ct->rest_rotation_y + yaw_flutter, 0);
          gthree_object_set_rotation (GTHREE_OBJECT (ct->group), &mesh_rot);

          if (t >= 1.0f)
            {
              ct->paper_state = PAPER_SETTLING;
              ct->settle_time = 0;
              gthree_object_set_position_xyz (GTHREE_OBJECT (ct->group),
                                              ct->rest_x, target_y, ct->rest_z);
              graphene_euler_init (&mesh_rot, -90, ct->rest_rotation_y, 0);
              gthree_object_set_rotation (GTHREE_OBJECT (ct->group), &mesh_rot);
            }

          needs_redraw = TRUE;
        }
      else if (ct->paper_state == PAPER_SETTLING)
        {
          ct->settle_time += dt;
          float st = ct->settle_time / SETTLE_DURATION;
          if (st >= 1.0f)
            {
              ct->paper_state = PAPER_RESTING;
              set_bone_curl (ct, 0);
            }
          else
            {
              float blend = 1.0f - smoothstep (0, 1, st);
              set_bone_curl (ct, ct->pixel_width * 0.04f * blend);
            }
          needs_redraw = TRUE;
        }
      else if (ct->paper_state == PAPER_GRABBED)
        {
          set_bone_curl (ct, ct->pixel_width * 0.04f);
          needs_redraw = TRUE;
        }
    }

  {
    GList *sorted = g_list_copy (state->toplevels);
    sorted = g_list_sort (sorted, compare_toplevel_by_y);
    int order = 0;
    for (GList *l = sorted; l; l = l->next)
      {
        CompositorToplevel *ct = l->data;
        if (!ct->group)
          continue;
        if (ct->mesh)
          gthree_object_set_render_order (GTHREE_OBJECT (ct->mesh), order++);
        for (GList *s = ct->subsurfaces; s; s = s->next)
          {
            CompositorSubsurface *sub = s->data;
            if (sub->mesh)
              gthree_object_set_render_order (GTHREE_OBJECT (sub->mesh), order++);
          }
        for (GList *p = ct->popups; p; p = p->next)
          {
            CompositorPopup *popup = p->data;
            if (popup->mesh)
              gthree_object_set_render_order (GTHREE_OBJECT (popup->mesh), order++);
            for (GList *s = popup->subsurfaces; s; s = s->next)
              {
                CompositorSubsurface *sub = s->data;
                if (sub->mesh)
                  gthree_object_set_render_order (GTHREE_OBJECT (sub->mesh), order++);
              }
          }
      }
    g_list_free (sorted);
  }

  if (needs_redraw)
    gtk_widget_queue_draw (widget);

  /* Send frame to Wayland output */
  wlr_output_send_frame (&state->output);

  /* Raycast for pointer tracking */
  struct timespec now;
  clock_gettime (CLOCK_MONOTONIC, &now);
  uint32_t time_msec = (uint32_t)(now.tv_sec * 1000 + now.tv_nsec / 1000000);

  if (!state->meta_held)
    handle_raycast_input (state, time_msec);

  /* Send frame_done for all mapped surfaces */
  for (GList *l = state->toplevels; l; l = l->next)
    {
      CompositorToplevel *ct = l->data;
      struct wlr_surface *surface = ct->xdg_toplevel->base->surface;
      if (surface->mapped)
        wlr_surface_send_frame_done (surface, &now);
      for (GList *p = ct->popups; p; p = p->next)
        {
          CompositorPopup *popup = p->data;
          struct wlr_surface *psurface = popup->xdg_popup->base->surface;
          if (psurface->mapped)
            wlr_surface_send_frame_done (psurface, &now);
          for (GList *ps = popup->subsurfaces; ps; ps = ps->next)
            {
              CompositorSubsurface *sub = ps->data;
              struct wlr_surface *ssurface = sub->wlr_subsurface->surface;
              if (ssurface->mapped)
                wlr_surface_send_frame_done (ssurface, &now);
            }
        }
      for (GList *s = ct->subsurfaces; s; s = s->next)
        {
          CompositorSubsurface *sub = s->data;
          struct wlr_surface *ssurface = sub->wlr_subsurface->surface;
          if (ssurface->mapped)
            wlr_surface_send_frame_done (ssurface, &now);
        }
    }

  return G_SOURCE_CONTINUE;
}

/* --- Scene setup --- */

static gboolean
set_receive_shadow_cb (GthreeObject *object, G_GNUC_UNUSED gpointer data)
{
  if (GTHREE_IS_MESH (object))
    gthree_object_set_receive_shadow (object, TRUE);
  return TRUE;
}

static gboolean
set_cast_shadow_cb (GthreeObject *object, G_GNUC_UNUSED gpointer data)
{
  if (GTHREE_IS_MESH (object))
    gthree_object_set_cast_shadow (object, TRUE);
  return TRUE;
}

static const char *
get_examples_data_dir (void)
{
  const char *dir = g_getenv ("GTHREE_EXAMPLES_DIR");
  if (dir)
    return dir;

  static char *data_dir;
  if (!data_dir)
    {
      g_autofree char *self = g_file_read_link ("/proc/self/exe", NULL);
      if (self)
        {
          g_autofree char *bin_dir = g_path_get_dirname (self);
          data_dir = g_build_filename (bin_dir, "..", "examples", NULL);
          if (g_file_test (data_dir, G_FILE_TEST_IS_DIR))
            return data_dir;
          g_free (data_dir);
        }
      data_dir = g_strdup (".");
    }
  return data_dir;
}

static GthreeObject *
load_glb_model (const char *filename, GthreeLoader **out_loader)
{
  g_autoptr (GBytes) bytes = NULL;
  g_autofree char *res_path =
    g_build_filename ("/org/gnome/gthree-examples/models/", filename, NULL);
  bytes = g_resources_lookup_data (res_path, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

  g_autofree char *file_path = NULL;
  if (!bytes)
    {
      char *data;
      gsize size;
      g_autoptr (GError) error = NULL;
      file_path = g_build_filename (get_examples_data_dir (), "models", filename, NULL);
      if (!g_file_get_contents (file_path, &data, &size, &error))
        {
          g_warning ("Failed to load %s: %s", file_path, error->message);
          return NULL;
        }
      bytes = g_bytes_new_take (data, size);
    }

  g_autoptr (GError) error = NULL;
  GthreeLoader *loader = gthree_loader_parse_gltf (bytes, NULL, &error);
  if (!loader)
    {
      g_warning ("Failed to parse %s: %s", filename, error->message);
      return NULL;
    }

  GthreeScene *loader_scene = gthree_loader_get_scene (loader, 0);
  if (!loader_scene)
    {
      g_warning ("No scene in %s", filename);
      g_object_unref (loader);
      return NULL;
    }

  if (out_loader)
    *out_loader = loader;
  else
    g_object_unref (loader);

  return GTHREE_OBJECT (g_object_ref (loader_scene));
}

static void
reparent_children (GthreeObject *from, GthreeObject *to)
{
  GList *children = NULL;
  GthreeObjectIter it;
  GthreeObject *ch;
  gthree_object_iter_init (&it, from);
  while (gthree_object_iter_next (&it, &ch))
    children = g_list_prepend (children, g_object_ref (ch));
  for (GList *l = children; l; l = l->next)
    {
      gthree_object_remove_child (from, l->data);
      gthree_object_add_child (to, l->data);
      g_object_unref (l->data);
    }
  g_list_free (children);
}

static GthreeScene *
create_scene (CompositorState *state)
{
  GthreeScene *scene = gthree_scene_new ();

  graphene_vec3_t bg_color;
  graphene_vec3_init (&bg_color, 0.85f, 0.85f, 0.82f);
  gthree_scene_set_background_color (scene, &bg_color);

  /* Neutral environment map for PBR materials */
  guchar env_pixel[] = { 200, 200, 195, 255 };
  GBytes *env_face = g_bytes_new_static (env_pixel, 4);
  GBytes *env_faces[6] = { env_face, env_face, env_face,
                           env_face, env_face, env_face };
  g_autoptr (GthreeCubeTexture) env_cube =
    gthree_cube_texture_new_from_bytes (env_faces, 1, 4, GTHREE_MEMORY_FORMAT_R8G8B8A8);
  g_bytes_unref (env_face);
  gthree_scene_set_environment (scene, GTHREE_TEXTURE (env_cube));

  graphene_vec3_t ambient_color;
  graphene_vec3_init (&ambient_color, 0.7f, 0.7f, 0.7f);
  g_autoptr (GthreeAmbientLight) ambient = gthree_ambient_light_new (&ambient_color);
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), 1.0f * G_PI);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient));

  graphene_vec3_t light_color;
  graphene_vec3_init (&light_color, 1.0f, 1.0f, 0.95f);
  g_autoptr (GthreeDirectionalLight) dir_light =
    gthree_directional_light_new (&light_color, 2.0f * G_PI);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir_light), 1000, 2500, 1500);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (dir_light), TRUE);

  GthreeLightShadow *shadow = gthree_light_get_shadow (GTHREE_LIGHT (dir_light));
  gthree_light_shadow_set_bias (shadow, -0.0005f);
  gthree_light_shadow_set_map_size (shadow, 2048, 2048);
  GthreeCamera *shadow_cam = gthree_light_shadow_get_camera (shadow);
  gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), -2500);
  gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), 2500);
  gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), 2500);
  gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), -2500);
  gthree_camera_set_near (shadow_cam, 1);
  gthree_camera_set_far (shadow_cam, 7500);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (dir_light));

  /* Load floor model — raw model is 256x256, scale 10x */
  state->floor_model = load_glb_model ("checkered_tile_floor.glb", &state->floor_loader);
  if (state->floor_model)
    {
      GthreeGroup *floor_group = gthree_group_new ();
      set_uniform_scale (GTHREE_OBJECT (floor_group), FLOOR_SCALE);
      reparent_children (state->floor_model, GTHREE_OBJECT (floor_group));

      g_clear_object (&state->floor_model);
      state->floor_model = GTHREE_OBJECT (g_object_ref (floor_group));
      gthree_object_traverse (state->floor_model, set_receive_shadow_cb, NULL);
      gthree_object_add_child (GTHREE_OBJECT (scene), state->floor_model);
    }

  float desk_scale_factor = DESK_SCALE;
  state->desk_model = load_glb_model ("wooden_writing_desk_with_props.glb", &state->desk_loader);
  if (state->desk_model)
    {
      GthreeGroup *desk_group = gthree_group_new ();
      set_uniform_scale (GTHREE_OBJECT (desk_group), desk_scale_factor);
      reparent_children (state->desk_model, GTHREE_OBJECT (desk_group));

      g_clear_object (&state->desk_model);
      state->desk_model = GTHREE_OBJECT (g_object_ref (desk_group));

      gthree_object_traverse (state->desk_model, set_receive_shadow_cb, NULL);
      gthree_object_traverse (state->desk_model, set_cast_shadow_cb, NULL);
      gthree_object_add_child (GTHREE_OBJECT (scene), state->desk_model);

      /* The model's internal root has a 1.25x scale and a Y/Z axis swap,
         so effective scale is desk_scale_factor * 1.25 = 100.
         Local mesh coords are ~[-1,1], so world bounds are ~[-100,100].
         The Y/Z swap means local Y becomes world -Z and local Z becomes
         world Y.  Approximate world-space bounds: */
      float s = desk_scale_factor * 1.25f;
      float bx0 = -1.0f * s, bx1 = 1.0f * s;
      float by0 = -0.05f * s, by1 = 1.2f * s;
      float bz0 = -1.0f * s, bz1 = 1.0f * s;

      if (state->debug)
        g_print ("DESK approx bounds: (%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f)\n",
                 bx0, by0, bz0, bx1, by1, bz1);

      float margin = 50.0f;
      state->hm_x_min = bx0 - margin;
      state->hm_x_max = bx1 + margin;
      state->hm_z_min = bz0 - margin;
      state->hm_z_max = bz1 + margin;
      state->hm_y_min = by0;
      state->hm_y_max = by1 + 50.0f;

      graphene_vec3_init (&state->cam_target,
                          bx0 + (bx1 - bx0) * 0.5,
                          by0 + (by1 - by0) * 0.8,
                          bz0 + (bz1 - bz0) * 0.6);
    }
  else
    {
      state->hm_x_min = -300; state->hm_x_max = 300;
      state->hm_z_min = -200; state->hm_z_max = 200;
      state->hm_y_min = 0; state->hm_y_max = 200;
      graphene_vec3_init (&state->cam_target, 0, 50, 0);
    }

  /* Soldier model */
  {
    GthreeObject *soldier_scene = load_glb_model ("Soldier.glb", &state->soldier_loader);
    if (soldier_scene)
      {
        float soldier_scale = 560.0f;
        GthreeGroup *soldier_group = gthree_group_new ();
        set_uniform_scale (GTHREE_OBJECT (soldier_group), soldier_scale);
        gthree_object_set_position_xyz (GTHREE_OBJECT (soldier_group), -100, 0, -700);
        graphene_euler_t soldier_rot;
        graphene_euler_init (&soldier_rot, 0, 180, 0);
        gthree_object_set_rotation (GTHREE_OBJECT (soldier_group), &soldier_rot);
        reparent_children (soldier_scene, GTHREE_OBJECT (soldier_group));
        g_object_unref (soldier_scene);

        state->soldier_model = GTHREE_OBJECT (g_object_ref (soldier_group));
        gthree_object_traverse (state->soldier_model, set_cast_shadow_cb, NULL);
        gthree_object_add_child (GTHREE_OBJECT (scene), state->soldier_model);

        state->soldier_mixer = gthree_animation_mixer_new (state->soldier_model);
        int n_anims = gthree_loader_get_n_animations (state->soldier_loader);
        if (n_anims > 0)
          {
            GthreeAnimationAction *idle_action =
              gthree_animation_mixer_clip_action (state->soldier_mixer,
                                                  gthree_loader_get_animation (state->soldier_loader, 0),
                                                  NULL);
            gthree_animation_action_set_loop_mode (idle_action, GTHREE_LOOP_MODE_REPEAT, -1);
            gthree_animation_action_play (idle_action);
          }
      }
  }

  state->cam_theta = 0;
  state->cam_phi = G_PI / 4.0f;
  state->cam_distance = 500.0f;

  return scene;
}

static void
resize_cb (GthreeArea *area, int width, int height, gpointer user_data)
{
  CompositorState *state = user_data;
  gthree_perspective_camera_set_aspect (state->camera, (float)width / (float)height);
}

/* --- Keyboard init (after widget is realized) --- */

static void
keyboard_init (CompositorState *state)
{
  GdkDisplay *gdisplay = gtk_widget_get_display (state->area);
  GdkSeat *gseat = gdk_display_get_default_seat (gdisplay);
  GdkDevice *gkeyboard = gdk_seat_get_keyboard (gseat);

  g_signal_connect (gkeyboard, "notify", G_CALLBACK (on_gdk_keyboard_notify), state);

  struct xkb_keymap *keymap = get_xkb_keymap (gkeyboard);
  wlr_keyboard_set_keymap (&state->keyboard, keymap);
  xkb_keymap_unref (keymap);
}

/* --- Main --- */

static void
render_heightmap (CompositorState *state)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (GTHREE_AREA (state->area));
  gtk_gl_area_make_current (GTK_GL_AREA (state->area));

  /* We need the world matrices up to date for the desk model so
     get_mesh_extents returns correct results */
  gthree_object_update_matrix_world (GTHREE_OBJECT (state->scene), TRUE);

  if (state->desk_model)
    {
      graphene_box_t bbox;
      gthree_object_get_mesh_extents (state->desk_model, &bbox);
      graphene_point3d_t bmin, bmax;
      graphene_box_get_min (&bbox, &bmin);
      graphene_box_get_max (&bbox, &bmax);

      if (state->debug)
        g_print ("DESK real extents: (%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f)\n",
                 bmin.x, bmin.y, bmin.z, bmax.x, bmax.y, bmax.z);

      float margin = 50.0f;
      state->hm_x_min = bmin.x - margin;
      state->hm_x_max = bmax.x + margin;
      state->hm_z_min = bmin.z - margin;
      state->hm_z_max = bmax.z + margin;
      state->hm_y_min = bmin.y;
      state->hm_y_max = bmax.y + 50.0f;

      GthreeObject *table_node = gthree_object_find_first_by_name (state->desk_model, "Table_17");
      if (table_node)
        {
          graphene_box_t table_bbox;
          gthree_object_get_mesh_extents (table_node, &table_bbox);
          graphene_point3d_t tmin, tmax;
          graphene_box_get_min (&table_bbox, &tmin);
          graphene_box_get_max (&table_bbox, &tmax);
          state->desk_x_min = tmin.x;
          state->desk_x_max = tmax.x;
          state->desk_z_min = tmin.z;
          state->desk_z_max = tmax.z;
          if (state->debug)
            g_print ("TABLE top bounds: x=[%.1f, %.1f] z=[%.1f, %.1f]\n",
                     tmin.x, tmax.x, tmin.z, tmax.z);
        }
      else
        {
          state->desk_x_min = bmin.x;
          state->desk_x_max = bmax.x;
          state->desk_z_min = bmin.z;
          state->desk_z_max = bmax.z;
        }
    }

  /* Render a depth pass from above to build the heightmap */
  float hm_half_x = (state->hm_x_max - state->hm_x_min) / 2.0f;
  float hm_half_z = (state->hm_z_max - state->hm_z_min) / 2.0f;
  state->hm_cam_y = state->hm_y_max + 50.0f;
  state->hm_near = 0.1f;
  state->hm_far = state->hm_cam_y - state->hm_y_min + 100.0f;
  GthreeOrthographicCamera *ortho_cam =
    gthree_orthographic_camera_new (-hm_half_x, hm_half_x,
                                    hm_half_z, -hm_half_z,
                                    state->hm_near, state->hm_far);

  float cx = (state->hm_x_min + state->hm_x_max) / 2.0f;
  float cz = (state->hm_z_min + state->hm_z_max) / 2.0f;
  gthree_object_add_child (GTHREE_OBJECT (state->scene), GTHREE_OBJECT (ortho_cam));
  gthree_object_set_position_xyz (GTHREE_OBJECT (ortho_cam),
                                  cx, state->hm_cam_y, cz);
  graphene_euler_t rot;
  gthree_object_set_rotation (GTHREE_OBJECT (ortho_cam),
                              graphene_euler_init (&rot, -90, 0, 0));

  g_autoptr (GthreeMeshDepthMaterial) depth_mat = gthree_mesh_depth_material_new ();
  gthree_mesh_depth_material_set_depth_packing_format (depth_mat,
                                                        GTHREE_DEPTH_PACKING_FORMAT_RGBA);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (depth_mat),
                                  GTHREE_BLEND_NO,
                                  GL_FUNC_ADD,
                                  GL_SRC_ALPHA,
                                  GL_ONE_MINUS_SRC_ALPHA);

  /* Hide window meshes during depth pass */
  GList *hidden = NULL;
  for (GList *l = state->toplevels; l; l = l->next)
    {
      CompositorToplevel *ct = l->data;
      if (ct->group && gthree_object_get_visible (GTHREE_OBJECT (ct->group)))
        {
          gthree_object_set_visible (GTHREE_OBJECT (ct->group), FALSE);
          hidden = g_list_prepend (hidden, ct);
        }
    }

  gthree_scene_set_override_material (state->scene, GTHREE_MATERIAL (depth_mat));

  GthreeRenderTarget *target = gthree_render_target_new (HEIGHTMAP_SIZE, HEIGHTMAP_SIZE);
  gthree_render_target_set_stencil_buffer (target, FALSE);
  gthree_renderer_set_render_target (renderer, target, 0, 0);
  gthree_renderer_render (renderer, state->scene, GTHREE_CAMERA (ortho_cam));

  /* Download while the render target FBO is still bound */
  state->heightmap_data = g_malloc (HEIGHTMAP_SIZE * HEIGHTMAP_SIZE * 4);
  gthree_render_target_download (target, renderer,
                                 state->heightmap_data, HEIGHTMAP_SIZE * 4, TRUE);

  gthree_renderer_set_render_target (renderer, NULL, 0, 0);
  g_object_unref (target);
  gthree_scene_set_override_material (state->scene, NULL);
  gthree_object_remove_child (GTHREE_OBJECT (state->scene), GTHREE_OBJECT (ortho_cam));
  g_object_unref (ortho_cam);

  for (GList *l = hidden; l; l = l->next)
    {
      CompositorToplevel *ct = l->data;
      gthree_object_set_visible (GTHREE_OBJECT (ct->group), TRUE);
    }
  g_list_free (hidden);

  if (state->debug)
    g_print ("HEIGHTMAP rendered: %dx%d y_range=[%.0f, %.0f] near=%.1f far=%.1f\n",
             HEIGHTMAP_SIZE, HEIGHTMAP_SIZE, state->hm_y_min, state->hm_y_max,
             state->hm_near, state->hm_far);
}

static void
on_realize (GtkWidget *widget, gpointer user_data)
{
  CompositorState *state = user_data;
  keyboard_init (state);

  GthreeRenderer *renderer = gthree_area_get_renderer (GTHREE_AREA (state->area));
  gthree_renderer_set_shadow_map_enabled (renderer, TRUE);

  render_heightmap (state);

  update_camera_from_spherical (state);
}

int
main (int argc, char *argv[])
{
  gboolean no_dmabuf = FALSE;
  gboolean debug = FALSE;
  GOptionEntry entries[] = {
    { "no-dmabuf", 0, 0, G_OPTION_ARG_NONE, &no_dmabuf, "Disable DMABUF, use SHM only", NULL },
    { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "Log compositor protocol events", NULL },
    { NULL }
  };

  g_autoptr (GOptionContext) ctx = g_option_context_new ("[-- COMMAND [ARGS...]]");
  g_option_context_set_summary (ctx, "Launch a Wayland client as the initial program");
  g_option_context_add_main_entries (ctx, entries, NULL);
  g_autoptr (GError) opt_error = NULL;
  if (!g_option_context_parse (ctx, &argc, &argv, &opt_error))
    {
      g_printerr ("%s\n", opt_error->message);
      return EXIT_FAILURE;
    }

  gtk_init ();

  CompositorState *state = &global_state;
  state->no_dmabuf = no_dmabuf;
  state->debug = debug;

  compositor_wlr_init (state);

  /* Window */
  GtkWidget *window = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Gthree Wayland Compositor");
  gtk_window_set_default_size (GTK_WINDOW (window), 1024, 768);

  /* Scene and camera */
  state->scene = create_scene (state);
  state->camera = gthree_perspective_camera_new (50, 1.33f, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (state->scene), GTHREE_OBJECT (state->camera));

  /* GthreeArea */
  state->area = gthree_area_new (state->scene, GTHREE_CAMERA (state->camera));
  gtk_widget_set_hexpand (state->area, TRUE);
  gtk_widget_set_vexpand (state->area, TRUE);
  gtk_widget_set_focusable (state->area, TRUE);

  g_signal_connect (state->area, "resize", G_CALLBACK (resize_cb), state);
  g_signal_connect (state->area, "realize", G_CALLBACK (on_realize), state);

  /* Input controllers */
  GtkEventController *motion = gtk_event_controller_motion_new ();
  g_signal_connect (motion, "motion", G_CALLBACK (on_motion), state);
  g_signal_connect (motion, "leave", G_CALLBACK (on_motion_leave), state);
  gtk_widget_add_controller (state->area, motion);

  GtkGesture *click = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), 0);
  g_signal_connect (click, "pressed", G_CALLBACK (on_click_pressed), state);
  g_signal_connect (click, "released", G_CALLBACK (on_click_released), state);
  gtk_widget_add_controller (state->area, GTK_EVENT_CONTROLLER (click));

  GtkEventController *scroll = gtk_event_controller_scroll_new (
    GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
  g_signal_connect (scroll, "scroll", G_CALLBACK (on_scroll), state);
  gtk_widget_add_controller (state->area, scroll);

  GtkEventController *key = gtk_event_controller_key_new ();
  g_signal_connect (key, "key-pressed", G_CALLBACK (on_key_pressed), state);
  g_signal_connect (key, "key-released", G_CALLBACK (on_key_released), state);
  g_signal_connect (key, "modifiers", G_CALLBACK (on_key_modifiers), state);
  gtk_widget_add_controller (state->area, key);

  /* Tick callback for frame timing and animation */
  gtk_widget_add_tick_callback (state->area, tick_cb, state, NULL);

  gtk_window_set_child (GTK_WINDOW (window), state->area);
  gtk_window_present (GTK_WINDOW (window));

  if (argc > 1)
    {
      GPid child_pid;
      g_auto (GStrv) merged_env = g_get_environ ();
      merged_env = g_environ_setenv (merged_env, "WAYLAND_DISPLAY", state->socket, TRUE);
      merged_env = g_environ_setenv (merged_env, "GDK_BACKEND", "wayland", TRUE);
      merged_env = g_environ_unsetenv (merged_env, "GDK_SCALE");
      merged_env = g_environ_unsetenv (merged_env, "GDK_DPI_SCALE");
      merged_env = g_environ_unsetenv (merged_env, "QT_SCALE_FACTOR");
      merged_env = g_environ_unsetenv (merged_env, "QT_SCREEN_SCALE_FACTORS");
      merged_env = g_environ_unsetenv (merged_env, "DISPLAY");
      g_autoptr (GError) error = NULL;

      if (!g_spawn_async (NULL, argv + 1, merged_env,
                          G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                          NULL, NULL, &child_pid, &error))
        g_warning ("Failed to launch client: %s", error->message);
      else
        {
          g_child_watch_add (child_pid, (GChildWatchFunc)g_spawn_close_pid, NULL);
          g_print ("Launched client (pid %d): %s\n", (int)child_pid, argv[1]);
        }
    }
  else
    {
      g_print ("Launch a Wayland client with:\n");
      g_print ("  WAYLAND_DISPLAY=%s GDK_BACKEND=wayland <command>\n", state->socket);
    }

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  /* Cleanup */
  g_source_destroy (state->wl_source);
  g_source_unref (state->wl_source);
  wl_list_remove (&state->new_xdg_toplevel.link);
  wl_list_remove (&state->new_xdg_popup.link);
  wl_list_remove (&state->request_activate.link);
  wl_list_remove (&state->request_set_shape.link);
  wl_list_remove (&state->request_set_selection.link);
  wl_list_remove (&state->output_bind.link);
  wl_display_destroy (state->wl_display);
  g_free (state->socket);
  g_free (state->heightmap_data);
  g_clear_object (&state->desk_model);
  g_clear_object (&state->floor_model);
  g_clear_object (&state->desk_loader);
  g_clear_object (&state->floor_loader);
  g_clear_object (&state->soldier_model);
  g_clear_object (&state->soldier_loader);
  g_clear_object (&state->soldier_mixer);

  return EXIT_SUCCESS;
}
