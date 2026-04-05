#include <cairo.h>
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
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), G_PI);
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
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), G_PI);
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
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), G_PI);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 0.2 * G_PI, 0.2 * G_PI, 0.2 * G_PI));
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
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), G_PI);
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
  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, G_PI, G_PI, G_PI));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreePointLight *point = gthree_point_light_new (graphene_vec3_init (&white, 1, 1, 1), G_PI, 0);
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

/* Render-to-texture test: renders a blue sphere to a render target,
   then displays that texture on a cube in the main scene. */
static GthreeScene *rt_inner_scene;
static GthreeCamera *rt_inner_camera;
static GthreeRenderTarget *rt_target;
static GthreeMeshBasicMaterial *rt_cube_material;

static void
test_render_target (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 1, 1000));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 4);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t bg;
  gthree_scene_set_background_color (*scene, graphene_vec3_init (&bg, 0.2, 0.2, 0.2));

  rt_cube_material = gthree_mesh_basic_material_new ();
  g_autoptr(GthreeGeometry) geom = test_geometry_box ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (rt_cube_material));
  graphene_euler_t euler;
  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                               graphene_euler_init (&euler, 20, 30, 0));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));

  /* Inner scene: blue sphere with white light, red background */
  rt_inner_scene = gthree_scene_new ();
  gthree_scene_set_background_color (rt_inner_scene, graphene_vec3_init (&bg, 0.8, 0.2, 0.2));

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), G_PI);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (rt_inner_scene), GTHREE_OBJECT (light));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 0.3 * G_PI, 0.3 * G_PI, 0.3 * G_PI));
  gthree_object_add_child (GTHREE_OBJECT (rt_inner_scene), GTHREE_OBJECT (ambient));

  g_autoptr(GthreeMeshPhongMaterial) sphere_mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t blue;
  gthree_mesh_phong_material_set_color (sphere_mat, graphene_vec3_init (&blue, 0.2, 0.4, 0.9));
  g_autoptr(GthreeGeometry) sphere_geom = test_geometry_sphere ();
  GthreeMesh *sphere = gthree_mesh_new (sphere_geom, GTHREE_MATERIAL (sphere_mat));
  gthree_object_add_child (GTHREE_OBJECT (rt_inner_scene), GTHREE_OBJECT (sphere));

  rt_inner_camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 1, 1, 1000));
  gthree_object_set_position_xyz (GTHREE_OBJECT (rt_inner_camera), 0, 0, 3);
  gthree_object_add_child (GTHREE_OBJECT (rt_inner_scene), GTHREE_OBJECT (rt_inner_camera));

  rt_target = gthree_render_target_new (256, 256);
}

static void
render_target_pre_render (GthreeRenderer *renderer)
{
  gthree_renderer_set_render_target (renderer, rt_target, 0, 0);
  gthree_renderer_render (renderer, rt_inner_scene, rt_inner_camera);

  gthree_mesh_basic_material_set_map (rt_cube_material,
                                      gthree_render_target_get_texture (rt_target));

  gthree_renderer_set_render_target (renderer, NULL, 0, 0);
}

/* Cairo texture test: creates a cairo surface with drawn shapes,
   uses it as a texture on a cube. Tests GL_BGRA upload on GLES. */
static void
test_cairo_texture (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t bg;
  gthree_scene_set_background_color (*scene, graphene_vec3_init (&bg, 0.2, 0.2, 0.2));

  cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 256, 256);
  cairo_t *cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 0, 0);
  cairo_paint (cr);

  cairo_move_to (cr, 128.0, 25.6);
  cairo_line_to (cr, 230.4, 230.4);
  cairo_rel_line_to (cr, -102.4, 0.0);
  cairo_curve_to (cr, 51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
  cairo_close_path (cr);

  cairo_set_line_width (cr, 10.0);
  cairo_set_source_rgb (cr, 0, 0, 1);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_stroke (cr);
  cairo_destroy (cr);

  GthreeTexture *texture = gthree_texture_new_from_surface (surface);
  gthree_texture_set_flip_y (texture, FALSE);
  cairo_surface_destroy (surface);

  g_autoptr(GthreeMeshBasicMaterial) mat = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_map (mat, texture);

  g_autoptr(GthreeGeometry) geom = test_geometry_box ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  graphene_euler_t euler;
  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                               graphene_euler_init (&euler, 20, 30, 0));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

