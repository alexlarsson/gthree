#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

GthreeGroup *group = NULL;

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  g_autoptr(GthreeHemisphereLight) light = NULL;
  graphene_plane_t clip_planes[3];
  graphene_vec3_t v;
  int i;

  scene = gthree_scene_new ();

  light = gthree_hemisphere_light_new (white (), dark_grey (), 1.5);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light),
                                  -1.5, 1, 1.25);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (light));

  group = gthree_group_new ();
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (group));

  graphene_plane_init (&clip_planes[0], graphene_vec3_init (&v,  1,  0,  0), 0);
  graphene_plane_init (&clip_planes[1], graphene_vec3_init (&v,  0, -1,  0), 0);
  graphene_plane_init (&clip_planes[2], graphene_vec3_init (&v,  0,  0, -1), 0);

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

  // TODO: Plane helpers
  // TODO: Change planes

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

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area, *check;
  GthreePerspectiveCamera *camera;
  GthreeScene *scene;
  gboolean done = FALSE;
  g_autoptr(OrbitControls) orbit = NULL;

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
  gtk_container_add (GTK_CONTAINER (box), area);
  gtk_widget_show (area);

  orbit = orbit_controls_new (GTHREE_OBJECT (camera), area);
  orbit_controls_set_enable_pan (orbit, FALSE);

  check = gtk_check_button_new_with_label ("Clip intersection");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
  gtk_container_add (GTK_CONTAINER (box), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (clip_intersection_toggled), area);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
