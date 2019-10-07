#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GPtrArray *env_maps;
static GPtrArray *model_paths;
static GtkWidget *models_combo;
static GtkWidget *animations_combo;
static GtkWidget *morph_scale;

static int current_env_map;
static int current_model;

static float current_distance = 3;
static float current_angle_y, current_angle_x;
static float press_angle_y, press_angle_x;
static float press_y, press_x;

static gboolean button_down;
static gboolean auto_rotate;
static gboolean fade_animations;
static float auto_rotate_start_time;
static float auto_rotate_start_angle;
static float last_frame_time;

static GthreeScene *scene;
static GthreeAnimationMixer *mixer;
static GthreeAnimationAction *active_action;
static GthreeLoader *loader;
static float scene_radius;
static graphene_point3d_t scene_center;

/* These are owned by the scene */
static GthreeGroup *camera_group;
static GthreePerspectiveCamera *camera;
static GthreeGroup *point_light_group;
static GthreePointLight *point_light;

static void
light_scene (void)
{
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;
  GthreeMeshBasicMaterial *material_light;
  GthreeGeometry *geometry_light;
  GthreeMesh *particle_light;
  graphene_point3d_t pos;

  ambient_light = gthree_ambient_light_new (white ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (1, 8, 8);
  material_light = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_light, white ());

  point_light_group = gthree_group_new  ();
  gthree_object_set_position_point3d (GTHREE_OBJECT (point_light_group), &scene_center);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light_group));

  point_light = gthree_point_light_new (white (), 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (point_light_group), GTHREE_OBJECT (point_light));
  gthree_object_set_position_point3d (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos, scene_radius, 0, 0));
  gthree_object_set_scale_point3d (GTHREE_OBJECT (point_light),
                           graphene_point3d_init (&pos,
                                                  scene_radius / 40,
                                                  scene_radius / 40,
                                                  scene_radius / 40));

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (white (), 0.125);
  gthree_object_set_position_point3d (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     1, 1, -1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
}

// Temporary Work around bug in graphene (not addin in min to center)
static graphene_sphere_t *
box_get_bounding_sphere (const graphene_box_t *box,
                         graphene_sphere_t    *sphere)
{
  graphene_vec3_t size;
  graphene_vec3_t center;
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

static void
load_scene (void)
{
  GError *error = NULL;
  g_autoptr(GFile) file = NULL;
  g_autoptr(GFile) parent = NULL;
  g_autoptr(GBytes) bytes = NULL;
  const char *path;

  path = g_ptr_array_index (model_paths, current_model);

  file = g_file_new_for_commandline_arg (path);
  parent = g_file_get_parent (file);
  bytes = g_file_load_bytes (file, NULL, NULL, &error);
  if (bytes == NULL)
    g_error ("Failed to load %s: %s\n", path, error->message);

  loader = gthree_loader_parse_gltf (bytes, parent, &error);
  if (loader == NULL)
    g_error ("Failed to %s: %s\n", path, error->message);

  scene = g_object_ref (gthree_loader_get_scene (loader, 0));
}

static void
get_scene_size (void)
{
  graphene_box_t bounding_box;
  graphene_sphere_t bounding_sphere;

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), TRUE);

  gthree_object_get_mesh_extents (GTHREE_OBJECT (scene), &bounding_box);

  box_get_bounding_sphere (&bounding_box, &bounding_sphere);

  graphene_sphere_get_center (&bounding_sphere, &scene_center);
  scene_radius = graphene_sphere_get_radius (&bounding_sphere);
}

static void
add_camera (void)
{
  /* Generate default camera */
  camera_group = gthree_group_new ();
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_group));
  gthree_object_set_position_point3d (GTHREE_OBJECT (camera_group), &scene_center);

  camera = gthree_perspective_camera_new (37, 1.5, scene_radius / 1000, scene_radius * 1000);
  gthree_object_add_child (GTHREE_OBJECT (camera_group), GTHREE_OBJECT (camera));
}

