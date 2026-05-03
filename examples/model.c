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

enum {
  COL_NAME,
  COL_TYPE,
  COL_OBJECT,
  N_COLUMNS
};

static GtkTreeStore *tree_store;
static GtkWidget *tree_view;
static GthreeMesh *bbox_helper;
static GthreeObject *bbox_helper_parent;

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
  gthree_light_set_intensity (GTHREE_LIGHT (ambient_light), G_PI);

  geometry_light = gthree_geometry_new_sphere (1, 8, 8);
  material_light = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_light, white ());

  point_light_group = gthree_group_new  ();
  gthree_object_set_position_point3d (GTHREE_OBJECT (point_light_group), &scene_center);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light_group));

  point_light = gthree_point_light_new (white (), 1 * G_PI, 0);
  gthree_object_add_child (GTHREE_OBJECT (point_light_group), GTHREE_OBJECT (point_light));
  gthree_object_set_position_xyz (GTHREE_OBJECT (point_light),
                                  scene_radius, 0, 0);
  gthree_object_set_scale_uniform (GTHREE_OBJECT (point_light),
                                   scene_radius / 40);

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (white (), 0.125 * G_PI);
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
  const char *path = g_ptr_array_index (model_paths, current_model);

  if (g_str_has_prefix (path, "/") || g_str_has_prefix (path, "file://"))
    {
      GError *error = NULL;
      g_autoptr(GFile) file = g_file_new_for_commandline_arg (path);
      g_autoptr(GFile) parent = g_file_get_parent (file);
      g_autoptr(GBytes) bytes = g_file_load_bytes (file, NULL, NULL, &error);
      if (bytes == NULL)
        g_error ("Failed to load %s: %s\n", path, error->message);

      loader = gthree_loader_parse_gltf (bytes, parent, &error);
      if (loader == NULL)
        g_error ("Failed to parse %s: %s\n", path, error->message);
    }
  else
    {
      loader = examples_load_gltl (path);
    }

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
  GthreeTexture *texture = g_ptr_array_index (env_maps, current_env_map);

  gthree_scene_set_background_texture (scene, texture);
  gthree_scene_set_environment (scene, texture);
}

static const char *
get_object_type_name (GthreeObject *object)
{
  const char *name = G_OBJECT_TYPE_NAME (object);
  if (g_str_has_prefix (name, "Gthree"))
    return name + 6;
  return name;
}

static void
populate_tree_recursive (GthreeObject *object, GtkTreeIter *parent)
{
  GtkTreeIter iter;
  GthreeObjectIter obj_iter;
  GthreeObject *child;
  const char *name = gthree_object_get_name (object);
  const char *type = get_object_type_name (object);

  gtk_tree_store_append (tree_store, &iter, parent);
  gtk_tree_store_set (tree_store, &iter,
                      COL_NAME, name && *name ? name : type,
                      COL_TYPE, type,
                      COL_OBJECT, object,
                      -1);

  gthree_object_iter_init (&obj_iter, object);
  while (gthree_object_iter_next (&obj_iter, &child))
    populate_tree_recursive (child, &iter);
}

static void
populate_tree (void)
{
  gtk_tree_store_clear (tree_store);
  if (scene)
    populate_tree_recursive (GTHREE_OBJECT (scene), NULL);
  gtk_tree_view_expand_all (GTK_TREE_VIEW (tree_view));
}

static void
remove_bbox_helper (void)
{
  if (bbox_helper)
    {
      gthree_object_remove_child (bbox_helper_parent, GTHREE_OBJECT (bbox_helper));
      bbox_helper = NULL;
      bbox_helper_parent = NULL;
    }
}

