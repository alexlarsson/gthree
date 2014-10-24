#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>

GthreeScene *scene;

GdkRGBA red =    {1, 0, 0, 1};
GdkRGBA green =  {0, 1, 0, 1};
GdkRGBA blue =   {0, 0, 1, 1};
GdkRGBA yellow = {1, 1, 0, 1};
GdkRGBA cyan   = {0, 1, 1, 1};
GdkRGBA magenta= {1, 0, 1, 1};
GdkRGBA white =  {1, 1, 1, 1};

GthreeMesh *marine, *knight;

GthreeGeometry *
load_model (const char *name)
{
  GthreeLoader *loader;
  GthreeGeometry *geometry;
  char *file;
  char *json;
  GError *error;

  error = NULL;
  file = g_build_filename ("models/", name, NULL);
  if (!g_file_get_contents (file, &json, NULL, &error))
    {
      error = NULL;
      g_free (file);
      file = g_build_filename ("examples/models/", name, NULL);
      if (!g_file_get_contents (file, &json, NULL, &error))
        g_error ("can't load model %s: %s", name, error->message);
      g_free (file);
    }

  loader = gthree_loader_new_from_json (json, NULL, &error);
  if (loader == NULL)
    g_error ("can't parse json: %s", error->message);

  g_free (json);

  geometry = g_object_ref (gthree_loader_get_geometry (loader));
  g_object_unref (loader);
  return geometry;
}



GthreeScene *
init_scene (void)
{
  GthreeGeometry *marine_geometry, *knight_geometry;
  GthreeBasicMaterial *material_wireframe, *material_texture;
  GthreePhongMaterial *material_phong;
  GthreeTexture *texture;
  GdkPixbuf *pixbuf;
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;
  graphene_point3d_t pos;
  GdkRGBA gray = {0.5, 0.5, 0.5, 1.0};
  GdkRGBA dark_grey = {0.1, 0.1, 0.1, 1.0};
  graphene_point3d_t scale = {15,15,15};

  pixbuf = gdk_pixbuf_new_from_file ("textures/MarineCv2_color.jpg", NULL);
  if (pixbuf == NULL)
    pixbuf = gdk_pixbuf_new_from_file ("examples/textures/MarineCv2_color.jpg", NULL);

  if (pixbuf == NULL)
    g_error ("could not load crate.gif");

  texture = gthree_texture_new (pixbuf);

  material_wireframe = gthree_basic_material_new ();
  gthree_material_set_is_wireframe (GTHREE_MATERIAL (material_wireframe), TRUE);
  gthree_basic_material_set_color (material_wireframe, &yellow);
  gthree_basic_material_set_vertex_colors (material_wireframe, GTHREE_COLOR_NONE);

  material_phong = gthree_phong_material_new ();
  gthree_phong_material_set_ambient_color (material_phong, &red);
  gthree_phong_material_set_emissive_color (material_phong, &gray);
  gthree_phong_material_set_specular_color (material_phong, &white);

  material_texture = gthree_basic_material_new ();
  gthree_basic_material_set_vertex_colors (material_texture, GTHREE_COLOR_NONE);
  gthree_basic_material_set_map (material_texture, texture);

  scene = gthree_scene_new ();

  marine_geometry = load_model ("marine.js");

  marine = gthree_mesh_new (marine_geometry, GTHREE_MATERIAL (material_texture));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (marine));

  knight_geometry = load_model ("knight.js");

  knight = gthree_mesh_new (knight_geometry, GTHREE_MATERIAL (material_phong));

  gthree_object_set_scale (GTHREE_OBJECT (knight), &scale);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (knight));

  ambient_light = gthree_ambient_light_new (&dark_grey);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));
  
  directional_light = gthree_directional_light_new (&white, 0.125);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
			      graphene_point3d_init (&pos,
						     0,
						     1,
						     1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
  
  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  static graphene_point3d_t rot2 = { 0, 0, 0};

  rot.y += 0.04;
  rot.z += 0.01;

  rot2.x += 0.01;
  rot2.z -= 0.03;

  gthree_object_set_rotation (GTHREE_OBJECT (marine), &rot);
  gthree_object_set_rotation (GTHREE_OBJECT (knight), &rot2);

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  graphene_point3d_t pos;

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

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 600));

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
