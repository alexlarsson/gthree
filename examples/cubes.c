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
GPtrArray *multi_materials;
GthreeMesh *mesh;
double rot = 0;

GList *cubes;

static void
colorise_faces (GthreeGeometry *geometry)
{
  int count = gthree_geometry_get_position_count (geometry);
  GthreeAttribute *color = gthree_geometry_add_attribute (geometry, "color",
                                                          gthree_attribute_new ("color", GTHREE_ATTRIBUTE_TYPE_FLOAT, count,
                                                                                3, FALSE));
  int i, j;

  for (i = 0; i < count / 4; i++)
    {
      const graphene_vec3_t *rgb = NULL;
      switch (i)
        {
        case 0:
          rgb = red ();
          break;
        case 1:
          rgb = green ();
          break;
        case 2:
          rgb = blue ();
          break;
        case 3:
          rgb = cyan ();
          break;
        case 4:
          rgb = magenta ();
          break;
        case 5:
          rgb = yellow ();
          break;
        }

      for (j = 0; j < 4; j++)
        gthree_attribute_set_vec3 (color, i * 4 + j, rgb);
    }

  g_object_unref (color);
}

static void
colorise_vertices (GthreeGeometry *geometry)
{
  int count = gthree_geometry_get_position_count (geometry);
  GthreeAttribute *color = gthree_geometry_add_attribute (geometry, "color",
                                                          gthree_attribute_new ("color", GTHREE_ATTRIBUTE_TYPE_FLOAT, count,
                                                                                3, FALSE));
  int i;

  for (i = 0; i < count / 4; i++)
    {
      gthree_attribute_set_vec3 (color, i * 4 + 0, red ());
      gthree_attribute_set_vec3 (color, i * 4 + 1, blue ());
      gthree_attribute_set_vec3 (color, i * 4 + 2, green ());
      gthree_attribute_set_vec3 (color, i * 4 + 3, yellow ());
    }

  g_object_unref (color);
}

static GthreeObject *
new_cube (GthreeMaterial *material, GPtrArray *materials, gboolean vertex_colors)
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
  if (materials)
    gthree_mesh_set_materials (mesh, materials);

  sub_mesh = gthree_mesh_new (sub_geometry, material);
  if (materials)
    gthree_mesh_set_materials (sub_mesh, materials);

  pos.y = 25;
  gthree_object_add_child (GTHREE_OBJECT (mesh), GTHREE_OBJECT (sub_mesh));
  gthree_object_set_position_point3d (GTHREE_OBJECT (sub_mesh), &pos);

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
  gthree_mesh_basic_material_set_color (material_simple, cyan ());
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_simple), FALSE);

  material_vertex_color = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_vertex_color), TRUE);

  material_wireframe = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material_wireframe), TRUE);
  gthree_mesh_basic_material_set_color (material_wireframe, yellow ());
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_wireframe), FALSE);

  material_texture = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (material_texture), FALSE);
  gthree_mesh_basic_material_set_map (material_texture, texture);

  scene = gthree_scene_new ();

  multi_materials = g_ptr_array_new_with_free_func (g_object_unref);
  for (i = 0; i < 3; i++)
    g_ptr_array_add (multi_materials, g_object_ref (material_simple));
  for (i = 3; i < 6; i++)
    g_ptr_array_add (multi_materials, g_object_ref (material_simple));

  pos.x = -60;
  pos.y = 40;

  cube = new_cube (GTHREE_MATERIAL (material_simple), NULL, FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (material_vertex_color), NULL, FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (material_texture), NULL, FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.y -= 80;
  pos.x -= 2*60;

  cube = new_cube (GTHREE_MATERIAL (material_wireframe), NULL, FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (GTHREE_MATERIAL (material_vertex_color), NULL, TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 60;
  cube = new_cube (NULL, multi_materials, FALSE);
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position_point3d (GTHREE_OBJECT (cube), &pos);
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

      gthree_object_set_rotation_xyz (cube,
                                      0.0 * relative_time,
                                      2.0 * relative_time,
                                      1.0 * relative_time);

      cube = gthree_object_get_first_child (cube);
      gthree_object_set_rotation_xyz (cube,
                                      0.0 * relative_time,
                                      -4.0 * relative_time,
                                      0.0 * relative_time);
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
  GtkWidget *window, *box, *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  gboolean done = FALSE;

  window = examples_init ("Cubes", &box, &done);

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
