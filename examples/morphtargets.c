#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMeshBasicMaterial *material_wireframe;
GthreeMesh *mesh;
double rot = 0;

GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeAttribute *morph1;

  material_wireframe = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material_wireframe), TRUE);
  gthree_mesh_basic_material_set_color (material_wireframe, &yellow);
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_wireframe), FALSE);

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (80, 80, 80, 1, 1, 1);

  morph1 = gthree_attribute_copy ("name1", gthree_geometry_get_position (geometry));
  /* Corner 1 */
  gthree_attribute_set_xyz (morph1, 0, 60, 60, 60);
  gthree_attribute_set_xyz (morph1, 11, 60, 60, 60);
  gthree_attribute_set_xyz (morph1, 17, 60, 60, 60);

  /* Corner 2 */
  gthree_attribute_set_xyz (morph1, 2, 20, -20, 20);
  gthree_attribute_set_xyz (morph1, 13, 20, -20, 20);
  gthree_attribute_set_xyz (morph1, 19, 20, -20, 20);

  gthree_geometry_add_morph_attribute (geometry, "position",
                                       morph1);

  gthree_mesh_material_set_morph_targets (GTHREE_MESH_MATERIAL (material_wireframe), TRUE);

  mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material_wireframe));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

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
  graphene_euler_t euler;
  GArray *morph_targets;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                              graphene_euler_init (&euler,
                                                   0.0 * relative_time,
                                                   1.0 * relative_time,
                                                   0.4 * relative_time
                                                   ));

  morph_targets = gthree_mesh_get_morph_targets (mesh);
  g_array_index (morph_targets, float, 0) = sin (relative_time / 10) / 2 + 0.5;

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
  gtk_window_set_title (GTK_WINDOW (window), "Morph targets");
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
