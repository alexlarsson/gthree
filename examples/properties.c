#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "prop-editor.h"

#define N_OBJECT_TYPES 4
#define N_MATERIALS 3

GthreeObject *objects[N_OBJECT_TYPES * N_MATERIALS];
GthreeMaterial *materials[N_MATERIALS];

GthreeAmbientLight *ambient_light;
GthreeDirectionalLight *directional_light;
GthreePointLight *point_light;

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

static GthreeMaterial *
sample_material (int num)
{
  switch (num)
    {
    case 0:
      return GTHREE_MATERIAL (gthree_mesh_basic_material_new ());
      break;
    case 1:
      return GTHREE_MATERIAL (gthree_mesh_lambert_material_new ());
      break;
    case 2:
      return GTHREE_MATERIAL (gthree_mesh_phong_material_new ());
      break;
    default:
      g_assert_not_reached ();
    }
}

static GthreeObject *
sample_object (int num, GthreeMaterial *material)
{
  GthreeGeometry *geo;

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

  colorise_vertices (geo);

  return GTHREE_OBJECT (gthree_mesh_new (geo, material));
}

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  graphene_point3d_t pos;
  GthreeGeometry *geometry_light;
  GthreeMeshBasicMaterial *material_light;
  GthreeMesh *particle_light;

  int i, j;

  scene = gthree_scene_new ();

  ambient_light = gthree_ambient_light_new (&red);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (4, 8, 8);
  material_light = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_light, &green);

  point_light = gthree_point_light_new (&green, 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (&blue, 1.2);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     1, 1, -1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));


  for (i = 0; i < N_MATERIALS; i++)
    {
      materials[i] = sample_material (i);

      for (j = 0; j < N_OBJECT_TYPES; j++)
        {
          GthreeObject * obj = sample_object (j, materials[i]);

          objects[i * N_OBJECT_TYPES + j] = obj;

          gthree_object_add_child (GTHREE_OBJECT (scene), obj);
          gthree_object_set_position (obj, graphene_point3d_init (&pos,
                                                                  j * 70 - 100,
                                                                  - i * 70 + 80,
                                                                  -30));
        }
    }

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  gint64 frame_time;
  float angle;
  graphene_euler_t euler;
  graphene_point3d_t pos;
  int i;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 4000000.0;

  rot.x += 1.8;
  rot.y += 1.2;
  rot.z += 0.6;

  for (i = 0; i < G_N_ELEMENTS (objects); i++)
    {
      GthreeObject * obj = objects[i];

      gthree_object_set_rotation (obj, graphene_euler_init (&euler, rot.x, rot.y, rot.z));
    }

  gthree_object_set_position (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos,
                                                     sin (angle * 7) * 300,
                                                     cos (angle * 5) * 400,
                                                     cos (angle * 3) * 300));

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
  GtkWidget *window, *box, *hbox, *button, *area, *hbox2, *sw, *panel;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  graphene_point3d_t pos;
  int i;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_window_set_title (GTK_WINDOW (window), "GTK+");
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

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (box), sw);
  gtk_widget_show (sw);

  gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (sw), 300);

  hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_container_add (GTK_CONTAINER (sw), hbox2);
  gtk_widget_show (hbox2);

  for (i = 0; i < N_MATERIALS; i++)
    {
      panel = property_editor_widget_new (G_OBJECT (materials[i]),
                                          g_type_name_from_instance ((gpointer)materials[i]) + strlen ("Gthree"));
      gtk_container_add (GTK_CONTAINER (hbox2), panel);
    }

  panel = property_editor_widget_new (G_OBJECT (ambient_light),
                                      g_type_name_from_instance ((gpointer)ambient_light) + strlen ("Gthree"));
  gtk_container_add (GTK_CONTAINER (hbox2), panel);

  panel = property_editor_widget_new (G_OBJECT (point_light),
                                      g_type_name_from_instance ((gpointer)point_light) + strlen ("Gthree"));
  gtk_container_add (GTK_CONTAINER (hbox2), panel);

  panel = property_editor_widget_new (G_OBJECT (directional_light),
                                      g_type_name_from_instance ((gpointer)directional_light) + strlen ("Gthree"));
  gtk_container_add (GTK_CONTAINER (hbox2), panel);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
