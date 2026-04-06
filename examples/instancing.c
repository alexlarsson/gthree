#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

#define INSTANCE_COUNT 1000

GthreeScene *scene;
GthreeInstancedMesh *instanced_mesh;

GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeMeshPhongMaterial *material;
  GthreeDirectionalLight *directional_light;
  GthreeAmbientLight *ambient_light;
  graphene_vec3_t color;
  int i;

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (1, 1, 1, 1, 1, 1);

  material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material, graphene_vec3_init (&color, 1.0, 1.0, 1.0));

  instanced_mesh = gthree_instanced_mesh_new (geometry, GTHREE_MATERIAL (material), INSTANCE_COUNT);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (instanced_mesh));

  for (i = 0; i < INSTANCE_COUNT; i++)
    {
      graphene_matrix_t matrix;
      graphene_vec3_t instance_color;
      float x, y, z;
      float rx, ry, rz;
      float scale;

      x = g_random_double_range (-20, 20);
      y = g_random_double_range (-20, 20);
      z = g_random_double_range (-20, 20);
      rx = g_random_double_range (0, 2 * G_PI);
      ry = g_random_double_range (0, 2 * G_PI);
      rz = g_random_double_range (0, 2 * G_PI);
      scale = g_random_double_range (0.3, 1.5);

      graphene_matrix_init_scale (&matrix, scale, scale, scale);
      graphene_matrix_rotate_x (&matrix, rx * 180.0 / G_PI);
      graphene_matrix_rotate_y (&matrix, ry * 180.0 / G_PI);
      graphene_matrix_rotate_z (&matrix, rz * 180.0 / G_PI);
      graphene_matrix_translate (&matrix, &GRAPHENE_POINT3D_INIT (x, y, z));

      gthree_instanced_mesh_set_matrix_at (instanced_mesh, i, &matrix);

      graphene_vec3_init (&instance_color,
                          g_random_double_range (0.0, 1.0),
                          g_random_double_range (0.0, 1.0),
                          g_random_double_range (0.0, 1.0));
      gthree_instanced_mesh_set_color_at (instanced_mesh, i, &instance_color);
    }

  directional_light = gthree_directional_light_new (graphene_vec3_init (&color, 1, 1, 1), 0.8);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light), 1, 1, 1);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  ambient_light = gthree_ambient_light_new (graphene_vec3_init (&color, 0.3, 0.3, 0.3));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

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

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  gthree_object_set_rotation_xyz (GTHREE_OBJECT (instanced_mesh),
                                  0.3 * relative_time,
                                  0.5 * relative_time,
                                  0.0);

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
  GthreePerspectiveCamera *camera;
  gboolean done = FALSE;

  window = examples_init ("Instancing", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (50, 1, 1, 100);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 0, 50);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
