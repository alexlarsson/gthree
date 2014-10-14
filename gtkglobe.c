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

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  GthreeGeometry *geometry;
  GthreeMaterial *material;
  GthreeMesh *mesh;

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_box (100, 100, 100, 1, 1, 1);
  material = gthree_material_new ();
  mesh = gthree_mesh_new (geometry, material);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

  return scene;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *button, *area;
  GthreeScene *scene;
  GthreeCamera *camera;

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

  area = gthree_area_new (scene, camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (box), area);
  gtk_widget_show (area);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
