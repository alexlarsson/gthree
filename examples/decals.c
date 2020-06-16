#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

static GthreeObject *head;

static GthreeObject *
load_lee_perry_smith (void)
{
  g_autoptr(GthreeLoader) loader = examples_load_gltl ("LeePerrySmith/LeePerrySmith.glb");
  GthreeScene *scene = gthree_loader_get_scene (loader, 0);
  GthreeObject *mesh_group = gthree_object_get_first_child (GTHREE_OBJECT (scene));
  GthreeObject *mesh = gthree_object_get_first_child (GTHREE_OBJECT (mesh_group));
  g_autoptr(GthreeMeshPhongMaterial) material = NULL;
  g_autoptr(GthreeTexture) map = examples_load_texture ("../models/LeePerrySmith/Map-COL.jpg");
  g_autoptr(GthreeTexture) specular_map = examples_load_texture ("../models/LeePerrySmith/Map-SPEC.jpg");
  g_autoptr(GthreeTexture) normal_map = examples_load_texture ("../models/LeePerrySmith/Infinite-Level_02_Tangent_SmoothUV.jpg");

  g_object_ref (mesh);
  gthree_object_remove_child (GTHREE_OBJECT (mesh_group), mesh);

  gthree_object_set_scale_uniform (mesh, 10);

  material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_specular_color (material, dark_grey ());
  gthree_mesh_phong_material_set_map (material, map);
  gthree_mesh_phong_material_set_specular_map (material, specular_map);
  gthree_mesh_phong_material_set_normal_map (material, normal_map);
  gthree_mesh_phong_material_set_shininess (material, 25);

  gthree_mesh_set_material (GTHREE_MESH (mesh), 0, GTHREE_MATERIAL (material));

  return mesh;
}

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  g_autoptr(GthreeAmbientLight) ambient_light = NULL;
  g_autoptr(GthreeDirectionalLight) directional_light = NULL;
  g_autoptr(GthreeDirectionalLight) directional_light2 = NULL;
  g_autoptr(GthreeAttribute) pos = NULL;
  g_autoptr(GthreeGeometry) geometry = NULL;
  g_autoptr(GthreeLineBasicMaterial) material = NULL;
  g_autoptr(GthreeLine) line = NULL;
  graphene_vec3_t color;

  scene = gthree_scene_new ();

  ambient_light = gthree_ambient_light_new (graphene_vec3_init (&color, 0.27, 0.2, 0.2));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  directional_light = gthree_directional_light_new (graphene_vec3_init (&color, 1.0, 0.87, 0.8), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light), 1, 0.75, 0.5);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  directional_light2 = gthree_directional_light_new (graphene_vec3_init (&color, 0.8, 0.8, 1.0), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light2), -1, 0.75, -0.5);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light2));

  pos = gthree_attribute_new ("position", GTHREE_ATTRIBUTE_TYPE_FLOAT, 2, 3, FALSE);
  gthree_attribute_set_xyz (pos, 0,
                            0, 0, 0);
  gthree_attribute_set_xyz (pos, 0,
                            40, 40, 40);

  geometry = gthree_geometry_new ();
  gthree_geometry_add_attribute (geometry, "position", pos);

  material = gthree_line_basic_material_new ();

  line = gthree_line_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (line));

  head = load_lee_perry_smith ();
  gthree_object_add_child (GTHREE_OBJECT (scene), head);
  gthree_object_set_position_xyz (GTHREE_OBJECT (head), 0, 0, 0);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  // TODO: Do we need this?

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
  g_autoptr(GthreeOrbitControls) orbit = NULL;

  window = examples_init ("Decal splatter", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (45, 1, 1, 1000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 0, 120);
  orbit = gthree_orbit_controls_new (GTHREE_OBJECT (camera), area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
