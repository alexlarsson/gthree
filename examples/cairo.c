#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMesh *cube;
GthreeTexture *texture;
cairo_surface_t *surface;

GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeMeshBasicMaterial *material;

  surface =  cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 256, 256);

  cairo_t *cr = cairo_create (surface);
  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_paint (cr);
  cairo_destroy (cr);

  texture = gthree_texture_new_from_surface (surface);
  gthree_texture_set_flip_y (texture, FALSE); // We'll just draw upside down to avoid performance penalty

  material = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_map (material, texture);

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (130, 130, 130, 1, 1, 1);
  cube = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube));

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

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  gthree_object_set_rotation_xyz (GTHREE_OBJECT (cube),
                                  0.0 * relative_time,
                                  2.0 * relative_time,
                                  1.0 * relative_time);

  cairo_t *cr = cairo_create (surface);
  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_paint (cr);

  cairo_move_to (cr, 128.0, 25.6);
  cairo_line_to (cr, 210.4 + sin (relative_time / 10) * 80, 230.4);
  cairo_rel_line_to (cr, -102.4, 0.0);
  cairo_curve_to (cr, 51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
  cairo_close_path (cr);

  cairo_move_to (cr, 64.0, 25.6);
  cairo_rel_line_to (cr, 51.2, 51.2);
  cairo_rel_line_to (cr, -51.2, 51.2);
  cairo_rel_line_to (cr, -51.2, -51.2 + sin (relative_time / 10) * 80);
  cairo_close_path (cr);

  cairo_set_line_width (cr, 10.0);
  cairo_set_source_rgb (cr, 0, 0, 1);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_stroke (cr);

  cairo_destroy (cr);

  gthree_texture_set_needs_update (texture);

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

  window = examples_init ("Cairo textures", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 0, 400);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
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
