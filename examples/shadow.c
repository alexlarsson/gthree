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
  GthreeLightShadow *shadow;
  GthreeCamera *shadow_camera;

  particle_geometry = gthree_geometry_new_sphere (4, 8, 8);

  scene = gthree_scene_new ();

  camera = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  0, 200, 2000);

  ball_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (ball_material, white ());

  ball_geometry = gthree_geometry_new_sphere (100, 32, 16);
  ball = gthree_mesh_new (ball_geometry, GTHREE_MATERIAL (ball_material));
  gthree_object_set_cast_shadow (GTHREE_OBJECT (ball), TRUE);
  gthree_object_set_position_xyz (GTHREE_OBJECT (ball),
                                  100, 90, 20);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ball));

  cube_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (cube_material, light_grey ());

  cube_geometry = gthree_geometry_new_box (150, 150, 150, 1, 1, 1);
  cube = gthree_mesh_new (cube_geometry, GTHREE_MATERIAL (cube_material));
  gthree_object_set_cast_shadow (GTHREE_OBJECT (cube), TRUE);
  gthree_object_set_position_xyz (GTHREE_OBJECT (cube),
                                  -200, 50, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube));

  floor_geometry = gthree_geometry_new_box (1000, 10, 1000,
                                            40, 1, 40);
  floor_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (floor_material, white ());

  floor = gthree_mesh_new (floor_geometry, GTHREE_MATERIAL (floor_material));
  gthree_object_set_receive_shadow (GTHREE_OBJECT (floor), TRUE);
  gthree_object_set_position_xyz (GTHREE_OBJECT (floor),
                                  0, -75, 0);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (floor));

  ambient_light = gthree_ambient_light_new (dark_grey ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  /* Directional light */

  directional_light = gthree_directional_light_new (green (), 0.3);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (directional_light), TRUE);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light),
                                  0, 200, 200);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  shadow = gthree_light_get_shadow (GTHREE_LIGHT (directional_light));
  shadow_camera = gthree_light_shadow_get_camera (shadow);

  gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -500);
  gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 500);
  gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 500);
  gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -500);
  gthree_camera_set_far (shadow_camera, 1000);

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

  shadow = gthree_light_get_shadow (GTHREE_LIGHT (point_light));
  shadow_camera = gthree_light_shadow_get_camera (shadow);

  gthree_camera_set_far (shadow_camera, 1000);
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  gint64 frame_time;
  static gint64 first_frame_time = 0;
  float angle;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;
  angle = (frame_time - first_frame_time) / 4000000.0;

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  cos (angle*2) * 1000,
                                  200,
                                  sin (angle*2) * 1000);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera),
                             0, 0, 0);

  gthree_object_set_rotation_xyz (GTHREE_OBJECT (cube),
                                  angle*300, angle * 123, 0);

  gthree_object_set_position_xyz (GTHREE_OBJECT (spot_light),
                                  cos (angle*10) * 150,
                                  150,
                                  cos (angle*7) * 150);

  gthree_object_set_position_xyz (GTHREE_OBJECT (point_light),
                                  0,
                                  150,
                                  cos (angle*10) * 150);

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
  GtkWidget *window, *box, *area;
  gboolean done = FALSE;

  window = examples_init ("Shadows", &box, &done);

  init_scene ();
  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);
  g_signal_connect_after (area, "realize", G_CALLBACK (realize_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
