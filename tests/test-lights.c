#include "test-common.h"

static void
test_point_light (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white, red, blue;
  graphene_vec3_init (&white, 1, 1, 1);

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 0.1, 0.1, 0.1));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreePointLight *light1 = gthree_point_light_new (graphene_vec3_init (&red, 1, 0, 0), 1, 0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light1), 2, 1, 2);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light1));

  GthreePointLight *light2 = gthree_point_light_new (graphene_vec3_init (&blue, 0, 0, 1), 1, 0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light2), -2, 1, 2);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light2));

  g_autoptr(GthreeMeshPhongMaterial) mat = gthree_mesh_phong_material_new ();
  graphene_vec3_init (&white, 0.8, 0.8, 0.8);
  gthree_mesh_phong_material_set_color (mat, &white);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_hemisphere_light (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t sky, ground;
  GthreeHemisphereLight *hemi = gthree_hemisphere_light_new (
    graphene_vec3_init (&sky, 0.6, 0.8, 1.0),
    graphene_vec3_init (&ground, 0.3, 0.2, 0.1), 1.0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (hemi), 0, 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (hemi));

  g_autoptr(GthreeMeshLambertMaterial) mat = gthree_mesh_lambert_material_new ();
  graphene_vec3_t white;
  gthree_mesh_lambert_material_set_color (mat, graphene_vec3_init (&white, 0.9, 0.9, 0.9));

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

static void
test_shadow (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 2, 5);

  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  graphene_vec3_init (&white, 1, 1, 1);

  GthreeAmbientLight *ambient = gthree_ambient_light_new (&white);
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), 0.3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (&white, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 2, 4, 3);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (dir), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  {
    GthreeLightShadow *shadow = gthree_light_get_shadow (GTHREE_LIGHT (dir));
    GthreeCamera *shadow_cam = gthree_light_shadow_get_camera (shadow);
    gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), -10);
    gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), 10);
    gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), 10);
    gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), -10);
    gthree_camera_set_near (shadow_cam, 0.1);
    gthree_camera_set_far (shadow_cam, 50);
  }

  g_autoptr(GthreeMeshPhongMaterial) sphere_mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t red;
  gthree_mesh_phong_material_set_color (sphere_mat, graphene_vec3_init (&red, 0.8, 0.2, 0.2));

  g_autoptr(GthreeGeometry) sphere_geom = test_geometry_sphere ();
  GthreeMesh *sphere = gthree_mesh_new (sphere_geom, GTHREE_MATERIAL (sphere_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (sphere), 0, 1.2, 0);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (sphere), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (sphere));

  g_autoptr(GthreeMeshPhongMaterial) floor_mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t grey;
  gthree_mesh_phong_material_set_color (floor_mat, graphene_vec3_init (&grey, 0.6, 0.6, 0.6));

  g_autoptr(GthreeGeometry) floor_geom = gthree_geometry_new_plane (10, 10, 1, 1);
  GthreeMesh *floor_mesh = gthree_mesh_new (floor_geom, GTHREE_MATERIAL (floor_mat));
  graphene_euler_t euler;
  gthree_object_set_rotation (GTHREE_OBJECT (floor_mesh),
                               graphene_euler_init (&euler, -90, 0, 0));
  gthree_object_set_receive_shadow (GTHREE_OBJECT (floor_mesh), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (floor_mesh));
}

static void
shadow_renderer_setup (GthreeRenderer *renderer)
{
  gthree_renderer_set_shadow_map_enabled (renderer, TRUE);
}

void
register_light_tests (void)
{
  register_test ("point-light", test_point_light);
  register_test ("hemisphere-light", test_hemisphere_light);
  register_test_full ("shadow", test_shadow, shadow_renderer_setup);
}
