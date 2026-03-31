#include "test-common.h"

static void
test_fog (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t fog_color;
  GthreeFog *fog = gthree_fog_new_linear (graphene_vec3_init (&fog_color, 0.5, 0.5, 0.7), 2, 8);
  gthree_scene_set_fog (*scene, fog);
  gthree_scene_set_background_color (*scene, &fog_color);

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  g_autoptr(GthreeMeshLambertMaterial) mat = gthree_mesh_lambert_material_new ();
  graphene_vec3_t red;
  gthree_mesh_lambert_material_set_color (mat, graphene_vec3_init (&red, 0.8, 0.2, 0.2));

  g_autoptr(GthreeGeometry) geom = test_geometry_box ();
  for (int i = 0; i < 5; i++)
    {
      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 2) * 1.5, 0, -i);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
    }
}

static void
test_transparency (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();

  g_autoptr(GthreeMeshPhongMaterial) mat1 = gthree_mesh_phong_material_new ();
  graphene_vec3_t red;
  gthree_mesh_phong_material_set_color (mat1, graphene_vec3_init (&red, 1, 0, 0));
  gthree_material_set_is_transparent (GTHREE_MATERIAL (mat1), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (mat1), 0.5);

  g_autoptr(GthreeMeshPhongMaterial) mat2 = gthree_mesh_phong_material_new ();
  graphene_vec3_t blue;
  gthree_mesh_phong_material_set_color (mat2, graphene_vec3_init (&blue, 0, 0, 1));

  GthreeMesh *mesh1 = gthree_mesh_new (geom, GTHREE_MATERIAL (mat1));
  gthree_object_set_position_xyz (GTHREE_OBJECT (mesh1), 0.3, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh1));

  GthreeMesh *mesh2 = gthree_mesh_new (geom, GTHREE_MATERIAL (mat2));
  gthree_object_set_position_xyz (GTHREE_OBJECT (mesh2), -0.3, 0, -0.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh2));
}

static void
test_multi_material (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 0.2, 0.2, 0.2));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  graphene_vec3_t colors[4];
  graphene_vec3_init (&colors[0], 1, 0, 0);
  graphene_vec3_init (&colors[1], 0, 1, 0);
  graphene_vec3_init (&colors[2], 0, 0, 1);
  graphene_vec3_init (&colors[3], 1, 1, 0);

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  for (int i = 0; i < 4; i++)
    {
      g_autoptr(GthreeMeshPhongMaterial) mat = gthree_mesh_phong_material_new ();
      gthree_mesh_phong_material_set_color (mat, &colors[i]);
      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1.5) * 1.0, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
    }
}

static void
test_double_sided (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 1, 3);
  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  g_autoptr(GthreeMeshPhongMaterial) mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t cyan;
  gthree_mesh_phong_material_set_color (mat, graphene_vec3_init (&cyan, 0.2, 0.8, 0.8));
  gthree_material_set_side (GTHREE_MATERIAL (mat), GTHREE_SIDE_DOUBLE);

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_plane (2, 2, 1, 1);
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  graphene_euler_t euler;
  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                               graphene_euler_init (&euler, -30, 45, 0));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_envmap (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 1, 1, 1));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreePointLight *point = gthree_point_light_new (graphene_vec3_init (&white, 1, 1, 1), 1, 0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (point), 2, 2, 2);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (point));

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();

  g_autoptr(GthreeMeshLambertMaterial) mat = gthree_mesh_lambert_material_new ();
  graphene_vec3_init (&white, 1, 1, 1);
  gthree_mesh_lambert_material_set_color (mat, &white);
  gthree_mesh_lambert_material_set_env_map (mat, GTHREE_TEXTURE (cube_tex));

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

void
register_feature_tests (void)
{
  register_test ("fog", test_fog);
  register_test ("transparency", test_transparency);
  register_test ("multi-material", test_multi_material);
  register_test ("double-sided", test_double_sided);
  register_test ("envmap", test_envmap);
}
