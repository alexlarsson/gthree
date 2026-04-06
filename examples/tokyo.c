#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include <gthree/gthreearea.h>
#include "utils.h"
#include "orbitcontrols.h"

static GthreeScene *scene;
static GthreeScene *sky_scene;
static GthreeObject *model;
static GthreePerspectiveCamera *camera;
static GthreeOrbitControls *orbit;
static GthreeAnimationMixer *mixer;
static GthreeLoader *loader;
static GthreeSky *sky;
static GthreeCubeCamera *cube_camera;
static GthreeRenderTarget *cube_render_target;
static gboolean tone_mapping_set = FALSE;
static float aspect = 1.0;

static graphene_sphere_t *
box_get_bounding_sphere (const graphene_box_t *box,
                         graphene_sphere_t    *sphere)
{
  graphene_vec3_t size, center;
  float radius;
  graphene_point3d_t min;

  graphene_box_get_size (box, &size);
  radius = graphene_vec3_length (&size) * 0.5f;
  graphene_vec3_scale (&size, 0.5f, &center);
  graphene_box_get_min (box, &min);
  min.x += graphene_vec3_get_x (&center);
  min.y += graphene_vec3_get_y (&center);
  min.z += graphene_vec3_get_z (&center);

  return graphene_sphere_init (sphere, &min, radius);
}

static gboolean
render_area (GtkGLArea    *gl_area,
             GdkGLContext *context)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (GTHREE_AREA (gl_area));

  if (!tone_mapping_set)
    {
      gthree_renderer_set_tone_mapping (renderer, GTHREE_TONE_MAPPING_ACES_FILMIC);
      tone_mapping_set = TRUE;

      gthree_cube_camera_update (cube_camera, renderer, sky_scene);

      gthree_scene_set_background_texture (scene,
                                           gthree_render_target_get_texture (cube_render_target));
      gthree_scene_set_environment (scene,
                                    gthree_render_target_get_texture (cube_render_target));
    }

  gthree_renderer_set_render_target (renderer, NULL, 0, 0);
  gthree_renderer_render (renderer, scene, GTHREE_CAMERA (camera));

  return TRUE;
}

static gboolean
tick (GtkWidget     *widget,
     GdkFrameClock *frame_clock,
     gpointer       user_data)
{
  static gint64 last_frame_time_i = 0;
  gint64 frame_time_i;

  frame_time_i = gdk_frame_clock_get_frame_time (frame_clock);
  if (last_frame_time_i != 0)
    {
      float delta_time_sec = (frame_time_i - last_frame_time_i) / (float) G_USEC_PER_SEC;
      gthree_animation_mixer_update (mixer, delta_time_sec);
    }
  last_frame_time_i = frame_time_i;

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height)
{
  aspect = (float)width / (float)(height);
  gthree_perspective_camera_set_aspect (camera, aspect);
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area;
  gboolean done = FALSE;
  GthreeAnimationClip *clip;
  GthreeAnimationAction *action;
  graphene_vec3_t vec;
  GthreeScene *gltf_scene;
  graphene_box_t bounding_box;
  graphene_sphere_t bounding_sphere;
  graphene_point3d_t scene_center;
  float scene_radius;

  window = examples_init ("Littlest Tokyo", &box, &done);

  area = gthree_area_new (NULL, NULL);
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);
  g_signal_connect (area, "render", G_CALLBACK (render_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);

  loader = examples_load_gltl ("LittlestTokyo.glb");
  gltf_scene = gthree_loader_get_scene (loader, 0);

  scene = gthree_scene_new ();

  model = GTHREE_OBJECT (gltf_scene);
  gthree_object_set_position_xyz (model, 1, 1, 0);
  gthree_object_set_scale_uniform (model, 0.01);
  gthree_object_add_child (GTHREE_OBJECT (scene), model);

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), TRUE);
  gthree_object_get_mesh_extents (GTHREE_OBJECT (scene), &bounding_box);
  box_get_bounding_sphere (&bounding_box, &bounding_sphere);
  graphene_sphere_get_center (&bounding_sphere, &scene_center);
  scene_radius = graphene_sphere_get_radius (&bounding_sphere);

  sky_scene = gthree_scene_new ();
  sky = gthree_sky_new ();
  gthree_sky_set_turbidity (sky, 0);
  gthree_sky_set_rayleigh (sky, 3);
  gthree_sky_set_mie_directional_g (sky, 0.7);
  graphene_vec3_init (&vec, -0.8, 0.19, 0.56);
  gthree_sky_set_sun_position (sky, &vec);
  gthree_object_set_scale_uniform (GTHREE_OBJECT (sky), 450000);
  gthree_object_add_child (GTHREE_OBJECT (sky_scene), GTHREE_OBJECT (sky));

  cube_render_target = gthree_render_target_new_cube (256);
  cube_camera = gthree_cube_camera_new (1, 500000, cube_render_target);
  gthree_object_add_child (GTHREE_OBJECT (sky_scene), GTHREE_OBJECT (cube_camera));

  camera = gthree_perspective_camera_new (40, aspect,
                                          scene_radius / 100,
                                          scene_radius * 100);
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  scene_center.x,
                                  scene_center.y + scene_radius * 0.5,
                                  scene_center.z + scene_radius * 2.5);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  orbit = gthree_orbit_controls_new (GTHREE_OBJECT (camera), GTK_WIDGET (area));
  gthree_orbit_controls_set_enable_damping (orbit, TRUE);
  graphene_vec3_init (&vec, scene_center.x, scene_center.y, scene_center.z);
  gthree_orbit_controls_set_target (orbit, &vec);

  mixer = gthree_animation_mixer_new (model);
  if (gthree_loader_get_n_animations (loader) > 0)
    {
      clip = gthree_loader_get_animation (loader, 0);
      action = gthree_animation_mixer_clip_action (mixer, clip, NULL);
      gthree_animation_action_set_loop_mode (action, GTHREE_LOOP_MODE_REPEAT, -1);
      gthree_animation_action_play (action);
    }

  gthree_area_set_scene (GTHREE_AREA (area), scene);
  gthree_area_set_camera (GTHREE_AREA (area), GTHREE_CAMERA (camera));

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