static void
show_bbox_helper (GthreeObject *object)
{
  graphene_box_t world_bbox, local_bbox;
  graphene_matrix_t world_inv;
  graphene_point3d_t min_pt, max_pt;
  graphene_vec3_t center;
  float w, h, d;
  g_autoptr(GthreeGeometry) geo = NULL;
  g_autoptr(GthreeMeshBasicMaterial) mat = NULL;

  remove_bbox_helper ();

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), TRUE);
  gthree_object_get_mesh_extents (object, &world_bbox);

  if (graphene_box_equal (&world_bbox, graphene_box_empty ()))
    return;

  graphene_matrix_inverse (gthree_object_get_world_matrix (object), &world_inv);
  graphene_matrix_transform_box (&world_inv, &world_bbox, &local_bbox);

  graphene_box_get_min (&local_bbox, &min_pt);
  graphene_box_get_max (&local_bbox, &max_pt);

  graphene_vec3_t size;
  float pad;
  graphene_box_get_size (&local_bbox, &size);
  pad = graphene_vec3_length (&size) * 0.005f;

  min_pt.x -= pad; min_pt.y -= pad; min_pt.z -= pad;
  max_pt.x += pad; max_pt.y += pad; max_pt.z += pad;

  w = max_pt.x - min_pt.x;
  h = max_pt.y - min_pt.y;
  d = max_pt.z - min_pt.z;

  geo = gthree_geometry_new_box (w, h, d, 1, 1, 1);

  mat = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (mat, orange ());
  gthree_material_set_is_transparent (GTHREE_MATERIAL (mat), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (mat), 0.3);
  gthree_material_set_depth_write (GTHREE_MATERIAL (mat), FALSE);

  bbox_helper = gthree_mesh_new (geo, GTHREE_MATERIAL (mat));
  graphene_vec3_init (&center,
                      (min_pt.x + max_pt.x) / 2.0f,
                      (min_pt.y + max_pt.y) / 2.0f,
                      (min_pt.z + max_pt.z) / 2.0f);
  gthree_object_set_position (GTHREE_OBJECT (bbox_helper), &center);

  bbox_helper_parent = object;
  gthree_object_add_child (object, GTHREE_OBJECT (bbox_helper));
}

static void
tree_selection_changed (GtkTreeSelection *selection, gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GthreeObject *object;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      remove_bbox_helper ();
      return;
    }

  gtk_tree_model_get (model, &iter, COL_OBJECT, &object, -1);
  if (object == NULL)
    return;

  show_bbox_helper (object);
}

static gboolean
find_object_in_tree (GtkTreeModel *model, GtkTreeIter *result,
                     GtkTreeIter *parent, GthreeObject *target)
{
  GtkTreeIter iter;

  if (!gtk_tree_model_iter_children (model, &iter, parent))
    return FALSE;

  do
    {
      GthreeObject *obj;
      gtk_tree_model_get (model, &iter, COL_OBJECT, &obj, -1);
      if (obj == target)
        {
          *result = iter;
          return TRUE;
        }
      if (find_object_in_tree (model, result, &iter, target))
        return TRUE;
    }
  while (gtk_tree_model_iter_next (model, &iter));

  return FALSE;
}

static void
select_object_in_tree (GthreeObject *target)
{
  GtkTreeIter iter;
  GtkTreeModel *model = GTK_TREE_MODEL (tree_store);
  GtkTreeSelection *selection;
  GtkTreePath *path;

  if (!find_object_in_tree (model, &iter, NULL, target))
    return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  path = gtk_tree_model_get_path (model, &iter);
  gtk_tree_view_expand_to_path (GTK_TREE_VIEW (tree_view), path);
  gtk_tree_selection_select_iter (selection, &iter);
  gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, FALSE, 0, 0);
  gtk_tree_path_free (path);
}

