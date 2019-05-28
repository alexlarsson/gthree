#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;

GthreeMesh *marine, *knight;


GthreeScene *
init_scene (void)
{
  GthreeGeometry *marine_geometry, *knight_geometry;
  GthreeBasicMaterial *material_wireframe, *material_texture;
  GthreePhongMaterial *material_phong;
  GthreeTexture *texture;
  GdkPixbuf *pixbuf;
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;
  graphene_point3d_t pos;
  graphene_point3d_t scale = {15,15,15};

  pixbuf = examples_load_pixbuf ("MarineCv2_color.jpg");

  texture = gthree_texture_new (pixbuf);

  material_wireframe = gthree_basic_material_new ();
  gthree_material_set_is_wireframe (GTHREE_MATERIAL (material_wireframe), TRUE);
  gthree_basic_material_set_color (material_wireframe, &yellow);
  gthree_basic_material_set_vertex_colors (material_wireframe, FALSE);

  material_phong = gthree_phong_material_new ();
  gthree_basic_material_set_color (material_wireframe, &red);
  gthree_phong_material_set_emissive_color (material_phong, &grey);
  gthree_phong_material_set_specular_color (material_phong, &white);

  material_texture = gthree_basic_material_new ();
  gthree_basic_material_set_vertex_colors (material_texture, FALSE);
  gthree_basic_material_set_map (material_texture, texture);

  scene = gthree_scene_new ();

  marine_geometry = examples_load_geometry ("marine.js");

  marine = gthree_mesh_new (marine_geometry, GTHREE_MATERIAL (material_texture));
  gthree_object_set_position (GTHREE_OBJECT (marine),
			      graphene_point3d_init (&pos,
						     80,
						     -80,
						     0));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (marine));

  knight_geometry = examples_load_geometry ("knight.js");

  knight = gthree_mesh_new (knight_geometry, GTHREE_MATERIAL (material_phong));
  gthree_object_set_position (GTHREE_OBJECT (knight),
			      graphene_point3d_init (&pos,
						     -80,
						     -80,
						     0));

  gthree_object_set_scale (GTHREE_OBJECT (knight), &scale);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (knight));

  ambient_light = gthree_ambient_light_new (&dark_grey);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));
  
  directional_light = gthree_directional_light_new (&white, 0.125);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
			      graphene_point3d_init (&pos,
						     0,
						     1,
						     1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
  
  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  static graphene_point3d_t rot2 = { 0, 0, 0};
  graphene_euler_t euler;

  rot.y += 1.0;
  rot2.y += 0.7;

  gthree_object_set_rotation (GTHREE_OBJECT (marine),
                              graphene_euler_init (&euler,
                                                   rot.x, rot.y, rot.z));
  gthree_object_set_rotation (GTHREE_OBJECT (knight),
                              graphene_euler_init (&euler,
                                                   rot2.x, rot2.y, rot2.z));

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