/* Cube camera test: a reflective sphere surrounded by colored objects.
   The cube camera renders the scene into a cube render target, which
   is used as the env map on the sphere material. Tests that all 6
   cube map faces render with correct orientation. */
static GthreeScene *cc_scene;
static GthreeCubeCamera *cc_cube_camera;

static void
test_cube_camera (GthreeScene **scene, GthreeCamera **camera)
{
  graphene_vec3_t color;

  cc_scene = gthree_scene_new ();
  *scene = cc_scene;

  gthree_scene_set_background_texture (*scene,
    GTHREE_TEXTURE (test_cube_texture_colored ()));

  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (60, 4.0/3.0, 1, 1000));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 20, 55);
  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&color, 0.5, 0.5, 0.5));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (graphene_vec3_init (&color, 1, 1, 1), 2.0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 30, 30, 30);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  GthreeRenderTarget *cube_rt = gthree_render_target_new_cube (128);
  cc_cube_camera = gthree_cube_camera_new (1, 1000, cube_rt);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (cc_cube_camera));

  /* Reflective sphere using the cube render target as env map */
  g_autoptr(GthreeMeshStandardMaterial) sphere_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_env_map (sphere_mat,
    gthree_render_target_get_texture (cube_rt));
  gthree_mesh_standard_material_set_roughness (sphere_mat, 0.05);
  gthree_mesh_standard_material_set_metalness (sphere_mat, 1.0);

  g_autoptr(GthreeGeometry) sphere_geom = gthree_geometry_new_icosahedron (10, 5);
  GthreeMesh *sphere = gthree_mesh_new (sphere_geom, GTHREE_MATERIAL (sphere_mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (sphere));

  /* Place colored boxes around the sphere so reflections are visible */
  g_autoptr(GthreeGeometry) box_geom = gthree_geometry_new_box (8, 8, 8, 1, 1, 1);

  g_autoptr(GthreeMeshStandardMaterial) red_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (red_mat, graphene_vec3_init (&color, 0.9, 0.1, 0.1));
  GthreeMesh *box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (red_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), 25, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));

  g_autoptr(GthreeMeshStandardMaterial) green_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (green_mat, graphene_vec3_init (&color, 0.1, 0.9, 0.1));
  box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (green_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), -25, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));

  g_autoptr(GthreeMeshStandardMaterial) blue_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (blue_mat, graphene_vec3_init (&color, 0.1, 0.1, 0.9));
  box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (blue_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), 0, 0, 25);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));

  g_autoptr(GthreeMeshStandardMaterial) yellow_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (yellow_mat, graphene_vec3_init (&color, 0.9, 0.9, 0.1));
  box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (yellow_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), 0, 0, -25);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));

  g_autoptr(GthreeMeshStandardMaterial) white_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (white_mat, graphene_vec3_init (&color, 0.9, 0.9, 0.9));
  box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (white_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), 0, 25, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));

  g_autoptr(GthreeMeshStandardMaterial) purple_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (purple_mat, graphene_vec3_init (&color, 0.7, 0.1, 0.9));
  box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (purple_mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), 0, -25, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));
}

static void
cube_camera_pre_render (GthreeRenderer *renderer)
{
  gthree_cube_camera_update (cc_cube_camera, renderer, cc_scene);
}

/* Scene environment test: the background cube texture is also set as
   scene.environment, so the standard material sphere picks it up
   automatically without an explicit env_map. */
static void
test_scene_environment (GthreeScene **scene, GthreeCamera **camera)
{
  graphene_vec3_t color;

  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  gthree_scene_set_background_texture (*scene, GTHREE_TEXTURE (cube_tex));
  gthree_scene_set_environment (*scene, GTHREE_TEXTURE (cube_tex));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&color, 0.3, 0.3, 0.3));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (graphene_vec3_init (&color, 1, 1, 1), G_PI);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 2, 2, 2);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_roughness (mat, 0.1);
  gthree_mesh_standard_material_set_metalness (mat, 0.9);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

