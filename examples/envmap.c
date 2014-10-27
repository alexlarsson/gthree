#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMesh *obj1, *obj2;
static GthreePerspectiveCamera *camera;

GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeLambertMaterial *material;
  GthreeCubeTexture *texture;
  GdkPixbuf *pixbufs[6];
  GthreeAmbientLight *ambient_light;
  GthreePointLight *point_light;
  graphene_point3d_t pos;

  examples_load_cube_pixbufs ("cube/SwedishRoyalCastle", pixbufs);

  texture = gthree_cube_texture_new_from_array (pixbufs);

  material = gthree_lambert_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material), &white);
  gthree_lambert_material_set_ambient_color (material, &light_grey);
  gthree_basic_material_set_env_map (GTHREE_BASIC_MATERIAL (material), GTHREE_TEXTURE (texture));

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_sphere (100, 32, 16);
  obj1 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position (GTHREE_OBJECT (obj1),
                              graphene_point3d_init (&pos,
                                                     -80,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj1));

  geometry = gthree_geometry_new_box (100, 100, 100, 1, 1, 1);
  obj2 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position (GTHREE_OBJECT (obj2),
                              graphene_point3d_init (&pos,
                                                     80,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj2));


  ambient_light = gthree_ambient_light_new (&white);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  point_light = gthree_point_light_new (&white, 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  gthree_object_set_position (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos,
                                                     1000,
                                                     800,
                                                     400));

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  graphene_point3d_t pos;
  gint64 frame_time;
  float angle;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 1000000.0;

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos,
                                                     cos (angle) * 500,
                                                     0,
                                                     sin (angle) * 500));
  if (1)
    gthree_object_set_rotation (GTHREE_OBJECT (camera),
                                graphene_point3d_init (&pos, 0, G_PI/2 - angle, 0));
  else
    gthree_object_look_at (GTHREE_OBJECT (camera),
                           graphene_point3d_init (&pos, 0, 0, 0));

  rot.x += 0.01;
  rot.y += 0.005;

  gthree_object_set_rotation (GTHREE_OBJECT (obj2), &rot);

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
  gtk_window_set_title (GTK_WINDOW (window), "Environment map");
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
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 500));

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
