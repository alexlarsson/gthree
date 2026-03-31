#include <math.h>
#include "test-common.h"
#include <gthree/gthreemeshstandardmaterial.h>

static void
add_envmap_lights (GthreeScene *scene)
{
  graphene_vec3_t white;
  graphene_vec3_init (&white, 1, 1, 1);

  GthreeAmbientLight *ambient = gthree_ambient_light_new (&white);
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), 0.4);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (&white, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 2, 3, 4);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (dir));
}

/* Standard material: envmap with metallic sphere */
static void
test_envmap_standard (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();

  g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
  graphene_vec3_t gold;
  gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&gold, 0.8, 0.6, 0.2));
  gthree_mesh_standard_material_set_metalness (mat, 0.3);
  gthree_mesh_standard_material_set_roughness (mat, 0.3);
  gthree_mesh_standard_material_set_env_map (mat, GTHREE_TEXTURE (cube_tex));

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

/* Standard material: 4 spheres with varying roughness */
static void
test_standard_roughness (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float roughness[] = { 0.0, 0.3, 0.6, 1.0 };
  for (int i = 0; i < 4; i++)
    {
      g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
      graphene_vec3_t gold;
      gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&gold, 0.8, 0.6, 0.2));
      gthree_mesh_standard_material_set_metalness (mat, 0.3);
      gthree_mesh_standard_material_set_roughness (mat, roughness[i]);
      gthree_mesh_standard_material_set_env_map (mat, GTHREE_TEXTURE (cube_tex));

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1.5) * 1.0, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
    }
}

/* Standard material: 4 spheres with varying metalness */
static void
test_standard_metalness (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float metalness[] = { 0.0, 0.3, 0.7, 1.0 };
  for (int i = 0; i < 4; i++)
    {
      g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
      graphene_vec3_t red;
      gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&red, 0.8, 0.2, 0.2));
      gthree_mesh_standard_material_set_metalness (mat, metalness[i]);
      gthree_mesh_standard_material_set_roughness (mat, 0.3);
      gthree_mesh_standard_material_set_env_map (mat, GTHREE_TEXTURE (cube_tex));

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1.5) * 1.0, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
    }
}


void
register_pbr_tests (void)
{
  register_test ("envmap-standard", test_envmap_standard);
  register_test ("standard-roughness", test_standard_roughness);
  register_test ("standard-metalness", test_standard_metalness);
}
