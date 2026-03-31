#include "test-common.h"

static void
test_basic_color (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  g_autoptr(GthreeMeshBasicMaterial) mat = gthree_mesh_basic_material_new ();
  graphene_vec3_t red;
  gthree_mesh_basic_material_set_color (mat, graphene_vec3_init (&red, 1, 0, 0));

  g_autoptr(GthreeGeometry) geom = test_geometry_box ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                               graphene_euler_alloc ());
  graphene_euler_t euler;
  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                               graphene_euler_init (&euler, 30, 45, 0));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_basic_texture (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  g_autoptr(GthreeTexture) tex = test_load_texture ("checkerboard.png");
  g_autoptr(GthreeMeshBasicMaterial) mat = gthree_mesh_basic_material_new ();
  if (tex)
    gthree_mesh_basic_material_set_map (mat, tex);

  g_autoptr(GthreeGeometry) geom = test_geometry_box ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  graphene_euler_t euler;
  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                               graphene_euler_init (&euler, 30, 45, 0));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_wireframe (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  g_autoptr(GthreeMeshBasicMaterial) mat = gthree_mesh_basic_material_new ();
  graphene_vec3_t green;
  gthree_mesh_basic_material_set_color (mat, graphene_vec3_init (&green, 0, 1, 0));
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (mat), TRUE);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

void
register_basic_tests (void)
{
  register_test ("basic-color", test_basic_color);
  register_test ("basic-texture", test_basic_texture);
  register_test ("wireframe", test_wireframe);
}