static void
apply_env_map (void)
{
  int n_materials, i;
  GthreeTexture *texture = g_ptr_array_index (env_maps, current_env_map);

  gthree_scene_set_background_texture (scene, texture);

  n_materials = gthree_loader_get_n_materials (loader);
  for (i = 0; i < n_materials; i++)
    {
      GthreeMaterial *m = gthree_loader_get_material (loader, i);
      if (GTHREE_IS_MESH_STANDARD_MATERIAL (m))
        gthree_mesh_standard_material_set_env_map (GTHREE_MESH_STANDARD_MATERIAL (m), texture);
    }
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_point3d_t pos;
  graphene_euler_t rot;
  static gint64 last_frame_time_i = 0;
  static gint64 first_frame_time_i = 0;
  gint64 frame_time_i;
  float frame_time;

  frame_time_i = gdk_frame_clock_get_frame_time (frame_clock);
  if (last_frame_time_i != 0)
    {
      float delta_time_sec = (frame_time_i - last_frame_time_i) / (float) G_USEC_PER_SEC;
      gthree_animation_mixer_update (mixer, delta_time_sec);
    }
  else
    first_frame_time_i = frame_time_i;
  last_frame_time_i = frame_time_i;

  // Scale to some random useful float value
  frame_time = (frame_time_i - first_frame_time_i) / 100000.0;

  gthree_object_set_rotation (GTHREE_OBJECT (point_light_group),
                              graphene_euler_init (&rot,
                                                   frame_time * 7,
                                                   frame_time * 13,
                                                   0));

  if (auto_rotate && !button_down)
    current_angle_y = current_angle_y + 0.3;

  gthree_object_set_rotation (GTHREE_OBJECT (camera_group),
                                graphene_euler_init (&rot,
                                                     current_angle_x,
                                                     current_angle_y,
                                                     0));
  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, current_distance * scene_radius));


  gtk_widget_queue_draw (widget);

  last_frame_time = frame_time;

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height)
{
  gthree_perspective_camera_set_aspect (GTHREE_PERSPECTIVE_CAMERA (camera), (float)width / (float)(height));
}

static void
update_scene (GthreeArea *area)
{
  int i;

  active_action = NULL;
  g_clear_object (&mixer);
  g_clear_object (&scene);
  g_clear_object (&loader);

  load_scene ();
  get_scene_size ();
  add_camera ();
  light_scene ();
  apply_env_map ();

  mixer = gthree_animation_mixer_new (GTHREE_OBJECT (scene));

  gtk_combo_box_text_remove_all  (GTK_COMBO_BOX_TEXT (animations_combo));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (animations_combo), "No animation");
  gtk_combo_box_set_active (GTK_COMBO_BOX (animations_combo), 0);

  for (i = 0; i < gthree_loader_get_n_animations (loader); i++)
    {
      GthreeAnimationClip *clip = gthree_loader_get_animation (loader, i);
      gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (animations_combo), gthree_animation_clip_get_name (clip));
    }

  gthree_area_set_scene (area, scene);
  gthree_area_set_camera (area, GTHREE_CAMERA (camera));
}


static void
animations_combo_changed (GtkComboBox *combo)
{
  int index = gtk_combo_box_get_active (combo);
  GthreeAnimationClip *clip;
  float fade_time = 0.4;

  if (index <= 0)
    {
      // No animation
      clip = NULL;
    }
  else
    clip = gthree_loader_get_animation (loader, index - 1);

  if (active_action != NULL && !fade_animations)
      gthree_animation_action_set_enabled (active_action, FALSE);

  if (clip != NULL)
    {
      GthreeAnimationAction *action = gthree_animation_mixer_clip_action (mixer, clip, NULL);

      gthree_animation_action_set_loop_mode (action, GTHREE_LOOP_MODE_REPEAT, -1);
      gthree_animation_action_set_enabled (action, TRUE);

      if (fade_animations)
        {
          if (active_action)
            gthree_animation_action_cross_fade_to (active_action, action, fade_time, FALSE);
          else
            gthree_animation_action_fade_in (action, fade_time);
        }

      gthree_animation_action_play (action);

      active_action = action;
    }
  else if (active_action != NULL)
    {
      if (fade_animations)
        gthree_animation_action_fade_out (active_action, fade_time);
      active_action = NULL;
    }

}