#ifdef USE_GTK4
static void
area_click_pressed (GtkGestureClick *gesture,
                    int              n_press,
                    double           x,
                    double           y,
                    gpointer         user_data)
{
  GdkModifierType state;
  GtkWidget *widget;
  float ndc_x, ndc_y;
  g_autoptr(GthreeRaycaster) raycaster = NULL;
  g_autoptr(GPtrArray) intersections = NULL;
  guint i;

  state = gtk_event_controller_get_current_event_state (GTK_EVENT_CONTROLLER (gesture));
  if (!(state & GDK_CONTROL_MASK))
    return;

  widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
  ndc_x = ((float)x / gtk_widget_get_width (widget)) * 2 - 1;
  ndc_y = -((float)y / gtk_widget_get_height (widget)) * 2 + 1;

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), TRUE);

  raycaster = gthree_raycaster_new ();
  gthree_raycaster_set_from_camera (raycaster, GTHREE_CAMERA (camera), ndc_x, ndc_y);
  intersections = gthree_raycaster_intersect_object (raycaster, GTHREE_OBJECT (scene), TRUE, NULL);

  for (i = 0; i < intersections->len; i++)
    {
      GthreeRayIntersection *hit = g_ptr_array_index (intersections, i);
      if (hit->object == GTHREE_OBJECT (bbox_helper))
        continue;
      select_object_in_tree (hit->object);
      gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
      return;
    }

  gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)));
  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}
#else
static gboolean
area_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  float ndc_x, ndc_y;
  g_autoptr(GthreeRaycaster) raycaster = NULL;
  g_autoptr(GPtrArray) intersections = NULL;
  guint i;

  if (event->button != 1 || !(event->state & GDK_CONTROL_MASK))
    return FALSE;

  ndc_x = ((float)event->x / gtk_widget_get_width (widget)) * 2 - 1;
  ndc_y = -((float)event->y / gtk_widget_get_height (widget)) * 2 + 1;

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), TRUE);

  raycaster = gthree_raycaster_new ();
  gthree_raycaster_set_from_camera (raycaster, GTHREE_CAMERA (camera), ndc_x, ndc_y);
  intersections = gthree_raycaster_intersect_object (raycaster, GTHREE_OBJECT (scene), TRUE, NULL);

  for (i = 0; i < intersections->len; i++)
    {
      GthreeRayIntersection *hit = g_ptr_array_index (intersections, i);
      if (hit->object == GTHREE_OBJECT (bbox_helper))
        continue;
      select_object_in_tree (hit->object);
      return TRUE;
    }

  gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)));
  return TRUE;
}
#endif

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
  bbox_helper = NULL;
  bbox_helper_parent = NULL;
  g_clear_object (&mixer);
  g_clear_object (&scene);
  g_clear_object (&loader);
  g_clear_object (&orbit);

  load_scene ();
  get_scene_size ();
  populate_tree ();
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
tone_mapping_combo_changed (GtkComboBox *combo, GthreeArea *area)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (area);
  int index = gtk_combo_box_get_active (combo);
  gthree_renderer_set_tone_mapping (renderer, (GthreeToneMapping)index);
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
  int i;
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
    { "WaterBottle.glb", "WaterBottle" },
    { "Soldier.glb", "Soldier" },
    { "RobotExpressive.glb", "Robot" },
    { "ClearCoatCarPaint.glb", "Clear Coat Car Paint" },
    { "LeePerrySmith/LeePerrySmith.glb", "LeePerrySmith" },
    { "DamagedHelmet.glb", "Damaged Helmet"},
    { "LittlestTokyo.glb", "Littlest Tokyo"},
  };

  env_maps = g_ptr_array_new_with_free_func (g_object_unref);
  for (i = 0; i < G_N_ELEMENTS (cubes); i++)
    {
      GthreeCubeTexture *cube_texture;

      cube_texture = examples_load_cube_texture (cubes[i].path);
      gthree_texture_set_encoding (GTHREE_TEXTURE (cube_texture), GTHREE_ENCODING_FORMAT_SRGB);

      g_ptr_array_add (env_maps, cube_texture);
    }

  window = examples_init ("Models", &box, &done);
  gtk_window_set_default_size (GTK_WINDOW (window), 1200, 700);

  {
    GtkWidget *paned, *scrolled;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;

    paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand (paned, TRUE);
    gtk_widget_set_vexpand (paned, TRUE);
    gtk_box_append (GTK_BOX (box), paned);

    tree_store = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store));
    g_object_unref (tree_store);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Name", cell, "text", COL_NAME, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Type", cell, "text", COL_TYPE, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

