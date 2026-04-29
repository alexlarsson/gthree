#include <stdlib.h>
#include <gtk/gtk.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreeScene *scene;
static GthreePerspectiveCamera *camera;
static GthreeMeshPhysicalMaterial *material;
static GthreeMesh *sphere;

static GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeCubeTexture *env_cube;

  scene = gthree_scene_new ();

  env_cube = examples_load_cube_texture ("cube/SwedishRoyalCastle");
  gthree_texture_set_encoding (GTHREE_TEXTURE (env_cube), GTHREE_ENCODING_FORMAT_SRGB);

  gthree_scene_set_background_texture (scene, GTHREE_TEXTURE (env_cube));
  gthree_scene_set_environment (scene, GTHREE_TEXTURE (env_cube));

  material = gthree_mesh_physical_material_new ();
  gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (material), white ());
  gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (material), 0.0);
  gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (material), 0.5);

  geometry = gthree_geometry_new_sphere (40, 64, 32);
  sphere = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (sphere));

  return scene;
}

static int cursor_x, cursor_y;

static void
motion_cb (GtkEventControllerMotion *controller,
           gdouble                   x,
           gdouble                   y,
           gpointer                  user_data)
{
  cursor_x = x;
  cursor_y = y;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  float camera_angle = (cursor_x) * 2.0 * G_PI / gtk_widget_get_width (widget) - G_PI / 2.0;
  float camera_height = (((float)cursor_y / gtk_widget_get_height (widget)) - 0.5) * 200;

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  cos (camera_angle) * 120,
                                  camera_height,
                                  sin (camera_angle) * 120);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera), 0, 0, 0);

  gtk_widget_queue_draw (widget);
  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area, gint width, gint height,
             GthreePerspectiveCamera *cam)
{
  gthree_perspective_camera_set_aspect (cam, (float)width / (float)height);
}

/* Helper to add a labeled slider row */
static GtkWidget *
add_slider (GtkWidget *grid, int *row, const char *label,
            float min, float max, float initial, float step,
            GCallback callback)
{
  GtkWidget *lbl = gtk_label_new (label);
  gtk_label_set_xalign (GTK_LABEL (lbl), 0);
  GtkWidget *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL,
                                                min, max, step);
  gtk_range_set_value (GTK_RANGE (scale), initial);
  gtk_widget_set_hexpand (scale, TRUE);
  gtk_widget_set_size_request (scale, 120, -1);

#ifdef USE_GTK4
  gtk_grid_attach (GTK_GRID (grid), lbl, 0, *row, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), scale, 1, *row, 1, 1);
#else
  gtk_grid_attach (GTK_GRID (grid), lbl, 0, *row, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), scale, 1, *row, 1, 1);
  gtk_widget_show (lbl);
  gtk_widget_show (scale);
#endif
  (*row)++;

  g_signal_connect (scale, "value-changed", callback, NULL);
  return scale;
}

/* Helper to add a color button row */
static GtkWidget *
add_color (GtkWidget *grid, int *row, const char *label,
           const graphene_vec3_t *initial, GCallback callback)
{
  GtkWidget *lbl = gtk_label_new (label);
  gtk_label_set_xalign (GTK_LABEL (lbl), 0);

  GdkRGBA rgba = {
    graphene_vec3_get_x (initial),
    graphene_vec3_get_y (initial),
    graphene_vec3_get_z (initial),
    1.0
  };
#ifdef USE_GTK4
  GtkWidget *btn = gtk_color_button_new_with_rgba (&rgba);
#else
  GtkWidget *btn = gtk_color_button_new_with_rgba (&rgba);
  gtk_widget_show (lbl);
  gtk_widget_show (btn);
#endif

#ifdef USE_GTK4
  gtk_grid_attach (GTK_GRID (grid), lbl, 0, *row, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn, 1, *row, 1, 1);
#else
  gtk_grid_attach (GTK_GRID (grid), lbl, 0, *row, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn, 1, *row, 1, 1);
#endif
  (*row)++;

  g_signal_connect (btn, "color-set", callback, NULL);
  return btn;
}

static void
get_color_from_button (GtkColorButton *btn, graphene_vec3_t *color)
{
  GdkRGBA rgba;
#ifdef USE_GTK4
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (btn), &rgba);
#else
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (btn), &rgba);
#endif
  graphene_vec3_init (color, rgba.red, rgba.green, rgba.blue);
}