static void
fade_animations_toggled (GtkToggleButton *toggle_button)
{
  fade_animations = gtk_toggle_button_get_active (toggle_button);
}

static void
model_combo_changed (GtkComboBox *combo, GthreeArea *area)
{
  current_model = gtk_combo_box_get_active (combo);
  update_scene (area);
}

static void
env_map_combo_changed (GtkComboBox *combo)
{
  current_env_map = gtk_combo_box_get_active (combo);
  apply_env_map ();
}

static void
auto_rotate_toggled (GtkToggleButton *toggle_button)
{
  auto_rotate = gtk_toggle_button_get_active (toggle_button);
  if (auto_rotate)
    {
      const graphene_euler_t *current_rot = gthree_object_get_rotation (GTHREE_OBJECT (camera_group));

      auto_rotate_start_angle = graphene_euler_get_y (current_rot);
      auto_rotate_start_time = last_frame_time;
    }
}

static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  button_down = TRUE;

  if (event->button == GDK_BUTTON_PRIMARY)
    {
      press_angle_x = current_angle_x;
      press_angle_y = current_angle_y;
      press_x = event->x;
      press_y = event->y;
    }

  return TRUE;
}

static gboolean
button_release_event_cb (GtkWidget      *widget,
                         GdkEventButton *event,
                         gpointer        data)
{
  button_down = FALSE;
  return TRUE;
}

static gboolean
motion_notify_event_cb (GtkWidget      *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  if (event->state & GDK_BUTTON1_MASK)
    {
      current_angle_y = press_angle_y - (event->x - press_x)  * 180.0 / gtk_widget_get_allocated_width (widget);
      current_angle_x = press_angle_x - (event->y - press_y)  * 180.0 / gtk_widget_get_allocated_height (widget);
    }

  return TRUE;
}

static gboolean
scroll_event_cb (GtkWidget   *widget,
                 GdkEvent    *event,
                 gpointer     data)
{
  GdkScrollDirection direction;
  if (gdk_event_get_scroll_direction (event, &direction))
    {
      if (direction == GDK_SCROLL_UP)
        current_distance += 0.1;
      else if (direction == GDK_SCROLL_DOWN)
        current_distance -= 0.1;

      current_distance = MAX (current_distance, 0.5);
    }
  return TRUE;
}


static void
add_custom_model (const char *url)
{
  g_autofree char *basename = g_path_get_basename (url);
  int index;

  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (models_combo), basename);
  index = model_paths->len;
  g_ptr_array_add (model_paths, g_strdup (url));

  current_model = index;
  gtk_combo_box_set_active (GTK_COMBO_BOX (models_combo), current_model);
}

static void
open_response_cb (GtkNativeDialog *dialog,
                  gint             response_id,
                  gpointer         user_data)
{
  GtkFileChooserNative *native = user_data;
  GFile *file;
  g_autofree char *uri = NULL;

  if (response_id == GTK_RESPONSE_ACCEPT)
    {
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (native));
      uri = g_file_get_uri (file);
      add_custom_model (uri);
    }

  gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (native));
  g_object_unref (native);
}

static void
open_model (GtkButton *button)
{
  GtkFileChooserNative *native;

  native = gtk_file_chooser_native_new ("Open GLTF model",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "_Open",
                                        "_Cancel");
  g_signal_connect (native,
                    "response",
                    G_CALLBACK (open_response_cb),
                    native);
  gtk_native_dialog_show (GTK_NATIVE_DIALOG (native));
}

