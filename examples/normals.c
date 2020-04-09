#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GList *objects;

static GthreeObject *
green_object (int num)
{
  GthreeGeometry *geo;
  GthreeMaterial *material;

  switch (num)
    {
    case 0:
      geo = gthree_geometry_new_sphere (20, 40, 10);
      break;
    case 1:
      geo = gthree_geometry_new_torus_full (25, 12, 20, 30, 2 * G_PI);
      break;
    case 2:
      geo = gthree_geometry_new_box (40, 20, 30, 5, 5, 5);
      break;
    case 3:
      geo = gthree_geometry_new_cylinder_full (15, 30, 50, 15, 20, FALSE, 0, 2 * G_PI);
      break;
    default:
      g_assert_not_reached ();
    }

  material = GTHREE_MATERIAL (gthree_mesh_lambert_material_new ());
  gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), green ());
  gthree_mesh_lambert_material_set_emissive_color (GTHREE_MESH_LAMBERT_MATERIAL (material), green ());
  gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);

  return GTHREE_OBJECT (gthree_mesh_new (geo, material));
}

GthreeObject *
face_normals (GthreeMesh *object, float size, const graphene_vec3_t *color, float width)
{
  GthreeGeometry *geo, *geometry;
  GthreeMaterial *material;
  GthreeAttribute *position, *normal;
  GthreeAttribute *a_position;
  int vertex_index = 0;
  int n_vertices;
  int i;

  geometry = gthree_mesh_get_geometry (object);

  position = gthree_geometry_get_position (geometry);
  normal = gthree_geometry_get_normal (geometry);

  n_vertices = gthree_attribute_get_count (position);

  geo = gthree_geometry_new ();

  material = GTHREE_MATERIAL (gthree_line_basic_material_new ());
  gthree_line_basic_material_set_color (GTHREE_LINE_BASIC_MATERIAL (material), color);
  gthree_line_basic_material_set_line_width (GTHREE_LINE_BASIC_MATERIAL (material), width);

  a_position = gthree_attribute_new ("position", GTHREE_ATTRIBUTE_TYPE_FLOAT, n_vertices * 2, 3, FALSE);
  gthree_geometry_add_attribute (geo, "position", a_position);

  for (i = 0; i < n_vertices; i++)
    {
      graphene_point3d_t *p, *n;
      graphene_vec3_t pv, nv, p2v;

      p = gthree_attribute_peek_point3d_at (position, i);
      n = gthree_attribute_peek_point3d_at (normal, i);

      graphene_point3d_to_vec3 (p, &pv);
      graphene_point3d_to_vec3 (n, &nv);
      graphene_vec3_scale (&nv, size, &nv);
      graphene_vec3_add (&pv, &nv, &p2v);

      gthree_attribute_set_vec3 (a_position, vertex_index++, &pv);
      gthree_attribute_set_vec3 (a_position, vertex_index++, &p2v);
    }

  g_object_unref (geometry);

  return GTHREE_OBJECT (gthree_line_segments_new (geo, material));
}

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  GthreeAmbientLight *ambient_light;
  graphene_point3d_t pos;
  int i;

  scene = gthree_scene_new ();

  ambient_light = gthree_ambient_light_new (white ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  for (i = 0; i < 4; i++)
    {
      GthreeObject * obj = green_object (i);

      objects = g_list_prepend (objects, obj);

      gthree_object_add_child (GTHREE_OBJECT (scene), obj);
      gthree_object_set_position_point3d (obj, graphene_point3d_init (&pos, i * 70 - 100, 0, 0));
      gthree_object_add_child (obj, face_normals (GTHREE_MESH (obj), 10, red (), 1));
    }

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  graphene_euler_t euler;
  GList *l;

  rot.x += 1.8;
  rot.y += 1.2;
  rot.z += 0.6;

  for (l = objects; l; l = l->next)
    {
      GthreeObject *obj = l->data;

      gthree_object_set_rotation (obj, graphene_euler_init (&euler, rot.x, rot.y, rot.z));
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
  graphene_point3d_t pos;
  gboolean done = FALSE;

  window = examples_init ("Normals", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (box), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
