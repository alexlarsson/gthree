#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreeScene *scene;
static GthreeSkinnedMesh *cylinder;
static GPtrArray *bones = NULL;

#define SEGMENT_HEIGHT 5
#define N_SEGMENTS 4
#define SPLIT_PER_SEGMENT 2
#define TOTAL_HEIGHT (SEGMENT_HEIGHT * N_SEGMENTS)

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

GthreeScene *
init_scene (void)
{
  GthreeMeshStandardMaterial *material;
  GthreeGeometry *geometry;
  GthreeAttribute *position, *skin_indices, *skin_weights;
  int i, n_vert;
  GthreeBone *last_bone, *root_bone;
  GthreeSkeleton *skeleton;
  GthreeGeometry *dot_geometry;
  GthreeMeshBasicMaterial *dot_material;

  material = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (material, &cyan);
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material), TRUE);
  gthree_mesh_material_set_wireframe_line_width (GTHREE_MESH_MATERIAL (material), 3);
  gthree_mesh_material_set_skinning (GTHREE_MESH_MATERIAL (material), TRUE);

  scene = gthree_scene_new ();

  geometry = gthree_geometry_new_cylinder_full (5, 5,
                                                TOTAL_HEIGHT,
                                                10, SPLIT_PER_SEGMENT * N_SEGMENTS,
                                                TRUE,
                                                0, 2 * G_PI);

  position = gthree_geometry_get_position (geometry);

  n_vert = gthree_attribute_get_count (position);

  skin_indices = gthree_attribute_new ("skinIndex",
                                       GTHREE_ATTRIBUTE_TYPE_UINT16,
                                       n_vert, 4, FALSE);
  skin_weights = gthree_attribute_new ("skinWeight",
                                       GTHREE_ATTRIBUTE_TYPE_FLOAT,
                                       n_vert, 4, FALSE);

  gthree_geometry_add_attribute (geometry, "skinIndex", skin_indices);
  gthree_geometry_add_attribute (geometry, "skinWeight", skin_weights);

  for (i = 0; i < n_vert; i++)
    {
      graphene_point3d_t vertex;
      float y;
      int skin_index;
      float skin_weight;
      guint16 *idx;
      float *wt;

      gthree_attribute_get_point3d (position, i, &vertex);
      y = vertex.y + TOTAL_HEIGHT / 2;

      skin_index = floor (y / SEGMENT_HEIGHT);
      skin_weight = y  / (float)SEGMENT_HEIGHT - skin_index;

      idx = gthree_attribute_peek_uint16_at (skin_indices, i);
      idx[0] = skin_index;
      idx[1] = skin_index + 1;
      idx[2] = 0;
      idx[3] = 0;

      wt = gthree_attribute_peek_float_at (skin_weights, i);
      wt[0] = 1 - skin_weight;
      wt[1] = skin_weight;
      wt[2] = 0;
      wt[3] = 0;
    }

  cylinder = gthree_skinned_mesh_new (geometry, GTHREE_MATERIAL (material));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cylinder));

  dot_geometry = gthree_geometry_new_box (1.5, 1.5, 1.5, 1, 1, 1);
  colorise_faces (dot_geometry);
  dot_material = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (dot_material), TRUE);

  bones = g_ptr_array_new_with_free_func (g_object_unref);
  last_bone = NULL;
  for (i = 0; i < N_SEGMENTS+1; i++)
    {
      GthreeBone *bone = gthree_bone_new ();
      graphene_point3d_t p;
      GthreeMesh *dot;

      g_ptr_array_add (bones, bone);

      if (i == 0)
        gthree_object_set_position (GTHREE_OBJECT (bone),
                                    graphene_point3d_init (&p, 0, - (N_SEGMENTS) * SEGMENT_HEIGHT / 2, 0));
      else
        gthree_object_set_position (GTHREE_OBJECT (bone),
                                    graphene_point3d_init (&p, 0, SEGMENT_HEIGHT, 0));

      dot = gthree_mesh_new (dot_geometry, GTHREE_MATERIAL (dot_material));
      gthree_object_add_child (GTHREE_OBJECT (bone), GTHREE_OBJECT (dot));

      if (last_bone != NULL)
        gthree_object_add_child (GTHREE_OBJECT (last_bone),
                                 GTHREE_OBJECT (bone));
      else
        root_bone = bone;

      last_bone = bone;
    }

  skeleton = gthree_skeleton_new ((GthreeBone **)bones->pdata, bones->len, NULL);
  gthree_object_add_child (GTHREE_OBJECT (cylinder), GTHREE_OBJECT (root_bone));

  gthree_skinned_mesh_bind (cylinder, skeleton, NULL);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_euler_t rot;
  graphene_point3d_t pos;
  gint64 frame_time;
  float angle;
  int i;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 40000.0;

  for (i = 0; i < N_SEGMENTS+1; i++)
    {
      GthreeBone *bone = g_ptr_array_index (bones, i);

      graphene_euler_init (&rot,
                           0,  sin (angle / 40) * 15, 0);
      gthree_object_set_rotation (GTHREE_OBJECT (bone), &rot);

      graphene_point3d_init_from_vec3 (&pos,
                                       gthree_object_get_position (GTHREE_OBJECT (bone)));
      pos.x = sin (angle / 40) * 1;
      gthree_object_set_position (GTHREE_OBJECT (bone), &pos);
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
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Skinning");
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

  ambient_light = gthree_ambient_light_new (&white);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  directional_light = gthree_directional_light_new (&white, 0.125);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     1, 1, -1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 13, 50));
  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&pos, 0, 0, 0));

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
