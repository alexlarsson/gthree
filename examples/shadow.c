#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreeScene *scene;
static GthreePerspectiveCamera *camera;
static GthreeAmbientLight *ambient_light;
static GthreeDirectionalLight *directional_light;
static GthreeSpotLight *spot_light;
static GthreePointLight *point_light;
static GthreeMesh *ball;
static GthreeMesh *cube;

static void
init_scene (void)
{
  GthreeGeometry *floor_geometry, *ball_geometry, *cube_geometry;
  GthreeMeshPhongMaterial *ball_material, *cube_material, *floor_material;
  GthreeMeshBasicMaterial *particle_material;
  GthreeGeometry *particle_geometry;
  GthreeMesh *floor, *dir_particle, *spot_particle, *point_particle;
  graphene_point3d_t pos = { 0, 0, 0};

  particle_geometry = gthree_geometry_new_sphere (4, 8, 8);

  scene = gthree_scene_new ();

  camera = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 200, 2000));

  ball_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (ball_material, white ());

  ball_geometry = gthree_geometry_new_sphere (100, 32, 16);
  ball = gthree_mesh_new (ball_geometry, GTHREE_MATERIAL (ball_material));
  gthree_object_set_cast_shadow (GTHREE_OBJECT (ball), TRUE);
  gthree_object_set_position_point3d (GTHREE_OBJECT (ball),
                                      graphene_point3d_init (&pos,
                                                             100,
                                                             90,
                                                             20));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ball));

  cube_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (cube_material, light_grey ());

  cube_geometry = gthree_geometry_new_box (150, 150, 150, 1, 1, 1);
  cube = gthree_mesh_new (cube_geometry, GTHREE_MATERIAL (cube_material));
  gthree_object_set_cast_shadow (GTHREE_OBJECT (cube), TRUE);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube),
                                      graphene_point3d_init (&pos,
                                                             -200,
                                                             50,
                                                             0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube));

  floor_geometry = gthree_geometry_new_box (1000, 10, 1000,
                                            40, 1, 40);
  floor_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (floor_material, white ());

  floor = gthree_mesh_new (floor_geometry, GTHREE_MATERIAL (floor_material));
  gthree_object_set_receive_shadow (GTHREE_OBJECT (floor), TRUE);
  gthree_object_set_position_point3d (GTHREE_OBJECT (floor),
                                      graphene_point3d_init (&pos, 0, -75, 0));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (floor));

  ambient_light = gthree_ambient_light_new (dark_grey ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  /* Directional light */

  directional_light = gthree_directional_light_new (green (), 0.3);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (directional_light), TRUE);
  gthree_object_set_position_point3d (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     0, 200, 200));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  GthreeLightShadow *shadow = gthree_light_get_shadow (GTHREE_LIGHT (directional_light));
  GthreeCamera *shadow_camera = gthree_light_shadow_get_camera (shadow);

  gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -500);
  gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 500);
  gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 500);
  gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -500);

  particle_material = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (particle_material, green ());
  dir_particle = gthree_mesh_new (particle_geometry, GTHREE_MATERIAL (particle_material));
  gthree_object_add_child (GTHREE_OBJECT (directional_light), GTHREE_OBJECT (dir_particle));

  /* Spot light */

  spot_light = gthree_spot_light_new (blue (), 1.5, 5000, G_PI/4, 0.2);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (spot_light), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (spot_light));

  particle_material = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (particle_material, blue ());
  spot_particle = gthree_mesh_new (particle_geometry, GTHREE_MATERIAL (particle_material));
  gthree_object_add_child (GTHREE_OBJECT (spot_light), GTHREE_OBJECT (spot_particle));

  /* Point light */

  point_light = gthree_point_light_new (red (), 0.5, 0);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (point_light), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  particle_material = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (particle_material, red ());
  point_particle = gthree_mesh_new (particle_geometry, GTHREE_MATERIAL (particle_material));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (point_particle));
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_point3d_t pos;
  graphene_euler_t e;
  gint64 frame_time;
  static gint64 first_frame_time = 0;
  float angle;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;
  angle = (frame_time - first_frame_time) / 4000000.0;

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                                      graphene_point3d_init (&pos,
                                                             cos (angle*2) * 1000,
                                                             200,
                                                             sin (angle*2) * 1000));
  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&pos, 0, 0, 0));

  gthree_object_set_rotation (GTHREE_OBJECT (cube),
                              graphene_euler_init (&e,
                                                   angle*300, angle * 123, 0));

  gthree_object_set_position_point3d (GTHREE_OBJECT (spot_light),
                                      graphene_point3d_init (&pos,
                                                             cos (angle*10) * 150,
                                                             150,
                                                             cos (angle*7) * 150));

  gthree_object_set_position_point3d (GTHREE_OBJECT (point_light),
                                      graphene_point3d_init (&pos,
                                                             0,
                                                             150,
                                                             cos (angle*10) * 150));

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height)
{
  gthree_perspective_camera_set_aspect (camera, (float)width / (float)(height));
}

static void
realize_area (GthreeArea *area)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (area);

  gthree_renderer_set_shadow_map_enabled (renderer, TRUE);
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Shadows");
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
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);
  g_signal_connect_after (area, "realize", G_CALLBACK (realize_area), NULL);
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
