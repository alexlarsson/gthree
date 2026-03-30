#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "prop-editor.h"

#define N_OBJECT_TYPES 4
#define N_MATERIALS 5

GthreeObject *objects[N_OBJECT_TYPES];
GthreeMaterial *materials[N_MATERIALS];
GthreeMaterial *current_material;
GtkWidget *property_pane;


GthreeAmbientLight *ambient_light;
GthreeDirectionalLight *directional_light;
GthreePointLight *point_light;

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

static GthreeMaterial *
sample_material (int num)
{
  switch (num)
    {
    case 0:
      return GTHREE_MATERIAL (gthree_mesh_standard_material_new ());
      break;
    case 1:
      return GTHREE_MATERIAL (gthree_mesh_phong_material_new ());
      break;
    case 2:
      return GTHREE_MATERIAL (gthree_mesh_lambert_material_new ());
      break;
    case 3:
      return GTHREE_MATERIAL (gthree_mesh_basic_material_new ());
      break;
    case 4:
      return GTHREE_MATERIAL (gthree_mesh_toon_material_new ());
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
      geo = gthree_geometry_new_sphere (60, 40, 10);
      break;
    case 1:
      geo = gthree_geometry_new_torus_full (40, 20, 20, 30, 2 * G_PI);
      break;
    case 2:
      geo = gthree_geometry_new_box (60, 40, 80, 5, 5, 5);
      break;
    case 3:
      geo = gthree_geometry_new_cylinder_full (24, 40, 60, 15, 20, FALSE, 0, 2 * G_PI);
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
  GthreeGeometry *geometry_light;
  GthreeMeshBasicMaterial *material_light;
  GthreeMesh *particle_light;

  int i, j;

  scene = gthree_scene_new ();

  ambient_light = gthree_ambient_light_new (red ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (4, 8, 8);
  material_light = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_light, green ());

  point_light = gthree_point_light_new (green (), 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (blue (), 1.2);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light),
                                  1, 1, -1);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));


  for (i = 0; i < N_MATERIALS; i++)
    materials[i] = sample_material (i);
  current_material = materials[0];

  for (j = 0; j < N_OBJECT_TYPES; j++)
    {
      GthreeObject * obj = sample_object (j, current_material);

      objects[j] = obj;

      gthree_object_add_child (GTHREE_OBJECT (scene), obj);
      gthree_object_set_position_xyz (obj,
                                      140 * (j - 1.5),
                                      0,
                                      0);
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
  static gint64 first_frame_time_i = 0;
  int i;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time_i == 0)
    first_frame_time_i = frame_time;

  frame_time -= first_frame_time_i;

  angle = frame_time / 4000000.0;

  rot.x += 1.8;
  rot.y += 1.2;
  rot.z += 0.6;

  for (i = 0; i < G_N_ELEMENTS (objects); i++)
    {
      GthreeObject * obj = objects[i];

      gthree_object_set_rotation_xyz (obj, rot.x, rot.y, rot.z);
    }

  gthree_object_set_position_xyz (GTHREE_OBJECT (point_light),
                                  sin (angle * 7) * 300,
                                  cos (angle * 5) * 400,
                                  cos (angle * 3) * 300);

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

static void
update_property_pane (void)
{
  GtkWidget *hbox, *panel;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);

#ifdef USE_GTK4
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(property_pane), hbox);
#else
  GList *children = gtk_container_get_children (GTK_CONTAINER (property_pane));
  GList *l;

  for (l = children; l != NULL; l = l->next)
    gtk_widget_destroy (l->data);
  gtk_container_add (GTK_CONTAINER (property_pane), hbox);
#endif

  panel = property_editor_widget_new (G_OBJECT (current_material),
                                      g_type_name_from_instance ((gpointer)current_material) + strlen ("Gthree"));
  gtk_box_append (GTK_BOX (hbox), panel);

  panel = property_editor_widget_new (G_OBJECT (ambient_light),
                                      g_type_name_from_instance ((gpointer)ambient_light) + strlen ("Gthree"));
  gtk_box_append (GTK_BOX (hbox), panel);

  panel = property_editor_widget_new (G_OBJECT (point_light),
                                      g_type_name_from_instance ((gpointer)point_light) + strlen ("Gthree"));
  gtk_box_append (GTK_BOX (hbox), panel);

  panel = property_editor_widget_new (G_OBJECT (directional_light),
                                      g_type_name_from_instance ((gpointer)directional_light) + strlen ("Gthree"));
  gtk_box_append (GTK_BOX (hbox), panel);

#ifdef USE_GTK3
  gtk_widget_show_all (property_pane);
#endif
}

static void
material_combo_changed (GtkComboBox *combo_box)
{
  gint i, mat;

  mat = gtk_combo_box_get_active (combo_box);
  current_material = materials[mat];

  for (i = 0; i < N_OBJECT_TYPES; i++)
    {
      GthreeObject * obj = objects[i];

      gthree_mesh_set_material (GTHREE_MESH (obj), 0, current_material);
    }
  update_property_pane ();
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area, *sw, *combo;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  gboolean done = FALSE;
  int i;

  window = examples_init ("Properties", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  0, 0, 400);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  combo = gtk_combo_box_text_new ();
  for (i = 0; i < N_MATERIALS; i++)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), g_type_name_from_instance ((gpointer)materials[i]));

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  g_signal_connect (combo, "changed", (GCallback)material_combo_changed, NULL);

  gtk_widget_show (combo);
  gtk_box_append (GTK_BOX (box), combo);

#ifdef USE_GTK4
  sw = gtk_scrolled_window_new ();
#else
  sw = gtk_scrolled_window_new (NULL, NULL);
#endif
  gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (sw), 300);
  gtk_box_append (GTK_BOX (box), sw);
  gtk_widget_show (sw);

  property_pane = sw;

  gtk_widget_show (window);

  update_property_pane ();

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
