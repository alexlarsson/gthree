#include <stdlib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include <epoxy/gl.h>

#include "gthreearea.h"
#include "gthreemesh.h"

typedef struct {
  int dummy;
} Model;

Model *
load_model_from_json (char *path, GError **error)
{
  JsonParser *parser;

  parser = json_parser_new ();

  if (!json_parser_load_from_file (parser, path, error))
    {
      g_object_unref (parser);
      return NULL;
    }

  return NULL;
}

GthreeScene *scene;
GthreeGeometry *geometry;
GthreeMaterial *material;
GthreeMesh *mesh;
double rot = 0;

GthreeScene *
init_scene (void)
{
  static graphene_point3d_t rot = { 0, 0, 0};

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (100, 100, 100, 1, 1, 1);
  material = gthree_material_new ();
  mesh = gthree_mesh_new (geometry, material);

  //rot.y = 10;
  gthree_object_set_rotation (GTHREE_OBJECT (mesh), &rot);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t pos = { 0, 0, 0};
  static graphene_point3d_t rot = { 0, 0, 0};

  if (0)
    return G_SOURCE_CONTINUE;

  if (1)
    {
      //pos.x += 1;
      rot.z += 0.1;

      gthree_object_set_rotation (GTHREE_OBJECT (mesh), &rot);
      //gthree_object_set_position (GTHREE_OBJECT (mesh), &pos);
    }

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}


int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *button, *area;
  GthreeScene *scene;
  GthreeCamera *camera;
  graphene_point3d_t pos;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "GtkGlobe");
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (box), 6);
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show (box);

  scene = init_scene ();
  camera = gthree_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));

  area = gthree_area_new (scene, camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (box), area);
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
