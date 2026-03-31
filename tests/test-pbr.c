#include <math.h>
#include "test-common.h"
#include <gthree/gthreemeshphysicalmaterial.h>
#include <gthree/gthreepmremgenerator.h>

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

/* Physical material: envmap with clearcoat */
static void
test_physical_clearcoat (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float clearcoat[] = { 0.0, 0.5, 1.0 };
  for (int i = 0; i < 3; i++)
    {
      GthreeMeshPhysicalMaterial *mat = gthree_mesh_physical_material_new ();
      graphene_vec3_t blue;
      gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                graphene_vec3_init (&blue, 0.1, 0.2, 0.8));
      gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.0);
      gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.5);
      gthree_mesh_standard_material_set_env_map (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                  GTHREE_TEXTURE (cube_tex));
      gthree_mesh_physical_material_set_clearcoat (mat, clearcoat[i]);
      gthree_mesh_physical_material_set_clearcoat_roughness (mat, 0.1);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1) * 1.1, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
      g_object_unref (mat);
    }
}

/* Physical material: sheen (fabric-like) */
static void
test_physical_sheen (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  graphene_vec3_t sheen_colors[3];
  graphene_vec3_init (&sheen_colors[0], 0, 0, 0);
  graphene_vec3_init (&sheen_colors[1], 1, 0.5, 0);
  graphene_vec3_init (&sheen_colors[2], 0.5, 0, 1);

  for (int i = 0; i < 3; i++)
    {
      GthreeMeshPhysicalMaterial *mat = gthree_mesh_physical_material_new ();
      graphene_vec3_t red;
      gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                graphene_vec3_init (&red, 0.6, 0.1, 0.1));
      gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.0);
      gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.8);
      gthree_mesh_physical_material_set_sheen (mat, i == 0 ? 0.0 : 1.0);
      gthree_mesh_physical_material_set_sheen_color (mat, &sheen_colors[i]);
      gthree_mesh_physical_material_set_sheen_roughness (mat, 0.5);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1) * 1.1, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
      g_object_unref (mat);
    }
}

/* Physical material: transmission (glass-like) */
static void
test_physical_transmission (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);
  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  gthree_scene_set_background_texture (*scene, GTHREE_TEXTURE (cube_tex));

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float transmission[] = { 0.0, 0.5, 1.0 };
  for (int i = 0; i < 3; i++)
    {
      GthreeMeshPhysicalMaterial *mat = gthree_mesh_physical_material_new ();
      graphene_vec3_t white;
      gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                graphene_vec3_init (&white, 1, 1, 1));
      gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.0);
      gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.0);
      gthree_mesh_physical_material_set_transmission (mat, transmission[i]);
      gthree_mesh_physical_material_set_ior (mat, 1.5);
      gthree_mesh_physical_material_set_thickness (mat, 0.5);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1) * 1.1, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
      g_object_unref (mat);
    }
}

/* Physical material: iridescence */
static void
test_physical_iridescence (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float iridescence[] = { 0.0, 0.5, 1.0 };
  for (int i = 0; i < 3; i++)
    {
      GthreeMeshPhysicalMaterial *mat = gthree_mesh_physical_material_new ();
      graphene_vec3_t white;
      gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                graphene_vec3_init (&white, 0.9, 0.9, 0.9));
      gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (mat), 1.0);
      gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.2);
      gthree_mesh_standard_material_set_env_map (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                  GTHREE_TEXTURE (cube_tex));
      gthree_mesh_physical_material_set_iridescence (mat, iridescence[i]);
      gthree_mesh_physical_material_set_iridescence_ior (mat, 1.3);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1) * 1.1, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
      g_object_unref (mat);
    }
}

/* Physical material: envmap with varying IOR */
static void
test_physical_ior (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float iors[] = { 1.0, 1.5, 2.0, 2.5 };
  for (int i = 0; i < 4; i++)
    {
      GthreeMeshPhysicalMaterial *mat = gthree_mesh_physical_material_new ();
      graphene_vec3_t white;
      gthree_mesh_standard_material_set_color (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                graphene_vec3_init (&white, 0.9, 0.9, 0.9));
      gthree_mesh_standard_material_set_metalness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.0);
      gthree_mesh_standard_material_set_roughness (GTHREE_MESH_STANDARD_MATERIAL (mat), 0.0);
      gthree_mesh_standard_material_set_env_map (GTHREE_MESH_STANDARD_MATERIAL (mat),
                                                  GTHREE_TEXTURE (cube_tex));
      gthree_mesh_physical_material_set_ior (mat, iors[i]);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1.5) * 1.0, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
      g_object_unref (mat);
    }
}

/* Simplified WaterBottle body: gold metallic sphere at various roughness */
static void
test_standard_gold_metallic (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  /* Roughness: 0.1, 0.3, 0.6, 1.0 — all at metalness 1.0, gold color */
  float roughness[] = { 0.1, 0.3, 0.6, 1.0 };
  for (int i = 0; i < 4; i++)
    {
      g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
      graphene_vec3_t gold;
      gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&gold, 0.7, 0.55, 0.15));
      gthree_mesh_standard_material_set_metalness (mat, 1.0);
      gthree_mesh_standard_material_set_roughness (mat, roughness[i]);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1.5) * 1.0, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
    }
}

static GthreeScene *pmrem_scene = NULL;

static void
pmrem_renderer_setup (GthreeRenderer *renderer)
{
  if (!pmrem_scene)
    return;

  g_autoptr(GthreePMREMGenerator) gen = gthree_pmrem_generator_new (renderer);
  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  GthreeTexture *pmrem_tex = gthree_pmrem_generator_from_cubemap (gen, cube_tex);

  g_autoptr(GList) meshes = gthree_object_find_by_type (GTHREE_OBJECT (pmrem_scene), GTHREE_TYPE_MESH);
  for (GList *l = meshes; l; l = l->next)
    {
      GthreeMesh *mesh = l->data;
      GthreeMaterial *mat = gthree_mesh_get_material (mesh, 0);
      if (GTHREE_IS_MESH_STANDARD_MATERIAL (mat))
        gthree_mesh_standard_material_set_env_map (GTHREE_MESH_STANDARD_MATERIAL (mat), pmrem_tex);
    }
  g_object_unref (pmrem_tex);
}

static void
test_pmrem_metalness (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  pmrem_scene = *scene;
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  add_envmap_lights (*scene);

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.4, 32, 16);

  float metalness[] = { 0.0, 0.3, 0.7, 1.0 };
  for (int i = 0; i < 4; i++)
    {
      g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
      graphene_vec3_t red;
      gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&red, 0.8, 0.2, 0.2));
      gthree_mesh_standard_material_set_metalness (mat, metalness[i]);
      gthree_mesh_standard_material_set_roughness (mat, 0.3);

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
  register_test ("physical-clearcoat", test_physical_clearcoat);
  register_test ("physical-sheen", test_physical_sheen);
  register_test ("physical-transmission", test_physical_transmission);
  register_test ("physical-iridescence", test_physical_iridescence);
  register_test ("physical-ior", test_physical_ior);
  register_test ("standard-gold-metallic", test_standard_gold_metallic);
  register_test_full ("pmrem-metalness", test_pmrem_metalness, pmrem_renderer_setup);
}
