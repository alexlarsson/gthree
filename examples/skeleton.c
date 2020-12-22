#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

static GthreeAnimationMixer *mixer;
static GthreeAnimationAction *idle_action;
static GthreeAnimationAction *walk_action;
static GthreeAnimationAction *run_action;
static GthreeLoader *loader = NULL;
static GthreeSkeletonHelper *skeleton;

static float setting_idle_weight = 0.0;
static float setting_walk_weight = 1.0;
static float setting_run_weight = 0.0;

static GtkWidget *idle_scale, *walk_scale, *run_scale, *time_scale;
static GtkWidget *idle_to_walk_button, *walk_to_idle_button, *run_to_walk_button, *walk_to_run_button;


static void
set_weight (GthreeAnimationAction *action, float weight)
{
  gthree_animation_action_set_enabled (action, TRUE);
  gthree_animation_action_set_effective_time_scale (action, 1.0);
  gthree_animation_action_set_effective_weight (action, weight);
}

static void
activate_all_actions (void)
{
  set_weight (idle_action, setting_idle_weight);
  gthree_animation_action_play (idle_action);
  set_weight (walk_action, setting_walk_weight);
  gthree_animation_action_play (walk_action);
  set_weight (run_action, setting_run_weight);
  gthree_animation_action_play (run_action);
}

static gboolean in_update;
static void
update_ui (void)
{
  in_update = TRUE;
  setting_idle_weight = gthree_animation_action_get_effective_weight (idle_action);
  gtk_range_set_value (GTK_RANGE (idle_scale), setting_idle_weight);

  setting_walk_weight = gthree_animation_action_get_effective_weight (walk_action);
  gtk_range_set_value (GTK_RANGE (walk_scale), setting_walk_weight);

  setting_run_weight = gthree_animation_action_get_effective_weight (run_action);
  gtk_range_set_value (GTK_RANGE (run_scale), setting_run_weight);

  gtk_widget_set_sensitive (idle_to_walk_button,
                            setting_idle_weight == 1 && setting_walk_weight == 0 && setting_run_weight == 0);
  gtk_widget_set_sensitive (walk_to_idle_button,
                            setting_idle_weight == 0 && setting_walk_weight == 1 && setting_run_weight == 0);
  gtk_widget_set_sensitive (walk_to_run_button,
                            setting_idle_weight == 0 && setting_walk_weight == 1 && setting_run_weight == 0);
  gtk_widget_set_sensitive (run_to_walk_button,
                            setting_idle_weight == 0 && setting_walk_weight == 0 && setting_run_weight == 1);

  in_update = FALSE;
}

static gboolean
set_cast_shadow (GthreeObject                *object,
                 gpointer                     user_data)
{
  if (GTHREE_IS_MESH (object))
    gthree_object_set_cast_shadow (object, TRUE);
  return TRUE;
}

static GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  g_autoptr(GthreeHemisphereLight) hemi_light = NULL;
  g_autoptr(GthreeDirectionalLight) dir_light = NULL;
  g_autoptr(GthreeGeometry) ground_geometry = NULL;
  g_autoptr(GthreeMeshPhongMaterial) ground_material = NULL;
  g_autoptr(GthreeMesh) ground = NULL;
  graphene_vec3_t color;
  graphene_euler_t rotation;
  GthreeLightShadow *shadow;
  GthreeCamera *shadow_camera;
  GthreeScene *loader_scene;
  g_autoptr(GthreeFog) fog = NULL;

  scene = gthree_scene_new ();

  gthree_scene_set_background_color (scene,
                                     graphene_vec3_init (&color, 0.63, 0.63, 0.63));

  fog = gthree_fog_new_linear (graphene_vec3_init (&color, 0.63, 0.63, 0.63), 10, 50);
  gthree_scene_set_fog (scene, fog);

  hemi_light = gthree_hemisphere_light_new (white (),
                                            graphene_vec3_init (&color, 0.25, 0.25, 0.25), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (hemi_light), 0, 20, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (hemi_light));

  dir_light = gthree_directional_light_new (white (), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir_light), -3, 10, -10);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (dir_light), TRUE);

  shadow = gthree_light_get_shadow (GTHREE_LIGHT (dir_light));
  shadow_camera = gthree_light_shadow_get_camera (shadow);

  gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 2);
  gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -2);
  gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -2);
  gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 2);
  gthree_camera_set_near (shadow_camera, 0.1);
  gthree_camera_set_far (shadow_camera, 40);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (dir_light));

  // TODO:
  // scene.add( new THREE.CameraHelper( light.shadow.camera ) );

  // ground
  ground_geometry = gthree_geometry_new_plane (100, 100, 1, 1);
  ground_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (ground_material,
                                        graphene_vec3_init (&color, 0.6, 0.6, 0.6));
  gthree_material_set_depth_write (GTHREE_MATERIAL (ground_material), FALSE);
  ground = gthree_mesh_new (ground_geometry, GTHREE_MATERIAL (ground_material));

  gthree_object_set_rotation (GTHREE_OBJECT (ground),
                              graphene_euler_init (&rotation, -90, 0, 0));
  gthree_object_set_receive_shadow (GTHREE_OBJECT (ground), TRUE);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ground));

  loader = examples_load_gltl ("Soldier.glb");

  loader_scene = gthree_loader_get_scene (loader, 0);
  gthree_object_traverse (GTHREE_OBJECT (loader_scene), set_cast_shadow, NULL);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (loader_scene));

  mixer = gthree_animation_mixer_new (GTHREE_OBJECT (loader_scene));
  idle_action = gthree_animation_mixer_clip_action (mixer, gthree_loader_get_animation (loader, 0), NULL);
  walk_action = gthree_animation_mixer_clip_action (mixer, gthree_loader_get_animation (loader, 3), NULL);
  run_action = gthree_animation_mixer_clip_action (mixer, gthree_loader_get_animation (loader, 1), NULL);

  activate_all_actions ();

  skeleton = gthree_skeleton_helper_new (GTHREE_OBJECT (loader_scene));
  gthree_line_basic_material_set_line_width (GTHREE_LINE_BASIC_MATERIAL (gthree_line_get_material (GTHREE_LINE (skeleton))), 3);
  gthree_object_set_visible (GTHREE_OBJECT (skeleton), FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (skeleton));

  return scene;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height,
             GthreePerspectiveCamera *camera)
{
  gthree_perspective_camera_set_aspect (camera, (float)width / (float)(height));
}