/* --- Callbacks --- */

static void on_roughness (GtkRange *r) {
  gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (material),
                                                gtk_range_get_value (r));
}
static void on_metalness (GtkRange *r) {
  gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (material),
                                                gtk_range_get_value (r));
}
static void on_color (GtkColorButton *btn) {
  graphene_vec3_t c;
  get_color_from_button (btn, &c);
  gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (material), &c);
}
static void on_emissive_color (GtkColorButton *btn) {
  graphene_vec3_t c;
  get_color_from_button (btn, &c);
  gthree_mesh_standard_material_set_emissive_color (GTHREE_MESH_STANDARD_MATERIAL (material), &c);
}
static void on_emissive_intensity (GtkRange *r) {
  gthree_mesh_standard_material_set_emissive_intensity (GTHREE_MESH_STANDARD_MATERIAL (material),
                                                         gtk_range_get_value (r));
}
static void on_env_map_intensity (GtkRange *r) {
  gthree_mesh_standard_material_set_env_map_intensity (GTHREE_MESH_STANDARD_MATERIAL (material),
                                                        gtk_range_get_value (r));
}
static void on_clearcoat (GtkRange *r) {
  gthree_mesh_physical_material_set_clearcoat (material, gtk_range_get_value (r));
}
static void on_clearcoat_roughness (GtkRange *r) {
  gthree_mesh_physical_material_set_clearcoat_roughness (material, gtk_range_get_value (r));
}
static void on_ior (GtkRange *r) {
  gthree_mesh_physical_material_set_ior (material, gtk_range_get_value (r));
}
static void on_iridescence (GtkRange *r) {
  gthree_mesh_physical_material_set_iridescence (material, gtk_range_get_value (r));
}
static void on_iridescence_ior (GtkRange *r) {
  gthree_mesh_physical_material_set_iridescence_ior (material, gtk_range_get_value (r));
}
static void on_iridescence_thickness_min (GtkRange *r) {
  gthree_mesh_physical_material_set_iridescence_thickness_min (material, gtk_range_get_value (r));
}
static void on_iridescence_thickness_max (GtkRange *r) {
  gthree_mesh_physical_material_set_iridescence_thickness_max (material, gtk_range_get_value (r));
}
static void on_sheen (GtkRange *r) {
  gthree_mesh_physical_material_set_sheen (material, gtk_range_get_value (r));
}
static void on_sheen_roughness (GtkRange *r) {
  gthree_mesh_physical_material_set_sheen_roughness (material, gtk_range_get_value (r));
}
static void on_sheen_color (GtkColorButton *btn) {
  graphene_vec3_t c;
  get_color_from_button (btn, &c);
  gthree_mesh_physical_material_set_sheen_color (material, &c);
}
static void on_transmission (GtkRange *r) {
  gthree_mesh_physical_material_set_transmission (material, gtk_range_get_value (r));
}
static void on_thickness (GtkRange *r) {
  gthree_mesh_physical_material_set_thickness (material, gtk_range_get_value (r));
}
static void on_attenuation_distance (GtkRange *r) {
  gthree_mesh_physical_material_set_attenuation_distance (material, gtk_range_get_value (r));
}
static void on_attenuation_color (GtkColorButton *btn) {
  graphene_vec3_t c;
  get_color_from_button (btn, &c);
  gthree_mesh_physical_material_set_attenuation_color (material, &c);
}
static void on_dispersion (GtkRange *r) {
  gthree_mesh_physical_material_set_dispersion (material, gtk_range_get_value (r));
}
static void on_specular_intensity (GtkRange *r) {
  gthree_mesh_physical_material_set_specular_intensity (material, gtk_range_get_value (r));
}
static void on_specular_color (GtkColorButton *btn) {
  graphene_vec3_t c;
  get_color_from_button (btn, &c);
  gthree_mesh_physical_material_set_specular_color (material, &c);
}
static void on_anisotropy (GtkRange *r) {
  gthree_mesh_physical_material_set_anisotropy (material, gtk_range_get_value (r));
}
static void on_anisotropy_rotation (GtkRange *r) {
  gthree_mesh_physical_material_set_anisotropy_rotation (material, gtk_range_get_value (r));
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *area, *scrolled, *grid;
  GtkEventController *motion;
  gboolean done = FALSE;
  int row = 0;

  window = examples_init ("Physical Material", &box, &done);

  init_scene ();
  camera = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 0, 120);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_hexpand (hbox, TRUE);
  gtk_widget_set_vexpand (hbox, TRUE);
  gtk_box_append (GTK_BOX (box), hbox);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (hbox), area);

  motion = motion_controller_for (GTK_WIDGET (area));
  g_signal_connect (motion, "motion", (GCallback)motion_cb, NULL);
  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  /* Controls panel */
  scrolled = gtk_scrolled_window_new (
#ifndef USE_GTK4
    NULL, NULL
#endif
  );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (scrolled, 300, -1);
  gtk_box_append (GTK_BOX (hbox), scrolled);

  grid = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
