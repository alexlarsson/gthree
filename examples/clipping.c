#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

static GthreeGroup *group = NULL;
static GthreeGroup *helpers = NULL;
static graphene_plane_t clip_planes[3];

static void
update_planes (float value)
{
  graphene_vec3_t v;
  graphene_plane_init (&clip_planes[0], graphene_vec3_init (&v,  1,  0,  0), value);
  graphene_plane_init (&clip_planes[1], graphene_vec3_init (&v,  0, -1,  0), value);
  graphene_plane_init (&clip_planes[2], graphene_vec3_init (&v,  0,  0, -1), value);
}

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  g_autoptr(GthreeHemisphereLight) light = NULL;
  const graphene_vec3_t *colors[] = { red (), green (), blue () };
  int i;

  scene = gthree_scene_new ();

  light = gthree_hemisphere_light_new (white (), dark_grey (), 1.5);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light),
                                  -1.5, 1, 1.25);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (light));

  group = gthree_group_new ();
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (group));

  update_planes (0);

  for (i = 0; i <= 30; i+=2)
    {
      g_autoptr(GthreeGeometry) geometry = NULL;
      g_autoptr(GthreeMeshLambertMaterial) material = NULL;
      g_autoptr(GthreeMesh) mesh = NULL;
      graphene_vec3_t color;

      geometry = gthree_geometry_new_sphere (i / 30.0, 48, 24);

      material = gthree_mesh_lambert_material_new ();

      rgb_init_from_hsl (&color,
                         g_random_double_range (0, 360),
                         0.5, 0.5);

      gthree_mesh_lambert_material_set_color (material, &color);
      gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);

      gthree_material_set_clipping_plane (GTHREE_MATERIAL (material), 0, &clip_planes[0]);
      gthree_material_set_clipping_plane (GTHREE_MATERIAL (material), 1, &clip_planes[1]);
      gthree_material_set_clipping_plane (GTHREE_MATERIAL (material), 2, &clip_planes[2]);
      gthree_material_set_clip_intersection (GTHREE_MATERIAL (material), TRUE);
      mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));

      gthree_object_add_child (GTHREE_OBJECT (group), GTHREE_OBJECT (mesh));
    }

  helpers = gthree_group_new ();
  for (i = 0; i < 3; i++)
    {
      g_autoptr(GthreePlaneHelper) helper = gthree_plane_helper_new (&clip_planes[i], 2, colors[i]);
      gthree_object_add_child (GTHREE_OBJECT (helpers), GTHREE_OBJECT (helper));
    }
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (helpers));

  return scene;
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
realize_area (GthreeArea *area)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (area);

  /* Needed for per-material clipping */
  gthree_renderer_set_local_clipping_enabled (renderer, TRUE);
}

static void
clip_intersection_toggled (GtkToggleButton *toggle_button,
                           GtkWidget *area)
{
  gboolean clip_intersection = gtk_toggle_button_get_active (toggle_button);
  GthreeObject *child;

  for (child = gthree_object_get_first_child (GTHREE_OBJECT (group));
       child != NULL;
       child = gthree_object_get_next_sibling (child))
    {
      GthreeMaterial *material = gthree_mesh_get_material (GTHREE_MESH (child), 0);
      gthree_material_set_clip_intersection (material, clip_intersection);
    }

  gtk_widget_queue_draw (area);
}

static void
show_helpers_toggled (GtkToggleButton *toggle_button,
                      GtkWidget *area)
{
  gboolean visible = gtk_toggle_button_get_active (toggle_button);

  gthree_object_set_visible (GTHREE_OBJECT (helpers), visible);

  gtk_widget_queue_draw (area);
}

static void
scale_value_changed  (GtkRange *range,
                      GtkWidget *area)
{
  gdouble value = gtk_range_get_value (range);
  GthreeObject *child;
  int i;

  update_planes (value);

  for (child = gthree_object_get_first_child (GTHREE_OBJECT (group));
       child != NULL;
       child = gthree_object_get_next_sibling (child))
    {
      GthreeMaterial *material = gthree_mesh_get_material (GTHREE_MESH (child), 0);

      gthree_material_set_clipping_plane (material, 0, &clip_planes[0]);
      gthree_material_set_clipping_plane (material, 1, &clip_planes[1]);
      gthree_material_set_clipping_plane (material, 2, &clip_planes[2]);
    }

  for (i = 0, child = gthree_object_get_first_child (GTHREE_OBJECT (helpers));
       child != NULL;
       i++, child = gthree_object_get_next_sibling (child))
    {
      gthree_plane_helper_set_plane (GTHREE_PLANE_HELPER (child), &clip_planes[i]);
    }

  gtk_widget_queue_draw (area);
}


int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area, *check, *hbox, *scale;
  GthreePerspectiveCamera *camera;
  GthreeScene *scene;
  gboolean done = FALSE;
  g_autoptr(GthreeOrbitControls) orbit = NULL;

  window = examples_init ("Clipping intersection", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (40, 1, 1, 200);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), -1.5, 2.5, 3.0);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera), 0, 0, 0);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect_after (area, "realize", G_CALLBACK (realize_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  orbit = gthree_orbit_controls_new (GTHREE_OBJECT (camera), area);
  gthree_orbit_controls_set_enable_pan (orbit, FALSE);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_widget_show (hbox);
  gtk_box_append (GTK_BOX (box), hbox);

  check = gtk_check_button_new_with_label ("Clip intersection");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
  gtk_box_append (GTK_BOX (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (clip_intersection_toggled), area);

  check = gtk_check_button_new_with_label ("Show helpers");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
  gtk_box_append (GTK_BOX (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (show_helpers_toggled), area);

  scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -1, 1, 0.01);
  gtk_range_set_value (GTK_RANGE (scale), 0);
  gtk_widget_set_hexpand (scale, TRUE);
  gtk_box_append (GTK_BOX (hbox), scale);
  gtk_widget_show (scale);
  g_signal_connect (scale, "value-changed", G_CALLBACK (scale_value_changed), area);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
