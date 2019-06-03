#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GList *cameras;
static GthreeCamera *current_camera;

static GthreeGroup *default_camera_group;
static GthreePerspectiveCamera *default_camera;
static GthreePointLight *point_light;

static void
light_scene (GthreeScene *scene)
{
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;
  GthreeMeshBasicMaterial *material_light;
  GthreeGeometry *geometry_light;
  GthreeMesh *particle_light;
  graphene_point3d_t pos;

  ambient_light = gthree_ambient_light_new (&white);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (4, 8, 8);
  material_light = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_light, &white);

  point_light = gthree_point_light_new (&red, 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (&white, 0.125);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     1, 1, -1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
}


static GthreeScene *
init_scene (const char *path)
{
  g_autoptr(GthreeLoader) loader = NULL;
  GError *error = NULL;
  GthreeScene *scene;

  if (path)
    {
      g_autoptr(GFile) file = g_file_new_for_commandline_arg (path);
      g_autoptr(GFile) parent = g_file_get_parent (file);
      g_autoptr(GBytes) bytes = g_file_load_bytes (file, NULL, NULL, &error);
      if (bytes == NULL)
        g_error ("Failed to load %s: %s\n", path, error->message);

      loader = gthree_loader_parse_gltf (bytes, parent, &error);
      if (loader == NULL)
        g_error ("Failed to %s: %s\n", path, error->message);
    }
  else
    {
      loader = examples_load_gltl ("RobotExpressive.glb", &error);
      if (loader == NULL)
        g_error ("Failed to parse robot model: %s\n", error->message);
    }

  scene = gthree_loader_get_scene (loader, 0);

  return g_object_ref (scene);
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
add_default_camera (GthreeScene *scene)
{
  graphene_box_t bounding_box;
  graphene_sphere_t bounding_sphere;
  graphene_point3d_t bs_center;
  float bs_radius;
  graphene_point3d_t pos;

  /* Generate default camera */
  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), TRUE);

  gthree_object_get_mesh_extents (GTHREE_OBJECT (scene), &bounding_box);

  box_get_bounding_sphere (&bounding_box, &bounding_sphere);
  graphene_sphere_get_center (&bounding_sphere, &bs_center);
  bs_radius = graphene_sphere_get_radius (&bounding_sphere);

  default_camera_group = gthree_group_new ();
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (default_camera_group));
  gthree_object_set_position (GTHREE_OBJECT (default_camera_group), &bs_center);

  default_camera = gthree_perspective_camera_new (37, 1.5, bs_radius / 1000, bs_radius * 1000);
  gthree_object_set_position (GTHREE_OBJECT (default_camera),
                              graphene_point3d_init (&pos, 0, 0, 3 * bs_radius));

  gthree_object_add_child (GTHREE_OBJECT (default_camera_group), GTHREE_OBJECT (default_camera));
  cameras = g_list_prepend (cameras, default_camera);
}


static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_point3d_t pos;
  graphene_euler_t rot;
  gint64 frame_time;
  float angle;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 4000000.0;

  gthree_object_set_position (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos,
                                                     sin (angle * 7) * 300,
                                                     cos (angle * 5) * 400,
                                                     cos (angle * 3) * 300));

  gthree_object_set_rotation (GTHREE_OBJECT (default_camera_group),
                              graphene_euler_init (&rot, 0, angle * 150, 0));

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height)
{
  if (GTHREE_IS_PERSPECTIVE_CAMERA (current_camera))
    gthree_perspective_camera_set_aspect (GTHREE_PERSPECTIVE_CAMERA (current_camera), (float)width / (float)(height));
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Models");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (box), 6);
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show (box);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_container_add (GTK_CONTAINER (box), hbox);
  gtk_widget_show (hbox);

  scene = init_scene (argc == 2 ? argv[1] : NULL);

  cameras = gthree_object_find_by_type (GTHREE_OBJECT (scene), GTHREE_TYPE_CAMERA);
  add_default_camera (scene);

  // Must be after default camera creation to avoid the added geometry affecting the size calculation
  light_scene (scene);

  current_camera = GTHREE_CAMERA (cameras->data);

  area = gthree_area_new (scene, current_camera);
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);

  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