/* Dynamic cubemap test: cube camera renders 6 faces per frame, then
   main render draws the scene. Tests that VAO caching works correctly
   when the same geometry is rendered multiple times per frame across
   different render passes. */
static GthreeScene *dc_scene;
static GthreeCubeCamera *dc_cube_camera;

static void
test_dynamic_cubemap (GthreeScene **scene, GthreeCamera **camera)
{
  graphene_vec3_t color;

  dc_scene = gthree_scene_new ();
  *scene = dc_scene;

  graphene_vec3_t red;
  graphene_vec3_init (&red, 0.8, 0.2, 0.2);
  gthree_scene_set_background_color (*scene, &red);

  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (60, 4.0/3.0, 1, 1000));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 20, 55);
  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&color, 0.5, 0.5, 0.5));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (graphene_vec3_init (&color, 1, 1, 1), 2.0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 30, 30, 30);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  GthreeRenderTarget *cube_rt = gthree_render_target_new_cube (64);
  dc_cube_camera = gthree_cube_camera_new (1, 1000, cube_rt);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dc_cube_camera));

  g_autoptr(GthreeMeshStandardMaterial) sphere_mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_env_map (sphere_mat,
      gthree_render_target_get_texture (cube_rt));
  gthree_mesh_standard_material_set_roughness (sphere_mat, 0.05);
  gthree_mesh_standard_material_set_metalness (sphere_mat, 1.0);

  g_autoptr(GthreeGeometry) sphere_geom = gthree_geometry_new_icosahedron (20, 5);
  GthreeMesh *sphere = gthree_mesh_new (sphere_geom, GTHREE_MATERIAL (sphere_mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (sphere));

  g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_roughness (mat, 0.1);

  g_autoptr(GthreeGeometry) box_geom = gthree_geometry_new_box (15, 15, 15, 1, 1, 1);
  GthreeMesh *box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (box), 30, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));

  g_autoptr(GthreeGeometry) torus_geom = gthree_geometry_new_torus_knot (8, 3, 128, 16, 2, 3);
  GthreeMesh *torus = gthree_mesh_new (torus_geom, GTHREE_MATERIAL (mat));
  gthree_object_set_position_xyz (GTHREE_OBJECT (torus), -35, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (torus));
}

static void
dynamic_cubemap_pre_render (GthreeRenderer *renderer)
{
  gthree_cube_camera_update (dc_cube_camera, renderer, dc_scene);
}

static void
test_standard_box (GthreeScene **scene, GthreeCamera **camera)
{
  graphene_vec3_t color;

  *scene = gthree_scene_new ();
  gthree_scene_set_background_color (*scene, graphene_vec3_init (&color, 0.2, 0.2, 0.2));

  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (60, 4.0/3.0, 1, 1000));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 20, 55);
  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&color, 0.5, 0.5, 0.5));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (graphene_vec3_init (&color, 1, 1, 1), 2.0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 30, 30, 30);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  g_autoptr(GthreeGeometry) box_geom = gthree_geometry_new_box (8, 8, 8, 1, 1, 1);
  g_autoptr(GthreeMeshStandardMaterial) mat = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (mat, graphene_vec3_init (&color, 0.9, 0.1, 0.1));
  GthreeMesh *box = gthree_mesh_new (box_geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (box));
}

void
register_feature_tests (void)
{
  register_test ("fog", test_fog);
  register_test ("transparency", test_transparency);
  register_test ("multi-material", test_multi_material);
  register_test ("double-sided", test_double_sided);
  register_test ("envmap", test_envmap);
  register_test_with_pre_render ("render-target", test_render_target, NULL, render_target_pre_render);
  register_test ("cairo-texture", test_cairo_texture);
  register_test_with_pre_render ("cube-camera", test_cube_camera, NULL, cube_camera_pre_render);
  register_test ("scene-environment", test_scene_environment);
  register_test_with_pre_render ("dynamic-cubemap", test_dynamic_cubemap, NULL, dynamic_cubemap_pre_render);
  register_test ("standard-box", test_standard_box);
}
