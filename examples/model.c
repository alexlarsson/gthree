#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include "config.h"
#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

static GPtrArray *env_maps;
static GPtrArray *model_paths;
static GtkWidget *models_combo;
static GtkWidget *animations_combo;
static GtkWidget *morph_scale;

static int current_env_map;
static int current_model;
static float aspect = 1.0;

static GthreeOrbitControls *orbit;
static gboolean auto_rotate;
static gboolean fade_animations;

static GthreeScene *scene;
static GthreeAnimationMixer *mixer;
static GthreeAnimationAction *active_action;
static GthreeLoader *loader;
static float scene_radius;
static graphene_point3d_t scene_center;

/* These are owned by the scene */
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
  gthree_object_set_position_xyz (GTHREE_OBJECT (point_light),
                                  scene_radius, 0, 0);
  gthree_object_set_scale_uniform (GTHREE_OBJECT (point_light),
                                   scene_radius / 40);

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (white (), 0.125);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light),
                                  1, 1, -1);
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
add_camera (GthreeArea *area)
{
  graphene_vec3_t target;

  /* Generate default camera */
  camera = gthree_perspective_camera_new (37, aspect, scene_radius / 1000, scene_radius * 1000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  scene_center.x, scene_center.y, scene_radius * 3);

  orbit = gthree_orbit_controls_new (GTHREE_OBJECT (camera), GTK_WIDGET (area));
  graphene_vec3_init (&target, scene_center.x, scene_center.y, scene_center.z);
  gthree_orbit_controls_set_target (orbit, &target);
  gthree_orbit_controls_set_auto_rotate (orbit, auto_rotate);
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
      if (g_object_class_find_property (G_OBJECT_GET_CLASS (m), "env-map"))
        g_object_set (m, "env-map", texture, NULL);
    }
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
  gthree_perspective_camera_set_aspect (GTHREE_PERSPECTIVE_CAMERA (camera), aspect);
}

static void
update_scene (GthreeArea *area)
{
  int i;

  active_action = NULL;
  g_clear_object (&mixer);
  g_clear_object (&scene);
  g_clear_object (&loader);
  g_clear_object (&orbit);

  load_scene ();
  get_scene_size ();
  add_camera (area);
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
fade_animations_toggled (GtkWidget *toggle_button)
{
#ifdef USE_GTK4
  fade_animations = gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle_button));
#else
  fade_animations = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));
#endif
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
auto_rotate_toggled (GtkWidget *toggle_button)
{
#ifdef USE_GTK4
  auto_rotate = gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle_button));
#else
  auto_rotate = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));
#endif
  gthree_orbit_controls_set_auto_rotate (orbit, auto_rotate);
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
  gboolean done = FALSE;

  struct {
    char *path;
    char *name;
  } models[] = {
    { DATADIR "/gthree-examples/models/WaterBottle.glb", "WaterBottle" },
    { DATADIR "/gthree-examples/models/Soldier.glb", "Soldier" },
    { DATADIR "/gthree-examples/models/RobotExpressive.glb", "Robot" },
  };

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

  window = examples_init ("Models", &box, &done);

  area = gthree_area_new (NULL, NULL);

  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);

  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  /* Need a tick for the animations */
  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_box_append (GTK_BOX (box), hbox);
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

  for (i = 1; i < argc; i++)
    add_custom_model (argv[i]);

  g_signal_connect (combo, "changed", G_CALLBACK (model_combo_changed), area);

  gtk_box_append (GTK_BOX (hbox), combo);
  gtk_widget_show (combo);

  button = gtk_button_new_with_label ("Open");
  gtk_box_append (GTK_BOX (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (open_model), NULL);
  gtk_widget_show (button);

  combo = gtk_combo_box_text_new ();
  for (i = 0; i < G_N_ELEMENTS (cubes); i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), cubes[i].name);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (env_map_combo_changed), NULL);

  gtk_box_append (GTK_BOX (hbox), combo);
  gtk_widget_show (combo);

  check = gtk_check_button_new_with_label ("Auto rotate");
#ifdef USE_GTK4
  gtk_check_button_set_active (GTK_CHECK_BUTTON (check), FALSE);
#else
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
#endif
  gtk_box_append (GTK_BOX (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (auto_rotate_toggled), NULL);

  combo = gtk_combo_box_text_new ();
  animations_combo = combo;
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "No animation");
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (animations_combo_changed), NULL);

  gtk_box_append (GTK_BOX (hbox), combo);
  gtk_widget_show (combo);

  check = gtk_check_button_new_with_label ("Fade animations");
#ifdef USE_GTK4
  gtk_check_button_set_active (GTK_CHECK_BUTTON (check), FALSE);
#else
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
#endif
  gtk_box_append (GTK_BOX (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (fade_animations_toggled), NULL);

  morph_scale = scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1.0, 0.01);
  gtk_widget_set_size_request (scale, 100, -1);
  gtk_box_append (GTK_BOX (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (morph_scale, "value-changed", G_CALLBACK (morph_scale_changed), NULL);

  gtk_widget_show (window);

  update_scene (GTHREE_AREA (area));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
