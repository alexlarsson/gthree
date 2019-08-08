#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreePoints *points;
GthreePoints *flakes;

static void
init_cube (GthreeScene *scene)
{
  int particles = 500000;

  GthreeAttribute *position = gthree_attribute_new ("position", GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                                    particles, 3, FALSE);
  GthreeAttribute *color = gthree_attribute_new ("color", GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                                 particles, 3, FALSE);

  int n = 1000, n2 = n / 2; // particles spread in the cube

  for (int i = 0; i < particles; i ++ )
    {
      float x = g_random_double () * n - n2;
      float y = g_random_double () * n - n2;
      float z = g_random_double () * n - n2;

      gthree_attribute_set_xyz (position, i, x, y, z);
      gthree_attribute_set_xyz (color, i,
                                ( x / n ) + 0.5,
                                ( y / n ) + 0.5,
                                ( z / n ) + 0.5);
    }

  GthreeGeometry *geometry = gthree_geometry_new ();
  gthree_geometry_add_attribute (geometry, "position", position);
  gthree_geometry_add_attribute (geometry, "color", color);

  GthreePointsMaterial *material = gthree_points_material_new ();
  gthree_points_material_set_size (material, 15);
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material), TRUE);

  points = gthree_points_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (points));
}

static void
init_snow (GthreeScene *scene)
{
  g_autoptr(GdkPixbuf) snowflake_pixbuf = examples_load_pixbuf ("snowflake.png");
  g_autoptr(GthreeTexture) snowflake_texture = gthree_texture_new (snowflake_pixbuf);

  int n_flakes = 10000;

  GthreeAttribute *position = gthree_attribute_new ("position", GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                                    n_flakes, 3, FALSE);

  int n = 5000, n2 = n / 2; // flakes spread in the cube

  for (int i = 0; i < n_flakes; i ++ )
    {
      float x = g_random_double () * n - n2;
      float y = g_random_double () * n - n2;
      float z = g_random_double () * n - n2;

      gthree_attribute_set_xyz (position, i, x, y, z);
    }

  GthreeGeometry *geometry = gthree_geometry_new ();
  gthree_geometry_add_attribute (geometry, "position", position);

  GthreePointsMaterial *material = gthree_points_material_new ();
  gthree_points_material_set_size (material, 150);
  gthree_points_material_set_map (material, snowflake_texture);
  GdkRGBA color = { 0.5, 0.5, 1.0, 1.0 };
  gthree_points_material_set_color (material, &color);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (material), GTHREE_BLEND_NORMAL, 0, 0, 0);

  flakes = gthree_points_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (flakes));
}


GthreeScene *
init_scene (void)
{
  scene = gthree_scene_new ();

  gthree_scene_set_background_color (scene, &blue);

  init_cube (scene);
  init_snow (scene);

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
  graphene_euler_t rot;
  graphene_vec3_t pos;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  if (points)
  gthree_object_set_rotation (GTHREE_OBJECT (points),
                              graphene_euler_init (&rot,
                                                   relative_time * 0.25,
                                                   relative_time * 0.5,
                                                   0));


  if (flakes)
    {
      gthree_object_set_position (GTHREE_OBJECT (flakes),
                                  graphene_vec3_init (&pos,
                                                      sin (relative_time / 100) * 1000,
                                                      sin (0.5 * relative_time / 100) * 1000,
                                                      0));
      gthree_object_set_rotation (GTHREE_OBJECT (flakes),
                                  graphene_euler_init (&rot,
                                                       0,0,
                                                       relative_time * 0.25));
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
  GthreePerspectiveCamera *camera;
  graphene_point3d_t pos;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Points");
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
  camera = gthree_perspective_camera_new (27, 1, 5, 3500);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                                      graphene_point3d_init (&pos, 0, 0, 2750));

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
