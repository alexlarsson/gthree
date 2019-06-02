#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreePerspectiveCamera *camera;
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


GthreeScene *
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

  light_scene (scene);

  return g_object_ref (scene);
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_point3d_t pos;
  gint64 frame_time;
  float angle;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 4000000.0;

  gthree_object_set_position (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos,
                                                     sin (angle * 7) * 300,
                                                     cos (angle * 5) * 400,
                                                     cos (angle * 3) * 300));

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height,
             GthreePerspectiveCamera *camera)
{
  gthree_perspective_camera_set_aspect (camera, (float)width / (float)(height));
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;
  graphene_point3d_t pos;
  GList *cameras;

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

  cameras = NULL;//gthree_object_find_by_type (GTHREE_OBJECT (scene), GTHREE_TYPE_CAMERA);

  if (cameras == NULL)
    {
      g_warning ("No camera, adding default one");
      camera = gthree_perspective_camera_new (37, 1.5, 1, 10000);
      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

      gthree_object_set_position (GTHREE_OBJECT (camera),
                                  graphene_point3d_init (&pos, 10, 2, 4));
      gthree_object_look_at (GTHREE_OBJECT (camera),
                             graphene_point3d_init (&pos, 0, 0, 0));

      cameras = g_list_prepend (cameras, camera);
    }

  camera = cameras->data; // Use first camera

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
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