static void
show_skeleton_toggled (GtkToggleButton *toggle_button,
                       GtkWidget *area)
{
  gboolean show_skeleton;

#ifdef USE_GTK4
  show_skeleton = gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle_button));
#else
  show_skeleton = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));
#endif

  gthree_object_set_visible (GTHREE_OBJECT (skeleton), show_skeleton);

  gtk_widget_queue_draw (area);
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 last_frame_time = 0;
  gint64 frame_time;
  float delta_time_sec;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (last_frame_time != 0)
    {
      delta_time_sec = (frame_time - last_frame_time) / (float) G_USEC_PER_SEC;
      gthree_animation_mixer_update (mixer, delta_time_sec);
    }

  last_frame_time = frame_time;

  update_ui ();

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
realize_area (GthreeArea *area)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (area);

  gthree_renderer_set_shadow_map_enabled (renderer, TRUE);
}

static void
idle_weight_changed  (GtkRange *range,
                     GtkWidget *area)
{
  if (in_update)
    return;
  setting_idle_weight = gtk_range_get_value (range);
  set_weight (idle_action, setting_idle_weight);
}

static void
walk_weight_changed  (GtkRange *range,
                     GtkWidget *area)
{
  if (in_update)
    return;
  setting_walk_weight = gtk_range_get_value (range);
  set_weight (walk_action, setting_walk_weight);
}

static void
run_weight_changed  (GtkRange *range,
                     GtkWidget *area)
{
  if (in_update)
    return;
  setting_run_weight = gtk_range_get_value (range);
  set_weight (run_action, setting_run_weight);
}

static void
time_scale_changed  (GtkRange *range,
                     GtkWidget *area)
{
  float time_scale = gtk_range_get_value (range);

  gthree_animation_mixer_set_time_scale  (mixer, time_scale);
}

static void
execute_cross_fade (GthreeAnimationAction *start_action, GthreeAnimationAction *end_action, float duration)
{
  // Not only the start action, but also the end action must get a weight of 1 before fading
  // (concerning the start action this is already guaranteed in this place)

  set_weight (end_action, 1);
  gthree_animation_action_set_time (end_action, 0);

  // Crossfade with warping - you can also try without warping by setting the third parameter to false
  gthree_animation_action_cross_fade_to (start_action, end_action, duration, TRUE);
}

typedef struct {
  GthreeAnimationAction *start_action;
  GthreeAnimationAction *end_action;
  float duration;
} SyncCrossFade;

static void
on_loop_finished (GthreeAnimationMixer *mixer, GthreeAnimationAction *action, int direction, gpointer user_data)
{
  SyncCrossFade *data = user_data;

  if (action == data->start_action)
    {
      g_signal_handlers_disconnect_by_func (mixer, on_loop_finished, data);

      execute_cross_fade (data->start_action, data->end_action, data->duration);

      g_free (data);
    }
}


static void
synchronize_cross_fade (GthreeAnimationAction *start_action, GthreeAnimationAction *end_action, float duration)
{
  SyncCrossFade *data = g_new0 (SyncCrossFade, 1);
  data->start_action = start_action;
  data->end_action = end_action;
  data->duration = duration;

  g_signal_connect (mixer, "loop", G_CALLBACK (on_loop_finished), data);
}

static float
getCrossFadeDuration (float default_duration)
{
  // Switch default crossfade duration <-> custom crossfade duration

  //if ( settings[ 'use default duration' ] ) {
  return default_duration;
  //} else {
  //return settings[ 'set custom duration' ];
  //}
}