static void
morph_scale_changed (GtkRange *range)
{
  g_autoptr(GList) meshes = gthree_object_find_by_type (GTHREE_OBJECT (scene), GTHREE_TYPE_MESH);
  gdouble v = gtk_range_get_value (range);
  GList *l;

  for (l = meshes; l != NULL; l = l->next)
    {
      GthreeMesh *mesh = l->data;
      GArray *morph_targets = gthree_mesh_get_morph_targets (mesh);
      if (morph_targets != NULL && morph_targets->len > 0)
        g_array_index (morph_targets, float, 0) = (float)v;
    }

}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area, *combo, *check, *scale;
  int i, j;
  struct {
    char *path;
    char *name;
  } cubes[] = {
    { "cube/Park2", "Park" },
    { "cube/Bridge2", "Bridge" },
    { "cube/SwedishRoyalCastle", "Castle" },
  };

  struct {
    char *path;
    char *name;
  } models[] = {
    { "resource:///org/gnome/gthree-examples/models/WaterBottle.glb", "WaterBottle" },
    { "resource:///org/gnome/gthree-examples/models/Soldier.glb", "Soldier" },
    { "resource:///org/gnome/gthree-examples/models/RobotExpressive.glb", "Robot" },
  };

#ifdef USE_GTK4
  gtk_init ();
#else
  gtk_init (&argc, &argv);
#endif

  env_maps = g_ptr_array_new_with_free_func (g_object_unref);
  for (i = 0; i < G_N_ELEMENTS (cubes); i++)
    {
      GdkPixbuf *pixbufs[6];
      GthreeCubeTexture *cube_texture;

      examples_load_cube_pixbufs (cubes[i].path, pixbufs);
      cube_texture = gthree_cube_texture_new_from_array (pixbufs);
      for (j = 0; j < 6; j++)
        g_object_unref (pixbufs[j]);

      g_ptr_array_add (env_maps, cube_texture);
    }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Models");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
#ifdef USE_GTK3
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
#endif
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (box), 6);
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show (box);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_container_add (GTK_CONTAINER (box), hbox);
  gtk_widget_show (hbox);

  area = gthree_area_new (NULL, NULL);

  gtk_widget_set_events (area, gtk_widget_get_events (area)
                         | GDK_SCROLL_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_POINTER_MOTION_MASK);
  g_signal_connect (area, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);
  g_signal_connect (area, "button-press-event",
                    G_CALLBACK (button_press_event_cb), NULL);
  g_signal_connect (area, "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);
  g_signal_connect (area, "scroll-event",
                    G_CALLBACK (scroll_event_cb), NULL);

  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);

  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_container_add (GTK_CONTAINER (box), hbox);
  gtk_widget_show (hbox);

  model_paths = g_ptr_array_new_with_free_func (g_free);
  combo = gtk_combo_box_text_new ();
  for (i = 0; i < G_N_ELEMENTS (models); i++)
    {
      gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), models[i].name);
      g_ptr_array_add (model_paths, g_strdup (models[i].path));
    }
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), current_model);
  models_combo = combo;
  if (argc == 2)
    add_custom_model (argv[1]);

  g_signal_connect (combo, "changed", G_CALLBACK (model_combo_changed), area);

  gtk_container_add (GTK_CONTAINER (hbox), combo);
  gtk_widget_show (combo);

  button = gtk_button_new_with_label ("Open");
  gtk_container_add (GTK_CONTAINER (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (open_model), NULL);
  gtk_widget_show (button);

  combo = gtk_combo_box_text_new ();
  for (i = 0; i < G_N_ELEMENTS (cubes); i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), cubes[i].name);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (env_map_combo_changed), NULL);

  gtk_container_add (GTK_CONTAINER (hbox), combo);
  gtk_widget_show (combo);

  check = gtk_check_button_new_with_label ("Auto rotate");
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (auto_rotate_toggled), NULL);

  combo = gtk_combo_box_text_new ();
  animations_combo = combo;
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "No animation");
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (animations_combo_changed), NULL);

  gtk_container_add (GTK_CONTAINER (hbox), combo);
  gtk_widget_show (combo);

  check = gtk_check_button_new_with_label ("Fade animations");
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (fade_animations_toggled), NULL);

  morph_scale =scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1.0, 0.01);
  gtk_widget_set_size_request (scale, 100, -1);
  gtk_container_add (GTK_CONTAINER (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (morph_scale, "value-changed", G_CALLBACK (morph_scale_changed), NULL);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  update_scene (GTHREE_AREA (area));

  gtk_main ();

  return EXIT_SUCCESS;
}
