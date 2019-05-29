#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMeshBasicMaterial *material_simple;
GthreeMeshBasicMaterial *material_texture;
GthreeMeshBasicMaterial *material_vertex_color;
GthreeMeshBasicMaterial *material_wireframe;
GthreeMultiMaterial *multi_material;
GthreeMesh *mesh;
double rot = 0;

GList *cubes;

static void
colorise_faces (GthreeGeometry *geometry)
{
  int count = gthree_geometry_get_position_count (geometry);
  GthreeAttribute *color = gthree_geometry_add_attribute (geometry,
                                                          gthree_attribute_new ("color", GTHREE_ATTRIBUTE_TYPE_FLOAT, count,
                                                                                3, FALSE));
  int i, j;

  for (i = 0; i < count / 4; i++)
    {
      GdkRGBA *rgba;
      switch (i)
        {
        case 0:
          rgba = &red;
          break;
        case 1:
          rgba = &green;
          break;
        case 2:
          rgba = &blue;
          break;
        case 3:
          rgba = &cyan;
          break;
        case 4:
          rgba = &magenta;
          break;
        case 5:
          rgba = &yellow;
          break;
        }

      for (j = 0; j < 4; j++)
        gthree_attribute_set_rgb (color, i * 4 + j, rgba);
    }

  g_object_unref (color);
}

static void
colorise_vertices (GthreeGeometry *geometry)
{
  int count = gthree_geometry_get_position_count (geometry);
  GthreeAttribute *color = gthree_geometry_add_attribute (geometry,
                                                          gthree_attribute_new ("color", GTHREE_ATTRIBUTE_TYPE_FLOAT, count,
                                                                                3, FALSE));
  int i;

  for (i = 0; i < count / 4; i++)
    {
      gthree_attribute_set_rgb (color, i * 4 + 0, &red);
      gthree_attribute_set_rgb (color, i * 4 + 1, &blue);
      gthree_attribute_set_rgb (color, i * 4 + 2, &green);
      gthree_attribute_set_rgb (color, i * 4 + 3, &yellow);
    }

  g_object_unref (color);
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

  crate_pixbuf = examples_load_pixbuf ("crate.gif");

  texture = gthree_texture_new (crate_pixbuf);

  material_simple = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_simple, &cyan);
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_simple), FALSE);

  material_vertex_color = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_vertex_color), TRUE);

  material_wireframe = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material_wireframe), TRUE);
  gthree_mesh_basic_material_set_color (material_wireframe, &yellow);
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_wireframe), FALSE);

  material_texture = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_texture), FALSE);
  gthree_mesh_basic_material_set_map (material_texture, texture);

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
  cube = new_cube (GTHREE_MATERIAL (material_vertex_color), FALSE);
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
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float relative_time;
  graphene_euler_t euler;
  GList *l;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  for (l = cubes; l != NULL; l = l->next)
    {
      GthreeObject *cube = l->data;

      gthree_object_set_rotation (cube,
                                  graphene_euler_init (&euler,
                                                       0.0 * relative_time,
                                                       2.0 * relative_time,
                                                       1.0 * relative_time
                                                       ));

      cube = gthree_object_get_first_child (cube);
      gthree_object_set_rotation (cube,
                                  graphene_euler_init (&euler,
                                                        0.0 * relative_time,
                                                       -4.0 * relative_time,
                                                        0.0 * relative_time
                                                       ));
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
