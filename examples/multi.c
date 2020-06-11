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
  GthreeMeshBasicMaterial *material;
  GthreeGeometry *geometry;
  GthreeTexture *texture;
  graphene_point3d_t pos = { 0, 0, 0};

  crate_pixbuf = examples_load_pixbuf ("crate.gif");

  texture = gthree_texture_new (crate_pixbuf);

  material = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material), FALSE);
  gthree_mesh_basic_material_set_map (material, texture);

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (100*2, 100*2, 100*2, 1, 1, 1);
  cube = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube));

  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};

  rot.y += 1;
  rot.z += 0.25;

  gthree_object_set_rotation_xyz (GTHREE_OBJECT (cube),
                                  rot.x, rot.y, rot.z);

  gtk_widget_queue_draw (area_z);
  gtk_widget_queue_draw (area_y);
  gtk_widget_queue_draw (area_x);

  return G_SOURCE_CONTINUE;
}

static void
clicked_z (GtkEventController *controller,
           gint                n_press,
           gdouble             x,
           gdouble             y,
           gpointer            user_data)
{
  GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller));
  graphene_point3d_t point = {0, 0, 0}, a, b, pos;
  float w, h, u;

  w = gtk_widget_get_allocated_width (widget);
  h = gtk_widget_get_allocated_height (widget);

  graphene_point3d_init_from_vec3 (&pos,
                                   gthree_object_get_position (GTHREE_OBJECT (cube)));

  point.x = x / (0.5*w) - 1;
  point.y = -y / (0.5*h) + 1;

  point.z  = 0.0f;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_z), &point, &a);
  point.z = 1.0;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_z), &point, &b);

  u = (pos.z - a.z) / (b.z - a.z);

  point.x = a.x + u * (b.x - a.x);
  point.y = a.y + u * (b.y - a.y);
  point.z = pos.z;

  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &point);
}

static void
clicked_y (GtkEventController *controller,
           gint                n_press,
           gdouble             x,
           gdouble             y,
           gpointer            user_data)
{
  GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller));
  graphene_point3d_t point = {0, 0, 0}, a, b, pos;
  float w, h, u;

  w = gtk_widget_get_allocated_width (widget);
  h = gtk_widget_get_allocated_height (widget);

  graphene_point3d_init_from_vec3 (&pos,
                                   gthree_object_get_position (GTHREE_OBJECT (cube)));

  point.x = x / (0.5*w) - 1;
  point.y = -y / (0.5*h) + 1;

  point.z  = 0.0f;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_y), &point, &a);
  point.z = 1.0;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_y), &point, &b);

  u = (pos.y - a.y) / (b.y - a.y);

  point.x = a.x + u * (b.x - a.x);
  point.y = pos.y;
  point.z = a.z + u * (b.z - a.z);

  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &point);
}

static void
clicked_x (GtkEventController *controller,
           gint                n_press,
           gdouble             x,
           gdouble             y,
           gpointer            user_data)
{
  GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller));
  graphene_point3d_t point = {0, 0, 0}, a, b, pos;
  float w, h, u;

  w = gtk_widget_get_allocated_width (widget);
  h = gtk_widget_get_allocated_height (widget);

  graphene_point3d_init_from_vec3 (&pos,
                                   gthree_object_get_position (GTHREE_OBJECT (cube)));

  point.x = x / (0.5*w) - 1;
  point.y = -y / (0.5*h) + 1;

  point.z  = 0.0f;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_x), &point, &a);
  point.z = 1.0;
  gthree_camera_unproject_point3d (GTHREE_CAMERA (camera_x), &point, &b);

  u = (pos.x - a.x) / (b.x - a.x);

  point.x = pos.x;
  point.y = a.y + u * (b.y - a.y);
  point.z = a.z + u * (b.z - a.z);

  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &point);
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
  GtkWidget *window, *box, *grid;
  GthreeScene *scene;
  GtkEventController *click;
  gboolean done = FALSE;

  window = examples_init ("Multi views", &box, &done);

  scene = init_scene ();

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (box), grid);
  gtk_widget_show (grid);
  gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 6);


  camera_z = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_z));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera_z),
                                  0, 0, 1000);

  area_z = gthree_area_new (scene, GTHREE_CAMERA (camera_z));
  g_signal_connect (area_z, "resize", G_CALLBACK (resize_area), camera_z);
  gtk_widget_set_hexpand (area_z, TRUE);
  gtk_widget_set_vexpand (area_z, TRUE);
  gtk_grid_attach (GTK_GRID (grid), area_z,
                   0, 1, 1, 1);
  gtk_widget_show (area_z);

  click = click_controller_for (area_z);
  g_signal_connect (click, "pressed", G_CALLBACK (clicked_z), NULL);

  camera_y = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_y));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera_y),
                                  0, 1000, 0);
  gthree_object_set_rotation_xyz (GTHREE_OBJECT (camera_y),
                                  -90, 0, 0);

  area_y = gthree_area_new (scene, GTHREE_CAMERA (camera_y));
  g_signal_connect (area_y, "resize", G_CALLBACK (resize_area), camera_y);
  gtk_widget_set_hexpand (area_y, TRUE);
  gtk_widget_set_vexpand (area_y, TRUE);
  gtk_grid_attach (GTK_GRID (grid), area_y,
                   0, 0, 1, 1);
  gtk_widget_show (area_y);

  click = click_controller_for (area_y);
  g_signal_connect (click, "pressed", G_CALLBACK (clicked_y), NULL);

  camera_x = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera_x));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera_x),
                                  1000, 0, 0);
  gthree_object_set_rotation_xyz (GTHREE_OBJECT (camera_x),
                                  0, -90, 0);

  area_x = gthree_area_new (scene, GTHREE_CAMERA (camera_x));
  g_signal_connect (area_x, "resize", G_CALLBACK (resize_area), camera_x);
  gtk_widget_set_hexpand (area_x, TRUE);
  gtk_widget_set_vexpand (area_x, TRUE);
  gtk_grid_attach (GTK_GRID (grid), area_x,
                   1, 1, 1, 1);
  gtk_widget_show (area_x);

  click = click_controller_for (area_x);
  g_signal_connect (click, "pressed", G_CALLBACK (clicked_x), NULL);

  gtk_widget_add_tick_callback (GTK_WIDGET (area_z), tick, NULL, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
