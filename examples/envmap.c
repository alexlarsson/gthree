#include <stdlib.h>
#include <gtk/gtk.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene, *scene_cube;
GthreeMesh *obj1, *obj2, *obj3;
static GthreePerspectiveCamera *camera;

GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeMeshLambertMaterial *material, *material2;
  GthreeCubeTexture *reflectionCube, *refractionCube;
  GdkPixbuf *pixbufs[6];
  GthreeAmbientLight *ambient_light;
  GthreePointLight *point_light;
  graphene_point3d_t pos;

  examples_load_cube_pixbufs ("cube/SwedishRoyalCastle", pixbufs);

  reflectionCube = gthree_cube_texture_new_from_array (pixbufs);

  refractionCube = gthree_cube_texture_new_from_array (pixbufs);
  gthree_texture_set_mapping (GTHREE_TEXTURE (refractionCube), GTHREE_MAPPING_CUBE_REFRACTION);

  material = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material, white ());
  gthree_mesh_lambert_material_set_env_map (material, GTHREE_TEXTURE (reflectionCube));

  material2 = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material2, yellow ());
  gthree_mesh_lambert_material_set_refraction_ratio (material2, 0.99);
  gthree_mesh_lambert_material_set_env_map (material2, GTHREE_TEXTURE (refractionCube));

  scene = gthree_scene_new ();
  gthree_scene_set_background_texture (scene, GTHREE_TEXTURE (reflectionCube));

  geometry = gthree_geometry_new_sphere (40, 32, 16);
  obj1 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position_point3d (GTHREE_OBJECT (obj1),
                              graphene_point3d_init (&pos,
                                                     -80,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj1));

  geometry = gthree_geometry_new_sphere (40, 32, 16);
  obj2 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material2));
  gthree_object_set_position_point3d (GTHREE_OBJECT (obj2),
                              graphene_point3d_init (&pos,
                                                     0,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj2));

  geometry = gthree_geometry_new_box (70, 70, 70, 1, 1, 1);
  obj3 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position_point3d (GTHREE_OBJECT (obj3),
                              graphene_point3d_init (&pos,
                                                     80,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj3));

  ambient_light = gthree_ambient_light_new (white ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  point_light = gthree_point_light_new (white (), 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  gthree_object_set_position_point3d (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos,
                                                     1000,
                                                     800,
                                                     400));

  return scene;
}

static int cursor_x, cursor_y;

static void
motion_cb (GtkEventControllerMotion *controller,
           gdouble                   x,
           gdouble                   y,
           gpointer                  user_data)
{
  cursor_x = x;
  cursor_y = y;
}


static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  graphene_euler_t euler;
  graphene_point3d_t pos;
  float relative_time, camera_angle, camera_height;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  /* Control camera with mouse */
  camera_angle = (cursor_x)  * 2.0 * G_PI / gtk_widget_get_allocated_width (widget) - G_PI / 2.0;
  camera_height = (((float)cursor_y / gtk_widget_get_allocated_height (widget)) - 0.5) * 500;

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos,
                                                     cos (camera_angle) * 500,
                                                     camera_height,
                                                     sin (camera_angle) * 500));
  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&pos, 0, 0, 0));

  gthree_object_set_rotation (GTHREE_OBJECT (obj3),
                              graphene_euler_init (&euler,
                                                   0.5 * relative_time,
                                                   0.2 * relative_time,
                                                   0.0 * relative_time
                                                   ));

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
  GtkWidget *window, *box, *area;
  GthreeScene *scene;
  graphene_point3d_t pos;
  GtkEventController *motion;
  gboolean done = FALSE;

  window = examples_init ("Environment map", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 5000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 500));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (box), area);
  gtk_widget_show (area);

  motion = motion_controller_for (GTK_WIDGET (area));
  g_signal_connect (motion, "motion", (GCallback)motion_cb, NULL);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
