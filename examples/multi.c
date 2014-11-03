#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMesh *cube;
GtkWidget *area_z, *area_y, *area_x;
GthreePerspectiveCamera *camera_z, *camera_y, *camera_x;
double rot = 0;

GList *cubes;


GthreeScene *
init_scene (void)
{
  GdkPixbuf *crate_pixbuf;
  GthreeBasicMaterial *material;
  GthreeGeometry *geometry;
  GthreeTexture *texture;
  graphene_point3d_t pos = { 0, 0, 0};

  crate_pixbuf = examples_load_pixbuf ("crate.gif");

  texture = gthree_texture_new (crate_pixbuf);

  material = gthree_basic_material_new ();
  gthree_basic_material_set_vertex_colors (material, GTHREE_COLOR_NONE);
  gthree_basic_material_set_map (material, texture);

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (100*2, 100*2, 100*2, 1, 1, 1);
  cube = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube));

  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};

  rot.y += 0.04;
  rot.z += 0.01;

  gthree_object_set_rotation (GTHREE_OBJECT (cube), &rot);

  gtk_widget_queue_draw (area_z);
  gtk_widget_queue_draw (area_y);
  gtk_widget_queue_draw (area_x);

  return G_SOURCE_CONTINUE;
}

static gboolean
clicked_z (GtkWidget      *widget,
           GdkEventButton *event)
{
  graphene_point3d_t point = {0, 0, 0}, a, b, pos;
  float w, h, u;

  w = gtk_widget_get_allocated_width (widget);
  h = gtk_widget_get_allocated_height (widget);

  gthree_object_get_position (GTHREE_OBJECT (cube), &pos);

  point.x = event->x / (0.5*w) - 1;
  point.y = -event->y / (0.5*h) + 1;

  point.z  = 0.0f;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_z), &point, &a);
  point.z = 1.0;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_z), &point, &b);

  u = (pos.z - a.z) / (b.z - a.z);

  point.x = a.x + u * (b.x - a.x);
  point.y = a.y + u * (b.y - a.y);
  point.z = pos.z;

  gthree_object_set_position (GTHREE_OBJECT (cube), &point);

  return TRUE;
}

static gboolean
clicked_y (GtkWidget      *widget,
           GdkEventButton *event)
{
  graphene_point3d_t point = {0, 0, 0}, a, b, pos;
  float w, h, u;

  w = gtk_widget_get_allocated_width (widget);
  h = gtk_widget_get_allocated_height (widget);

  gthree_object_get_position (GTHREE_OBJECT (cube), &pos);

  point.x = event->x / (0.5*w) - 1;
  point.y = -event->y / (0.5*h) + 1;

  point.z  = 0.0f;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_y), &point, &a);
  point.z = 1.0;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_y), &point, &b);

  u = (pos.y - a.y) / (b.y - a.y);

  point.x = a.x + u * (b.x - a.x);
  point.y = pos.y;
  point.z = a.z + u * (b.z - a.z);

  gthree_object_set_position (GTHREE_OBJECT (cube), &point);

  return TRUE;
}

static gboolean
clicked_x (GtkWidget      *widget,
           GdkEventButton *event)
{
  graphene_point3d_t point = {0, 0, 0}, a, b, pos;
  float w, h, u;

  w = gtk_widget_get_allocated_width (widget);
  h = gtk_widget_get_allocated_height (widget);

  gthree_object_get_position (GTHREE_OBJECT (cube), &pos);

  point.x = event->x / (0.5*w) - 1;
  point.y = -event->y / (0.5*h) + 1;

  point.z  = 0.0f;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_x), &point, &a);
  point.z = 1.0;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_x), &point, &b);

  u = (pos.x - a.x) / (b.x - a.x);

  point.x = pos.x;
  point.y = a.y + u * (b.y - a.y);
  point.z = a.z + u * (b.z - a.z);

  gthree_object_set_position (GTHREE_OBJECT (cube), &point);

  return TRUE;
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
  GtkWidget *window, *box, *hbox, *button, *grid;
  GthreeScene *scene;
  graphene_point3d_t pos;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Cubes");
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

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (hbox), grid);
  gtk_widget_show (grid);
  gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 6);


  camera_z = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_z));

  gthree_object_set_position (GTHREE_OBJECT (camera_z),
                              graphene_point3d_init (&pos, 0, 0, 1000));

  area_z = gthree_area_new (scene, GTHREE_CAMERA (camera_z));
  g_signal_connect (area_z, "resize", G_CALLBACK (resize_area), camera_z);
  gtk_widget_add_events (GTK_WIDGET (area_z), GDK_BUTTON_PRESS_MASK);
  g_signal_connect (area_z, "button-press-event", G_CALLBACK (clicked_z), NULL);
  gtk_widget_set_hexpand (area_z, TRUE);
  gtk_widget_set_vexpand (area_z, TRUE);
  gtk_grid_attach (GTK_GRID (grid), area_z,
                   0, 1, 1, 1);
  gtk_widget_show (area_z);

  camera_y = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_y));

  gthree_object_set_position (GTHREE_OBJECT (camera_y),
                              graphene_point3d_init (&pos, 0, 1000, 0));
  gthree_object_set_rotation (GTHREE_OBJECT (camera_y),
                              graphene_point3d_init (&pos, -G_PI/2, 0, 0));

  area_y = gthree_area_new (scene, GTHREE_CAMERA (camera_y));
  g_signal_connect (area_y, "resize", G_CALLBACK (resize_area), camera_y);
  gtk_widget_add_events (GTK_WIDGET (area_y), GDK_BUTTON_PRESS_MASK);
  g_signal_connect (area_y, "button-press-event", G_CALLBACK (clicked_y), NULL);
  gtk_widget_set_hexpand (area_y, TRUE);
  gtk_widget_set_vexpand (area_y, TRUE);
  gtk_grid_attach (GTK_GRID (grid), area_y,
                   0, 0, 1, 1);
  gtk_widget_show (area_y);

  camera_x = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_x));

  gthree_object_set_position (GTHREE_OBJECT (camera_x),
                              graphene_point3d_init (&pos, 1000, 0, 0));
  gthree_object_set_rotation (GTHREE_OBJECT (camera_x),
                              graphene_point3d_init (&pos, 0, G_PI/2, 0));

  area_x = gthree_area_new (scene, GTHREE_CAMERA (camera_x));
  g_signal_connect (area_x, "resize", G_CALLBACK (resize_area), camera_x);
  gtk_widget_add_events (GTK_WIDGET (area_x), GDK_BUTTON_PRESS_MASK);
  g_signal_connect (area_x, "button-press-event", G_CALLBACK (clicked_x), NULL);
  gtk_widget_set_hexpand (area_x, TRUE);
  gtk_widget_set_vexpand (area_x, TRUE);
  gtk_grid_attach (GTK_GRID (grid), area_x,
                   1, 1, 1, 1);
  gtk_widget_show (area_x);

  gtk_widget_add_tick_callback (GTK_WIDGET (area_z), tick, NULL, NULL);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
