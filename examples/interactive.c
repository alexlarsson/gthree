#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreePerspectiveCamera *camera;
GthreeObject *intersected;
graphene_vec3_t intersected_color;

GthreeScene *
init_scene (void)
{
  graphene_vec3_t color, pos, scale;
  graphene_euler_t rotation;
  int i;

  g_autoptr(GthreeGeometry) geometry = gthree_geometry_new_box (20, 42, 20, 1, 1, 1);
  g_autoptr(GthreeDirectionalLight) directional_light = NULL;
  g_autoptr(GthreeAmbientLight) ambient_light = NULL;

  scene = gthree_scene_new ();

  for (i = 0; i < 2000; i++)
    {
      g_autoptr(GthreeMesh) mesh = gthree_mesh_new (geometry, NULL);
      g_object_set_data (mesh, "index", GINT_TO_POINTER(i));

      graphene_vec3_init (&color,
                          g_random_double (),
                          g_random_double (),
                          g_random_double ());
      graphene_vec3_init (&pos,
                          g_random_double_range (-400, 400),
                          g_random_double_range (-400, 400),
                          g_random_double_range (-400, 400));
      graphene_euler_init (&rotation,
                           g_random_double_range (0, 360),
                           g_random_double_range (0, 360),
                           g_random_double_range (0, 360));
      graphene_vec3_init (&scale,
                          g_random_double_range (0.5, 1.5),
                          g_random_double_range (0.5, 1.5),
                          g_random_double_range (0.5, 1.5));

      for (int j = 0; j < 6; j++)
        {
          g_autoptr(GthreeMeshLambertMaterial) material =  gthree_mesh_lambert_material_new ();

          gthree_mesh_lambert_material_set_color (material, &color);

          gthree_mesh_set_material (mesh, j, GTHREE_MATERIAL (material));
        }

      gthree_object_set_position (GTHREE_OBJECT (mesh), &pos);
      gthree_object_set_scale (GTHREE_OBJECT (mesh), &scale);
      gthree_object_set_rotation (GTHREE_OBJECT (mesh), &rotation);

      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));
    }

  directional_light = gthree_directional_light_new (white (), 1.0);
  graphene_vec3_init (&pos, 1, 1, 1);
  gthree_object_set_position (GTHREE_OBJECT (directional_light), &pos);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  ambient_light = gthree_ambient_light_new (dark_grey ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float relative_time;
  graphene_vec3_t pos;
  graphene_point3d_t point;
  int cursor_x, cursor_y;
  float x, y;
  g_autoptr(GthreeRaycaster) raycaster = gthree_raycaster_new ();
  g_autoptr(GPtrArray) intersections = NULL;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  float radius = 100;
  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_vec3_init (&pos,
                                                  radius * sinf (relative_time / 200),
                                                  radius * sinf (relative_time / 200),
                                                  radius * cosf (relative_time / 200)));

  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&point,
                                                0, 0, 0));
  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), FALSE);

  gtk_widget_get_pointer (widget, &cursor_x, &cursor_y);
  x = ((float)cursor_x / gtk_widget_get_allocated_width (widget)) * 2 - 1;
  y = -((float)cursor_y / gtk_widget_get_allocated_height (widget)) * 2 + 1;

  gthree_raycaster_set_from_camera  (raycaster, GTHREE_CAMERA (camera), x, y);

  intersections = gthree_raycaster_intersect_object (raycaster, GTHREE_OBJECT (scene), TRUE, NULL);
  if (intersected)
    {
      for (int j = 0; j < 6; j++)
        {
          GthreeMaterial *material = gthree_mesh_get_material (GTHREE_MESH (intersected), j);
          gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), &intersected_color);
        }
      intersected = NULL;
    }

  if (intersections->len > 0)
    {
      GthreeRayIntersection *intersection = g_ptr_array_index (intersections, 0);
      intersected = intersection->object;
      intersected_color = *gthree_mesh_lambert_material_get_color (GTHREE_MESH_LAMBERT_MATERIAL (gthree_mesh_get_material (GTHREE_MESH (intersected), 0)));
      for (int j = 0; j < 6; j++)
        {
          GthreeMaterial *material = gthree_mesh_get_material (GTHREE_MESH (intersected), j);
          if (j == intersection->material_index)
            gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), green ());
          else
            gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), red ());
        }
    }

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

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Interactive");
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

  scene = init_scene ();
  camera = gthree_perspective_camera_new (70, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));

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