static void
prepare_cross_fade (GthreeAnimationAction *start_action, GthreeAnimationAction *end_action, float default_duration)
{
  // Switch default / custom crossfade duration (according to the user's choice)
  float duration = getCrossFadeDuration (default_duration);

  // Make sure that we don't go on in singleStepMode, and that all actions are unpaused
  //singleStepMode = false;
  //unPauseAllActions();

  // If the current action is 'idle' (duration 4 sec), execute the crossfade immediately;
  // else wait until the current action has finished its current loop
  if (start_action == idle_action)
    execute_cross_fade (start_action, end_action, duration);
  else
    synchronize_cross_fade (start_action, end_action, duration);
}


static void
walk_to_idle (GtkButton *button,
              GtkWidget *area)
{
  prepare_cross_fade (walk_action, idle_action, 1.0);
}

static void
idle_to_walk (GtkButton *button,
              GtkWidget *area)
{
  prepare_cross_fade (idle_action, walk_action, 0.5);
}


static void
walk_to_run (GtkButton *button,
             GtkWidget *area)
{
  prepare_cross_fade (walk_action, run_action, 2.5);
}

static void
run_to_walk (GtkButton *button,
             GtkWidget *area)
{
  prepare_cross_fade (run_action, walk_action, 5.0);
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area, *check, *hbox, *scale, *label, *button, *outer_hbox;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  gboolean done = FALSE;

  window = examples_init ("Skeletal animation", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (45, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 1, 2, -3);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera), 0, 1, 0);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect_after (area, "realize", G_CALLBACK (realize_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  outer_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (outer_hbox);
  gtk_box_append (GTK_BOX (box), outer_hbox);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_widget_show (box);
  gtk_box_append (GTK_BOX (outer_hbox), box);


  check = gtk_check_button_new_with_label ("Show skeleton");
#ifdef USE_GTK4
  gtk_check_button_set_active (GTK_CHECK_BUTTON (check), FALSE);
#else
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
#endif
  gtk_box_append (GTK_BOX (box), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (show_skeleton_toggled), area);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (hbox);
  gtk_box_append (GTK_BOX (box), hbox);

  label = gtk_label_new ("Crossfades: ");
  gtk_widget_show (label);
  gtk_box_append (GTK_BOX (hbox), label);

  walk_to_idle_button = button = gtk_button_new_with_label ("Walk to Idle");
  gtk_widget_show (button);
  gtk_box_append (GTK_BOX (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (walk_to_idle), area);

  idle_to_walk_button = button = gtk_button_new_with_label ("Idle to Walk");
  gtk_widget_show (button);
  gtk_box_append (GTK_BOX (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (idle_to_walk), area);

  walk_to_run_button = button = gtk_button_new_with_label ("Walk to Run");
  gtk_widget_show (button);
  gtk_box_append (GTK_BOX (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (walk_to_run), area);

  run_to_walk_button = button = gtk_button_new_with_label ("Run to Walk");
  gtk_widget_show (button);
  gtk_box_append (GTK_BOX (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (run_to_walk), area);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (hbox);
  gtk_box_append (GTK_BOX (box), hbox);

  label = gtk_label_new ("Time scale: ");
  gtk_widget_show (label);
  gtk_box_append (GTK_BOX (hbox), label);

  time_scale = scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 3, 0.01);
  gtk_range_set_value (GTK_RANGE (scale), 1.0);
  gtk_widget_set_hexpand (scale, TRUE);
  gtk_box_append (GTK_BOX (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (scale, "value-changed", G_CALLBACK (time_scale_changed), area);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_widget_show (box);
  gtk_box_append (GTK_BOX (outer_hbox), box);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (hbox);
  gtk_box_append (GTK_BOX (box), hbox);

  label = gtk_label_new ("Idle weight: ");
  gtk_widget_show (label);
  gtk_box_append (GTK_BOX (hbox), label);

  idle_scale = scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
  gtk_range_set_value (GTK_RANGE (scale), setting_idle_weight);
  gtk_widget_set_hexpand (scale, TRUE);
  gtk_box_append (GTK_BOX (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (scale, "value-changed", G_CALLBACK (idle_weight_changed), area);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (hbox);
  gtk_box_append (GTK_BOX (box), hbox);

  label = gtk_label_new ("Walk weight: ");
  gtk_widget_show (label);
  gtk_box_append (GTK_BOX (hbox), label);

  walk_scale = scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
  gtk_range_set_value (GTK_RANGE (scale), setting_walk_weight);
  gtk_widget_set_hexpand (scale, TRUE);
  gtk_box_append (GTK_BOX (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (scale, "value-changed", G_CALLBACK (walk_weight_changed), area);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (hbox);
  gtk_box_append (GTK_BOX (box), hbox);

  label = gtk_label_new ("Run weight: ");
  gtk_widget_show (label);
  gtk_box_append (GTK_BOX (hbox), label);

  run_scale = scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
  gtk_range_set_value (GTK_RANGE (scale), setting_run_weight);
  gtk_widget_set_hexpand (scale, TRUE);
  gtk_box_append (GTK_BOX (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (scale, "value-changed", G_CALLBACK (run_weight_changed), area);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
