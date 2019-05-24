#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreePerspectiveCamera *camera;

GList *objects;
float pointer_x, pointer_y;

GthreeScene *
init_scene (void)
{
  GthreeNormalMaterial *material;
  GthreeGeometry *geometry;
  GthreeMesh *mesh;
  graphene_point3d_t pos, scale;
  graphene_euler_t rot;
  int i;

  geometry = examples_load_geometry ("Suzanne.js");

  gthree_geometry_compute_vertex_normals (geometry);

  material = gthree_normal_material_new ();
  gthree_normal_material_set_shading_type (material, GTHREE_SHADING_SMOOTH);

  scene = gthree_scene_new ();

  pos.x = 0;
  pos.y = 0;
  pos.z = 0;

  for (i = 0; i < 5000; i++)
    {
      mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));
      pos.x = g_random_double_range (-4000, 4000);
      pos.y = g_random_double_range (-4000, 4000);
      pos.z = g_random_double_range (-4000, 4000);
      gthree_object_set_position (GTHREE_OBJECT (mesh), &pos);
      scale.x = scale.y = scale.z = g_random_double_range (0, 50) + 100;
      gthree_object_set_scale (GTHREE_OBJECT (mesh), &scale);
      graphene_euler_init (&rot,
                           g_random_double_range (0, 360.0),
                           g_random_double_range (0, 360.0),
                           0);
      gthree_object_set_rotation (GTHREE_OBJECT (mesh), &rot);
      objects = g_list_prepend (objects, mesh);
    }

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_euler_t rot;
  const graphene_euler_t *old_rot;
  graphene_point3d_t pos;
  GList *l;

  gthree_object_get_position (GTHREE_OBJECT (camera), &pos);
  pos.x += (pointer_x * 8000 - pos.x) * 0.5;
  pos.y += (pointer_y * 8000 - pos.y) * 0.5;
  gthree_object_set_position (GTHREE_OBJECT (camera), &pos);
  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&pos, 0, 0, 0));

  for (l = objects; l != NULL; l = l->next)
    {
      GthreeObject *obj = l->data;

      old_rot = gthree_object_get_rotation (obj);
      graphene_euler_init (&rot,
                           graphene_euler_get_x (old_rot) + 0.5,
                           graphene_euler_get_y (old_rot) + 1.0,
                           0);
      gthree_object_set_rotation (obj, &rot);
    }

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
motion_event (GtkWidget      *widget,
              GdkEventMotion *event)
{
  pointer_x = (event->x - gtk_widget_get_allocated_width (widget) / 2) / (double)(gtk_widget_get_allocated_width (widget) / 2);
  pointer_y = (event->y - gtk_widget_get_allocated_height (widget) / 2) / (double)(gtk_widget_get_allocated_height (widget) / 2);
  return FALSE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;
  graphene_point3d_t pos;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Cubes");
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
  camera = gthree_perspective_camera_new (60, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 3200));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_add_events (GTK_WIDGET (area), GDK_POINTER_MOTION_MASK);
  g_signal_connect (area, "motion-notify-event", G_CALLBACK (motion_event), NULL);
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
