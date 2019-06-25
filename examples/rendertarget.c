#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMesh *mesh;
GthreePerspectiveCamera *camera;
GthreeMeshBasicMaterial *material;

GthreeScene *scene2;
GthreeMesh *mesh2;
GthreePerspectiveCamera *camera2;
GthreeMeshBasicMaterial *material2;

GthreeRenderTarget *render_target;

void
init_scene (void)
{
  GthreeGeometry *geometry;
  graphene_point3d_t pos;

  geometry = gthree_geometry_new_box (80, 80, 80, 1, 1, 1);

  scene = gthree_scene_new ();

  material = gthree_mesh_basic_material_new ();
  mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));


  scene2 = gthree_scene_new ();

  material2 = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material2, &blue);

  mesh2 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material2));

  gthree_object_add_child (GTHREE_OBJECT (scene2), GTHREE_OBJECT (mesh2));

  camera2 = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_set_position (GTHREE_OBJECT (camera2),
                              graphene_point3d_init (&pos, 0, 0, 400));
  gthree_object_add_child (GTHREE_OBJECT (scene2), GTHREE_OBJECT (camera2));

  render_target = gthree_render_target_new (256, 256);
  gthree_resource_use (GTHREE_RESOURCE (render_target));
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

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                              graphene_euler_init (&euler,
                                                   0.0 * relative_time,
                                                   2.0 * relative_time,
                                                   1.0 * relative_time
                                                   ));

  gthree_object_set_rotation (GTHREE_OBJECT (mesh2),
                              graphene_euler_init (&euler,
                                                   2.0 * relative_time,
                                                   0.0 * relative_time,
                                                   1.0 * relative_time
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

static gboolean
render_area (GtkGLArea    *gl_area,
             GdkGLContext *context)
{
  gthree_renderer_set_clear_color (gthree_area_get_renderer (GTHREE_AREA (gl_area)), &red);

  gthree_renderer_set_render_target (gthree_area_get_renderer (GTHREE_AREA (gl_area)),
                                     render_target, 0, 0);
  gthree_renderer_render (gthree_area_get_renderer (GTHREE_AREA (gl_area)),
                          scene2,
                          GTHREE_CAMERA (camera2));

  gthree_mesh_basic_material_set_map (material, gthree_render_target_get_texture (render_target));

  gthree_renderer_set_clear_color (gthree_area_get_renderer (GTHREE_AREA (gl_area)), &green);

  gthree_renderer_set_render_target (gthree_area_get_renderer (GTHREE_AREA (gl_area)),
                                     NULL, 0, 0);

  gthree_renderer_render (gthree_area_get_renderer (GTHREE_AREA (gl_area)),
                          scene,
                          GTHREE_CAMERA (camera));
  return TRUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Render targets");
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
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect (area, "render", G_CALLBACK (render_area), NULL);
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
