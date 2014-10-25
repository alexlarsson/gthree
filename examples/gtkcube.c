#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>

GthreeScene *scene;
GthreeBasicMaterial *material_simple;
GthreeBasicMaterial *material_texture;
GthreeBasicMaterial *material_face_color;
GthreeBasicMaterial *material_vertex_color;
GthreeBasicMaterial *material_wireframe;
GthreeMultiMaterial *multi_material;
GthreeMesh *mesh;
double rot = 0;

GdkRGBA red =    {1, 0, 0, 1};
GdkRGBA green =  {0, 1, 0, 1};
GdkRGBA blue =   {0, 0, 1, 1};
GdkRGBA yellow = {1, 1, 0, 1};
GdkRGBA cyan   = {0, 1, 1, 1};
GdkRGBA magenta= {1, 0, 1, 1};
GdkRGBA white =  {1, 1, 1, 1};

GList *cubes;

static void
colorise_faces (GthreeGeometry *geometry)
{
  int i;

  for (i = 0; i < gthree_geometry_get_n_faces (geometry); i++)
    {
      int c = (i / 2) % 6;
      switch (c)
        {
        case 0:
          gthree_geometry_face_set_color (geometry, i, &red);
          break;
        case 1:
          gthree_geometry_face_set_color (geometry, i, &green);
          break;
        case 2:
          gthree_geometry_face_set_color (geometry, i, &blue);
          break;
        case 3:
          gthree_geometry_face_set_color (geometry, i, &cyan);
          break;
        case 4:
          gthree_geometry_face_set_color (geometry, i, &magenta);
          break;
        case 5:
          gthree_geometry_face_set_color (geometry, i, &yellow);
          break;
        }
    }
}

static void
colorise_vertices (GthreeGeometry *geometry)
{
  int i;

  for (i = 0; i < gthree_geometry_get_n_faces (geometry); i++)
    {
      if (i % 2 == 0)
        gthree_geometry_face_set_vertex_colors (geometry, i,
						&red,
						&green,
						&blue);
      else
        gthree_geometry_face_set_vertex_colors (geometry, i,
						&green,
						&yellow,
						&blue);
    }
}

static GthreeObject *
new_cube (GthreeMaterial *material, gboolean vertex_colors)
{
  GthreeGeometry *geometry = NULL, *sub_geometry = NULL;
  static GthreeGeometry *geometry_face = NULL, *sub_geometry_face = NULL;
  static GthreeGeometry *geometry_vertex = NULL, *sub_geometry_vertex = NULL;
  GthreeMesh *mesh, *sub_mesh;
  graphene_point3d_t pos = { 0, 0, 0};

  if (!vertex_colors)
    {
      if (geometry_face == NULL)
        {
          geometry_face = gthree_geometry_new_box (40, 40, 40, 1, 1, 1);
          colorise_faces (geometry_face);

          sub_geometry_face = gthree_geometry_new_box (10, 10, 10, 1, 1, 1);
          colorise_faces (sub_geometry_face);
        }
      geometry = geometry_face;
      sub_geometry = sub_geometry_face;
    }
  else
    {
      if (geometry_vertex == NULL)
        {
          geometry_vertex = gthree_geometry_new_box (40, 40, 40, 1, 1, 1);
          colorise_vertices (geometry_vertex);

          sub_geometry_vertex = gthree_geometry_new_box (10, 10, 10, 1, 1, 1);
          colorise_vertices (sub_geometry_vertex);
        }
      geometry = geometry_vertex;
      sub_geometry = sub_geometry_vertex;
    }

  mesh = gthree_mesh_new (geometry, material);


  sub_mesh = gthree_mesh_new (sub_geometry, material);

  pos.y = 25;
  gthree_object_add_child (GTHREE_OBJECT (mesh), GTHREE_OBJECT (sub_mesh));
  gthree_object_set_position (GTHREE_OBJECT (sub_mesh), &pos);

  return GTHREE_OBJECT (mesh);
}

GthreeScene *
init_scene (void)
{
  int i;

  GdkPixbuf *crate_pixbuf;
  GthreeTexture *texture;
  GthreeObject *cube;
  graphene_point3d_t pos = { 0, 0, 0};

  crate_pixbuf = gdk_pixbuf_new_from_file ("textures/crate.gif", NULL);
  if (crate_pixbuf == NULL)
    crate_pixbuf = gdk_pixbuf_new_from_file ("examples/textures/crate.gif", NULL);

  if (crate_pixbuf == NULL)
    g_error ("could not load crate.gif");

  texture = gthree_texture_new (crate_pixbuf);

  material_simple = gthree_basic_material_new ();
  gthree_basic_material_set_color (material_simple, &cyan);
  gthree_basic_material_set_vertex_colors (material_simple, GTHREE_COLOR_NONE);

  material_face_color = gthree_basic_material_new ();
  gthree_basic_material_set_vertex_colors (material_face_color, GTHREE_COLOR_FACE);

  material_vertex_color = gthree_basic_material_new ();
  gthree_basic_material_set_vertex_colors (material_vertex_color, GTHREE_COLOR_VERTEX);

  material_wireframe = gthree_basic_material_new ();
  gthree_material_set_is_wireframe (GTHREE_MATERIAL (material_wireframe), TRUE);
  gthree_basic_material_set_color (material_wireframe, &yellow);
  gthree_basic_material_set_vertex_colors (material_wireframe, GTHREE_COLOR_NONE);

  material_texture = gthree_basic_material_new ();
  gthree_basic_material_set_vertex_colors (material_texture, GTHREE_COLOR_NONE);
  gthree_basic_material_set_map (material_texture, texture);

  scene = gthree_scene_new ();

  multi_material = gthree_multi_material_new ();
  for (i = 0; i < 3; i++)
    gthree_multi_material_set_index (multi_material, i, GTHREE_MATERIAL (material_simple));
  for (i = 3; i < 6; i++)
    gthree_multi_material_set_index (multi_material, i, GTHREE_MATERIAL (material_texture));

  pos.x = -60;
  pos.y = 40;

  cube = new_cube (GTHREE_MATERIAL (material_simple), FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (material_face_color), FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (material_texture), FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.y -= 80;
  pos.x -= 2*60;

  cube = new_cube (GTHREE_MATERIAL (material_wireframe), FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (material_vertex_color), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (multi_material), FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  static graphene_point3d_t rot2 = { 0, 0, 0};
  GList *l;

  rot.y += 0.04;
  rot.z += 0.01;

  rot2.y -= 0.07;

  for (l = cubes; l != NULL; l = l->next)
    {
      GthreeObject *cube = l->data;

      gthree_object_set_rotation (cube, &rot);

      cube = gthree_object_get_first_child (cube);
      gthree_object_set_rotation (cube, &rot2);
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

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
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
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
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
