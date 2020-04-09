#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreeObject *logo;

static GthreeObject *
new_cube (GPtrArray *materials)
{
  static GthreeGeometry *geometry = NULL;
  GthreeMesh *cube;

  if (geometry == NULL)
    geometry = gthree_geometry_new_box (60, 60, 60, 1, 1, 1);

  cube = gthree_mesh_new (geometry, NULL);
  gthree_mesh_set_materials (cube, materials);
  return GTHREE_OBJECT (cube);
}

static GthreeObject *
new_cylinder (GthreeMaterial *material)
{
  GthreeGeometry *geometry;

  geometry = gthree_geometry_new_cylinder (1, 60);

  return GTHREE_OBJECT (gthree_mesh_new (geometry, material));
}

static GthreeObject *
new_ball (GthreeMaterial *material)
{
  static GthreeGeometry *geometry = NULL;

  if (geometry == NULL)
    geometry = gthree_geometry_new_sphere (1, 1, 1);

  return GTHREE_OBJECT (gthree_mesh_new (geometry, material));
}

static GthreeObject *
gtk_logo (void)
{
  GPtrArray *materials;
  GthreeMeshLambertMaterial *material;
  GthreeObject *cube, *ball, *tube;
  GthreeMeshPhongMaterial *ball_material;
  graphene_point3d_t p;
  graphene_euler_t e;

  materials = g_ptr_array_new_with_free_func (g_object_unref);

  material = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material, red ());
  gthree_mesh_lambert_material_set_emissive_color (GTHREE_MESH_LAMBERT_MATERIAL (material), red ());
  gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (material), 0.75);
  g_ptr_array_add (materials, g_object_ref (material));
  g_ptr_array_add (materials, g_object_ref (material));
  g_object_unref (material);

  material = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material, green ());
  gthree_mesh_lambert_material_set_emissive_color (GTHREE_MESH_LAMBERT_MATERIAL (material), green ());
  gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (material), 0.75);
  g_ptr_array_add (materials, g_object_ref (material));
  g_ptr_array_add (materials, g_object_ref (material));
  g_object_unref (material);

  material = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material, blue ());
  gthree_mesh_lambert_material_set_emissive_color (GTHREE_MESH_LAMBERT_MATERIAL (material), blue ());
  gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (material), 0.75);
  g_ptr_array_add (materials, g_object_ref (material));
  g_ptr_array_add (materials, g_object_ref (material));
  g_object_unref (material);

  cube = new_cube (materials);

  ball_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_emissive_color (ball_material, white ());

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, 30, 30, 30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, -30, 30, 30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, 30, -30, 30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, 30, 30, -30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, -30, -30, -30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, 30, -30, -30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, -30, 30, -30));
  gthree_object_add_child (cube, ball);

  ball = new_ball (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (ball, graphene_point3d_init (&p, -30, -30, 30));
  gthree_object_add_child (cube, ball);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, -30, 0, 30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 30, 0, 30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 30, 0, -30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, -30, 0, -30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 90, 0, 0));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 30, 30, 0));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 90, 0, 0));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, -30, 30, 0));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 90, 0, 0));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 30, -30, 0));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 90, 0, 0));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, -30, -30, 0));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 0, 0, 90));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 0, 30, 30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 0, 0, 90));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 0, -30, 30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 0, 0, 90));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 0, 30, -30));
  gthree_object_add_child (cube, tube);

  tube = new_cylinder (GTHREE_MATERIAL (ball_material));
  gthree_object_set_rotation (tube, graphene_euler_init (&e, 0, 0, 90));
  gthree_object_set_position_point3d (tube, graphene_point3d_init (&p, 0, -30, -30));
  gthree_object_add_child (cube, tube);

  return cube;
}

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  GthreeAmbientLight *ambient_light;
  graphene_point3d_t pos;

  scene = gthree_scene_new ();

  gthree_scene_set_background_color (scene, grey ());

  ambient_light = gthree_ambient_light_new (white ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  logo = gtk_logo ();
  gthree_object_add_child (GTHREE_OBJECT (scene), logo);
  gthree_object_set_position_point3d (GTHREE_OBJECT (logo), graphene_point3d_init (&pos, 0, 0, 0));

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  graphene_euler_t euler;

  rot.x += 1.8;
  rot.y += 1.2;
  rot.z += 0.6;

  gthree_object_set_rotation (logo, graphene_euler_init (&euler, rot.x, rot.y, rot.z));

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

  window = examples_init ("GTK+ logo", &box, &done);

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