#ifdef USE_GTK4
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), grid);
#else
  gtk_container_add (GTK_CONTAINER (scrolled), grid);
  gtk_widget_show (grid);
#endif

  /* Standard material properties */
  add_color (grid, &row, "Color", white (), G_CALLBACK (on_color));
  add_slider (grid, &row, "Roughness",  0, 1, 0.5, 0.01, G_CALLBACK (on_roughness));
  add_slider (grid, &row, "Metalness",  0, 1, 0.0, 0.01, G_CALLBACK (on_metalness));
  add_color (grid, &row, "Emissive", black (), G_CALLBACK (on_emissive_color));
  add_slider (grid, &row, "Emissive intensity", 0, 2, 1.0, 0.01, G_CALLBACK (on_emissive_intensity));
  add_slider (grid, &row, "Env map intensity", 0, 2, 1.0, 0.01, G_CALLBACK (on_env_map_intensity));

  /* Physical material properties */
  add_slider (grid, &row, "Clearcoat",           0, 1, 0, 0.01, G_CALLBACK (on_clearcoat));
  add_slider (grid, &row, "Clearcoat roughness",  0, 1, 0, 0.01, G_CALLBACK (on_clearcoat_roughness));
  add_slider (grid, &row, "IOR",                  1, 2.33, 1.5, 0.01, G_CALLBACK (on_ior));
  add_slider (grid, &row, "Iridescence",          0, 1, 0, 0.01, G_CALLBACK (on_iridescence));
  add_slider (grid, &row, "Iridescence IOR",      1, 2.33, 1.3, 0.01, G_CALLBACK (on_iridescence_ior));
  add_slider (grid, &row, "Iridescence thick min", 100, 800, 100, 1, G_CALLBACK (on_iridescence_thickness_min));
  add_slider (grid, &row, "Iridescence thick max", 100, 800, 400, 1, G_CALLBACK (on_iridescence_thickness_max));
  add_slider (grid, &row, "Sheen",                0, 1, 0, 0.01, G_CALLBACK (on_sheen));
  add_slider (grid, &row, "Sheen roughness",      0, 1, 1, 0.01, G_CALLBACK (on_sheen_roughness));
  add_color (grid, &row, "Sheen color", black (), G_CALLBACK (on_sheen_color));
  add_slider (grid, &row, "Transmission",         0, 1, 0, 0.01, G_CALLBACK (on_transmission));
  add_slider (grid, &row, "Thickness",            0, 10, 0, 0.1, G_CALLBACK (on_thickness));
  add_slider (grid, &row, "Attenuation distance", 0, 100, 0, 0.1, G_CALLBACK (on_attenuation_distance));
  add_color (grid, &row, "Attenuation color", white (), G_CALLBACK (on_attenuation_color));
  add_slider (grid, &row, "Dispersion",           0, 2, 0, 0.01, G_CALLBACK (on_dispersion));
  add_slider (grid, &row, "Specular intensity",   0, 2, 1, 0.01, G_CALLBACK (on_specular_intensity));
  add_color (grid, &row, "Specular color", white (), G_CALLBACK (on_specular_color));
  add_slider (grid, &row, "Anisotropy",           0, 1, 0, 0.01, G_CALLBACK (on_anisotropy));
  add_slider (grid, &row, "Anisotropy rotation",  0, G_PI * 2, 0, 0.01, G_CALLBACK (on_anisotropy_rotation));

  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