#ifdef USE_GTK4
    scrolled = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), tree_view);
#else
    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scrolled), tree_view);
#endif
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request (scrolled, 250, -1);

    area = gthree_area_new (NULL, NULL);
    g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);

#ifdef USE_GTK4
    gtk_paned_set_start_child (GTK_PANED (paned), area);
    gtk_paned_set_end_child (GTK_PANED (paned), scrolled);
    gtk_paned_set_resize_end_child (GTK_PANED (paned), FALSE);
    gtk_paned_set_shrink_end_child (GTK_PANED (paned), FALSE);
#else
    gtk_paned_pack1 (GTK_PANED (paned), area, TRUE, FALSE);
    gtk_paned_pack2 (GTK_PANED (paned), scrolled, FALSE, FALSE);
#endif

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    g_signal_connect (selection, "changed", G_CALLBACK (tree_selection_changed), NULL);

#ifdef USE_GTK4
    {
      GtkGesture *click = gtk_gesture_click_new ();
      g_signal_connect (click, "pressed", G_CALLBACK (area_click_pressed), NULL);
      gtk_widget_add_controller (area, GTK_EVENT_CONTROLLER (click));
    }
#else
    g_signal_connect (area, "button-press-event", G_CALLBACK (area_button_press), NULL);
#endif
  }
  /* Need a tick for the animations */
  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_box_append (GTK_BOX (box), hbox);

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

  button = gtk_button_new_with_label ("Open");
  gtk_box_append (GTK_BOX (hbox), button);
  g_signal_connect (button, "clicked", G_CALLBACK (open_model), NULL);

  combo = gtk_combo_box_text_new ();
  for (i = 0; i < G_N_ELEMENTS (cubes); i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), cubes[i].name);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (env_map_combo_changed), NULL);

  gtk_box_append (GTK_BOX (hbox), combo);

  combo = gtk_combo_box_text_new ();
  {
    const char *tone_mappings[] = {
      "No Tone Mapping", "Linear", "Reinhard", "Cineon", "ACES Filmic", "AgX", "Neutral"
    };
    for (i = 0; i < G_N_ELEMENTS (tone_mappings); i++)
      gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), tone_mappings[i]);
  }
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (tone_mapping_combo_changed), area);
  gtk_box_append (GTK_BOX (hbox), combo);

  check = gtk_check_button_new_with_label ("Auto rotate");
#ifdef USE_GTK4
  gtk_check_button_set_active (GTK_CHECK_BUTTON (check), FALSE);
#else
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
#endif
  gtk_box_append (GTK_BOX (hbox), check);
  g_signal_connect (check, "toggled", G_CALLBACK (auto_rotate_toggled), NULL);

  combo = gtk_combo_box_text_new ();
  animations_combo = combo;
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), "No animation");
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  g_signal_connect (combo, "changed", G_CALLBACK (animations_combo_changed), NULL);

  gtk_box_append (GTK_BOX (hbox), combo);

  check = gtk_check_button_new_with_label ("Fade animations");
#ifdef USE_GTK4
  gtk_check_button_set_active (GTK_CHECK_BUTTON (check), FALSE);
#else
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
#endif
  gtk_box_append (GTK_BOX (hbox), check);
  g_signal_connect (check, "toggled", G_CALLBACK (fade_animations_toggled), NULL);

  morph_scale = scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1.0, 0.01);
  gtk_widget_set_size_request (scale, 100, -1);
  gtk_box_append (GTK_BOX (hbox), scale);
  g_signal_connect (morph_scale, "value-changed", G_CALLBACK (morph_scale_changed), NULL);

  gtk_window_present (GTK_WINDOW (window));

  update_scene (GTHREE_AREA (area));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
