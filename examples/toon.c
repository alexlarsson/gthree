#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

GthreeScene *scene;
GthreeMesh *mesh;
GthreePerspectiveCamera *camera;

void
init_scene (void)
{
  graphene_vec3_t color;
  g_autoptr(GthreeGeometry) geometry = NULL;
  g_autoptr(GthreeTexture) texture = NULL;
  g_autoptr(GthreeMeshToonMaterial) material = NULL;
  g_autoptr(GthreeDirectionalLight) light = NULL;
  g_autoptr(GthreeAmbientLight) ambient = NULL;

  geometry = gthree_geometry_new_torus_knot (90, 25, 200, 20, 2, 3);

  scene = gthree_scene_new ();
  gthree_scene_set_background_color (scene,
                                     graphene_vec3_init (&color, 1.0, 1.0, 0.8));

  camera = gthree_perspective_camera_new (45, 1, 1, 1000);
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 0, 400);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  texture = examples_load_texture ("gradient/threeTone.jpg");
  gthree_texture_set_mag_filter (texture, GTHREE_FILTER_NEAREST);
  gthree_texture_set_min_filter (texture, GTHREE_FILTER_NEAREST);

  material = gthree_mesh_toon_material_new ();
  gthree_mesh_toon_material_set_gradient_map (material, texture);
  gthree_mesh_toon_material_set_color (material, graphene_vec3_init (&color, 0.0, 0.2, 0.8));

  mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

  light = gthree_directional_light_new (graphene_vec3_init (&color, 0.8, 0.8, 1.0), 1);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (light));

  ambient = gthree_ambient_light_new (white ());
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), 0.3);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient));
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
  gboolean done = FALSE;
  g_autoptr(GthreeOrbitControls) orbit = NULL;

  window = examples_init ("Toon material", &box, &done);

  init_scene ();

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  orbit = gthree_orbit_controls_new (GTHREE_OBJECT (camera), area);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
