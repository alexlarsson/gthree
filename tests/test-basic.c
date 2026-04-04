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

static void
colorise_faces (GthreeGeometry *geometry)
{
  int count = gthree_geometry_get_position_count (geometry);
  GthreeAttribute *color = gthree_geometry_add_attribute (geometry, "color",
                                                          gthree_attribute_new ("color", GTHREE_ATTRIBUTE_TYPE_FLOAT, count,
                                                                                3, FALSE));
  graphene_vec3_t colors[6];
  graphene_vec3_init (&colors[0], 1, 0, 0);
  graphene_vec3_init (&colors[1], 0, 1, 0);
  graphene_vec3_init (&colors[2], 0, 0, 1);
  graphene_vec3_init (&colors[3], 0, 1, 1);
  graphene_vec3_init (&colors[4], 1, 0, 1);
  graphene_vec3_init (&colors[5], 1, 1, 0);

  for (int i = 0; i < count / 4; i++)
    for (int j = 0; j < 4; j++)
      gthree_attribute_set_vec3 (color, i * 4 + j, &colors[i]);

  g_object_unref (color);
}

static void
test_cubes (GthreeScene **scene, GthreeCamera **camera)
{
  graphene_vec3_t color;
  graphene_euler_t euler;

  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (30, 4.0/3.0, 1, 10000));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 400);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  g_autoptr(GthreeTexture) texture = test_load_texture ("checkerboard.png");

  /* Shared geometry with face colors */
  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_box (50, 50, 50, 1, 1, 1);
  colorise_faces (geom);
  g_autoptr(GthreeGeometry) sub_geom = gthree_geometry_new_box (20, 20, 20, 1, 1, 1);
  colorise_faces (sub_geom);

  /* Material: plain grey, no vertex colors */
  g_autoptr(GthreeMeshBasicMaterial) mat_simple = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (mat_simple, graphene_vec3_init (&color, 0.5, 0.5, 0.5));

  /* Material: vertex colors */
  g_autoptr(GthreeMeshBasicMaterial) mat_vcolor = gthree_mesh_basic_material_new ();
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (mat_vcolor), TRUE);

  /* Material: wireframe */
  g_autoptr(GthreeMeshBasicMaterial) mat_wire = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (mat_wire), TRUE);
  gthree_mesh_basic_material_set_color (mat_wire, graphene_vec3_init (&color, 1, 1, 0));

  /* Material: textured */
  g_autoptr(GthreeMeshBasicMaterial) mat_tex = gthree_mesh_basic_material_new ();
  if (texture)
    gthree_mesh_basic_material_set_map (mat_tex, texture);

  /* Multi-material array: 2x vertex color, 2x wireframe, 2x textured */
  g_autoptr(GPtrArray) multi = g_ptr_array_new_with_free_func (g_object_unref);
  g_ptr_array_add (multi, g_object_ref (mat_vcolor));
  g_ptr_array_add (multi, g_object_ref (mat_vcolor));
  g_ptr_array_add (multi, g_object_ref (mat_wire));
  g_ptr_array_add (multi, g_object_ref (mat_wire));
  g_ptr_array_add (multi, g_object_ref (mat_tex));
  g_ptr_array_add (multi, g_object_ref (mat_tex));

  struct { GthreeMaterial *mat; GPtrArray *mats; float x; float y; } cubes[] = {
    { GTHREE_MATERIAL (mat_simple), NULL, -80, 40 },
    { GTHREE_MATERIAL (mat_vcolor), NULL, 0, 40 },
    { GTHREE_MATERIAL (mat_tex), NULL, 80, 40 },
    { GTHREE_MATERIAL (mat_wire), NULL, -80, -40 },
    { GTHREE_MATERIAL (mat_vcolor), NULL, 0, -40 },
    { NULL, multi, 80, -40 },
  };

  graphene_euler_init (&euler, 0, 30, 15);

  for (int i = 0; i < G_N_ELEMENTS (cubes); i++)
    {
      GthreeMesh *mesh = gthree_mesh_new (geom, cubes[i].mat);
      if (cubes[i].mats)
        gthree_mesh_set_materials (mesh, cubes[i].mats);
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), cubes[i].x, cubes[i].y, 0);
      gthree_object_set_rotation (GTHREE_OBJECT (mesh), &euler);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));

      GthreeMesh *sub = gthree_mesh_new (sub_geom, cubes[i].mat);
      if (cubes[i].mats)
        gthree_mesh_set_materials (sub, cubes[i].mats);
      gthree_object_set_position_xyz (GTHREE_OBJECT (sub), 0, 35, 0);
      gthree_object_add_child (GTHREE_OBJECT (mesh), GTHREE_OBJECT (sub));
    }
}

void
register_basic_tests (void)
{
  register_test ("basic-color", test_basic_color);
  register_test ("basic-texture", test_basic_texture);
  register_test ("wireframe", test_wireframe);
  register_test ("cubes", test_cubes);
}
