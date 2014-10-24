#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>

GthreeScene *scene;
GthreePerspectiveCamera *camera;
GthreeMesh *particle_light;

GList *objects;

GthreeScene *
init_scene (void)
{
  GthreeGeometry *floor_geometry, *geometry_smooth, *geometry_light;// *geometry_flat, *geometry_pieces;
  GthreeBasicMaterial *material_wireframe, *material_light;
  GthreeLambertMaterial *material_lambert;
  GthreePhongMaterial *material_phong;
  GthreeAmbientLight *ambient_light;
  GthreePointLight *point_light;
  GthreeDirectionalLight *directional_light;
  GthreeMesh *floor;
  int i;
  GdkRGBA grey = {0.4, 0.4, 0.4, 1.0};
  GdkRGBA white = {1, 1, 1, 1.0};
  GdkRGBA red = {1, 0, 0, 1.0};
  GdkRGBA green = {0, 1, 0, 1.0};
  GdkRGBA dark_grey = {0.1, 0.1, 0.1, 1.0};
  graphene_point3d_t pos = { 0, 0, 0};
  GthreeMaterial *materials[14] = { NULL };
  int n_materials = 0;

  scene = gthree_scene_new ();

  camera = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 200, 2000));

  material_wireframe = gthree_basic_material_new ();
  gthree_material_set_is_wireframe (GTHREE_MATERIAL (material_wireframe), TRUE);
  gthree_basic_material_set_color (material_wireframe, &grey);
    materials[n_materials++] = GTHREE_MATERIAL (material_wireframe);
  
  material_lambert = gthree_lambert_material_new ();
  gthree_lambert_material_set_color (material_lambert, &white);
  gthree_lambert_material_set_shading_type (material_lambert,
					    GTHREE_SHADING_FLAT);
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_lambert = gthree_lambert_material_new ();
  gthree_lambert_material_set_color (material_lambert, &white);
  gthree_lambert_material_set_shading_type (material_lambert,
					    GTHREE_SHADING_SMOOTH);
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_phong = gthree_phong_material_new ();
  gthree_phong_material_set_color (material_phong, &white);
  gthree_phong_material_set_shading_type (material_phong,
					  GTHREE_SHADING_FLAT);
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_phong = gthree_phong_material_new ();
  gthree_phong_material_set_color (material_phong, &white);
  gthree_phong_material_set_shading_type (material_phong,
					  GTHREE_SHADING_SMOOTH);
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  floor_geometry = gthree_geometry_new_box (1000, 10, 1000,
                                            40, 1, 40);

  floor = gthree_mesh_new (floor_geometry, GTHREE_MATERIAL (material_wireframe));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (floor));
  gthree_object_set_position (GTHREE_OBJECT (floor),
                              graphene_point3d_init (&pos, 0, -75, 0));


  geometry_smooth = gthree_geometry_new_sphere (70, 32, 16);
  //geometry_flat = gthree_geometry_new_sphere (70, 32, 16);
  //geometry_pieces = gthree_geometry_new_sphere (70, 32, 16);

  for (i = 0; i < 16; i++)
    {
      GthreeMesh *sphere = gthree_mesh_new (geometry_smooth, materials[i % n_materials]);
      gthree_object_set_position (GTHREE_OBJECT (sphere),
                                  graphene_point3d_init (&pos,
                                                         (i % 4 ) * 200 - 400,
                                                         0,
                                                         (i / 4) * 200 - 200));
      gthree_object_set_rotation (GTHREE_OBJECT (sphere),
                                  graphene_point3d_init (&pos,
                                                         g_random_double_range (0, 2*G_PI),
                                                         g_random_double_range (0, 2*G_PI),
                                                         g_random_double_range (0, 2*G_PI)));

      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (sphere));

      objects = g_list_prepend (objects, sphere);
    }

  ambient_light = gthree_ambient_light_new (&dark_grey);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (4, 8, 8);
  material_light = gthree_basic_material_new ();
  gthree_basic_material_set_color (material_light, &white);
  
  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (particle_light));

  point_light = gthree_point_light_new (&red, 0.25, 0);
  gthree_object_add_child (GTHREE_OBJECT (particle_light), GTHREE_OBJECT (point_light));
  
  directional_light = gthree_directional_light_new (&green, 0.125);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
			      graphene_point3d_init (&pos,
						     1,
						     1,
						     -1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
  
  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_point3d_t pos;
  graphene_point3d_t rot;
  GList *l;
  gint64 frame_time;
  float angle;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 4000000.0;

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos,
                                                     cos (angle) * 1200,
                                                     200,
                                                     sin (angle) * 1200));
  if (1)
    gthree_object_set_rotation (GTHREE_OBJECT (camera),
                                graphene_point3d_init (&pos, 0, G_PI/2 - angle, 0));
  else
    gthree_object_look_at (GTHREE_OBJECT (camera),
                           graphene_point3d_init (&pos, 0, 0, 0));

  for (l = objects; l != NULL; l = l->next)
    {
      GthreeObject *object = l->data;
      rot = *gthree_object_get_rotation (object);
      rot.x += 0.01;
      rot.y += 0.005;
      gthree_object_set_rotation (object, &rot);
    }

  gthree_object_set_position (GTHREE_OBJECT (particle_light),
                              graphene_point3d_init (&pos,
                                                     sin (angle * 7) * 300,
                                                     cos (angle * 5) * 400,
                                                     cos (angle * 3) * 300));


  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Model");
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

  init_scene ();
  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
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
