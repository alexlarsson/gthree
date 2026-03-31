#include "test-common.h"

static GthreeDirectionalLight *
add_dir_light (GthreeScene *scene)
{
  graphene_vec3_t white;
  graphene_vec3_init (&white, 1, 1, 1);
  GthreeDirectionalLight *light = gthree_directional_light_new (&white, G_PI);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (light));

  graphene_vec3_t ambient_color;
  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&ambient_color, 0.3 * G_PI, 0.3 * G_PI, 0.3 * G_PI));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient));

  return light;
}

static void
test_lambert (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_dir_light (*scene);

  g_autoptr(GthreeMeshLambertMaterial) mat = gthree_mesh_lambert_material_new ();
  graphene_vec3_t blue;
  gthree_mesh_lambert_material_set_color (mat, graphene_vec3_init (&blue, 0.2, 0.4, 0.9));

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_phong (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_dir_light (*scene);

  g_autoptr(GthreeMeshPhongMaterial) mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t red, white;
  gthree_mesh_phong_material_set_color (mat, graphene_vec3_init (&red, 0.8, 0.1, 0.1));
  gthree_mesh_phong_material_set_specular_color (mat, graphene_vec3_init (&white, 1, 1, 1));
  gthree_mesh_phong_material_set_shininess (mat, 30);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_standard (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_dir_light (*scene);

  g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
  graphene_vec3_t gold;
  gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&gold, 0.8, 0.6, 0.2));
  gthree_mesh_standard_material_set_metalness (mat, 0.8);
  gthree_mesh_standard_material_set_roughness (mat, 0.3);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_toon (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_dir_light (*scene);

  g_autoptr(GthreeMeshToonMaterial) mat = gthree_mesh_toon_material_new ();
  graphene_vec3_t blue;
  gthree_mesh_toon_material_set_color (mat, graphene_vec3_init (&blue, 0.1, 0.3, 0.9));

  g_autoptr(GthreeGeometry) geom = test_geometry_torus_knot ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

void
register_material_tests (void)
{
  register_test ("lambert", test_lambert);
  register_test ("phong", test_phong);
  register_test ("standard", test_standard);
  register_test ("toon", test_toon);
}
