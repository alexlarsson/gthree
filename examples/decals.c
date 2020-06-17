#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

static GthreeObject *head;
static gboolean is_intersection;
static graphene_vec3_t intersection_point;
static graphene_vec3_t intersection_normal;
static GthreeAttribute *line_pos;

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

  line_pos = pos = gthree_attribute_new ("position", GTHREE_ATTRIBUTE_TYPE_FLOAT, 2, 3, FALSE);
  gthree_attribute_set_xyz (pos, 0, 0, 0, 0);
  gthree_attribute_set_xyz (pos, 1, 10, 10, 10);

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

static void
motion_cb (GtkEventControllerMotion *controller,
           gdouble                   x,
           gdouble                   y,
           gpointer                  user_data)
{
  GtkWidget *widget = GTK_WIDGET (user_data);
  GthreeCamera *camera = gthree_area_get_camera (GTHREE_AREA (widget));
  GthreeScene *scene = gthree_area_get_scene (GTHREE_AREA (widget));
  g_autoptr(GthreeRaycaster) raycaster = gthree_raycaster_new ();
  g_autoptr(GPtrArray) intersections = NULL;

  x = (x / gtk_widget_get_allocated_width (widget)) * 2 - 1;
  y = -(y / gtk_widget_get_allocated_height (widget)) * 2 + 1;

  gthree_raycaster_set_from_camera  (raycaster, camera, x, y);

  intersections = gthree_raycaster_intersect_object (raycaster, GTHREE_OBJECT (scene), TRUE, NULL);
  if (intersections->len > 0)
    {
      GthreeRayIntersection *intersection = g_ptr_array_index (intersections, 0);
      const graphene_matrix_t *world_matrix =  gthree_object_get_world_matrix (head);
      graphene_vec3_t pos2;

      is_intersection = TRUE;

      graphene_point3d_to_vec3 (&intersection->point, &intersection_point);

      graphene_triangle_get_normal (&intersection->face, &intersection_normal);

      graphene_matrix_transform_vec3 (world_matrix, &intersection_normal, &pos2);
      graphene_vec3_add (&pos2, &intersection_point, &pos2);

      gthree_attribute_set_vec3 (line_pos, 0, &intersection_point);
      gthree_attribute_set_vec3 (line_pos, 1, &pos2);
      gthree_attribute_set_needs_update (line_pos);
    }
  else
    {
      is_intersection = FALSE;
   }
}

static void
released_cb (GtkEventController *controller,
             gint                n_press,
             gdouble             x,
             gdouble             y,
             gpointer            user_data)
{
  if (is_intersection)
    g_print ("SPLAT!\n");
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  gboolean done = FALSE;
  g_autoptr(GthreeOrbitControls) orbit = NULL;
  GtkEventController *click;
  GtkEventController *motion;

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

  click = click_controller_for (GTK_WIDGET (area));
  g_signal_connect (click, "released", (GCallback)released_cb, NULL);

  gthree_orbit_controls_add_other_gesture (orbit, GTK_GESTURE (click));

  motion = motion_controller_for (GTK_WIDGET (area));
  g_signal_connect (motion, "motion", (GCallback)motion_cb, area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
