#include <math.h>
#include "test-common.h"
#include <gthree/gthreearea.h>

static GthreeLoader *
load_gltf_model (const char *name)
{
  g_autofree char *path = g_build_filename (SRCDIR, "..", "examples", "models", name, NULL);
  g_autoptr(GError) error = NULL;
  char *data;
  gsize size;

  if (!g_file_get_contents (path, &data, &size, &error))
    {
      g_warning ("Failed to load %s: %s", path, error->message);
      return NULL;
    }

  g_autoptr(GBytes) bytes = g_bytes_new_take (data, size);
  GthreeLoader *loader = gthree_loader_parse_gltf (bytes, NULL, &error);
  if (!loader)
    g_warning ("Failed to parse %s: %s", name, error->message);

  return loader;
}

static void
reparent_children (GthreeObject *from, GthreeObject *to)
{
  g_autoptr(GPtrArray) children = g_ptr_array_new_with_free_func (g_object_unref);
  GthreeObjectIter iter;
  GthreeObject *child;

  gthree_object_iter_init (&iter, from);
  while (gthree_object_iter_next (&iter, &child))
    g_ptr_array_add (children, g_object_ref (child));

  for (guint i = 0; i < children->len; i++)
    {
      child = g_ptr_array_index (children, i);
      gthree_object_remove_child (from, child);
      gthree_object_add_child (to, child);
    }
}

/* Clipping plane test */
static void
clipping_renderer_setup (GthreeRenderer *renderer)
{
  gthree_renderer_set_local_clipping_enabled (renderer, TRUE);
}

static void
test_clipping (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 2.8);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 0.3, 0.3, 0.3));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  g_autoptr(GthreeMeshPhongMaterial) mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t red;
  gthree_mesh_phong_material_set_color (mat, graphene_vec3_init (&red, 0.8, 0.2, 0.2));
  gthree_material_set_side (GTHREE_MATERIAL (mat), GTHREE_SIDE_DOUBLE);

  graphene_plane_t clip_plane;
  graphene_vec3_t normal;
  graphene_plane_init (&clip_plane, graphene_vec3_init (&normal, 0, 1, 0), 0.0);
  gthree_material_set_clipping_plane (GTHREE_MATERIAL (mat), 0, &clip_plane);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

/* Sprite rendering */
static void
test_sprites (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t colors[4];
  graphene_vec3_init (&colors[0], 1, 0, 0);
  graphene_vec3_init (&colors[1], 0, 1, 0);
  graphene_vec3_init (&colors[2], 0, 0, 1);
  graphene_vec3_init (&colors[3], 1, 1, 0);

  for (int i = 0; i < 4; i++)
    {
      g_autoptr(GthreeSpriteMaterial) mat = gthree_sprite_material_new ();
      gthree_sprite_material_set_color (mat, &colors[i]);
      GthreeSprite *sprite = gthree_sprite_new (GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (sprite), (i - 1.5) * 1.2, 0, 0);
      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (sprite));
    }
}

/* Refraction environment map */
static void
test_envmap_refraction (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 1.5, 1.0, 2.0);
  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 1, 1, 1));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreePointLight *point = gthree_point_light_new (graphene_vec3_init (&white, 1, 1, 1), 1, 0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (point), 2, 2, 2);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (point));

  g_autoptr(GthreeCubeTexture) bg_tex = test_cube_texture_colored ();
  gthree_scene_set_background_texture (*scene, GTHREE_TEXTURE (bg_tex));

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();
  gthree_texture_set_mapping (GTHREE_TEXTURE (cube_tex), GTHREE_MAPPING_CUBE_REFRACTION);

  g_autoptr(GthreeMeshLambertMaterial) mat = gthree_mesh_lambert_material_new ();
  graphene_vec3_init (&white, 1, 1, 1);
  gthree_mesh_lambert_material_set_color (mat, &white);
  gthree_mesh_lambert_material_set_env_map (mat, GTHREE_TEXTURE (cube_tex));
  gthree_mesh_lambert_material_set_refraction_ratio (mat, 0.7);

  g_autoptr(GthreeGeometry) geom = test_geometry_sphere ();
  GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
}

/* GLTF model helper */
static void
setup_gltf_test (GthreeScene **scene, GthreeCamera **camera, const char *model_name)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.01, 1000));

  graphene_vec3_t white;
  graphene_vec3_init (&white, 1, 1, 1);
  GthreeAmbientLight *ambient = gthree_ambient_light_new (&white);
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), 0.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (&white, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  g_autoptr(GthreeCubeTexture) cube_tex = test_cube_texture_colored ();

  GthreeLoader *loader = load_gltf_model (model_name);
  if (!loader)
    return;

  GthreeScene *model_scene = gthree_loader_get_scene (loader, 0);

  int n_mats = gthree_loader_get_n_materials (loader);
  for (int i = 0; i < n_mats; i++)
    {
      GthreeMaterial *m = gthree_loader_get_material (loader, i);
      if (g_object_class_find_property (G_OBJECT_GET_CLASS (m), "env-map"))
        g_object_set (m, "env-map", GTHREE_TEXTURE (cube_tex), NULL);
    }

  reparent_children (GTHREE_OBJECT (model_scene), GTHREE_OBJECT (*scene));

  gthree_object_update_matrix_world (GTHREE_OBJECT (*scene), TRUE);

  graphene_box_t bbox;
  graphene_box_init_from_box (&bbox, graphene_box_empty ());
  gthree_object_get_mesh_extents (GTHREE_OBJECT (*scene), &bbox);
  graphene_vec3_t size;
  graphene_point3d_t center;
  graphene_box_get_size (&bbox, &size);
  float radius = graphene_vec3_length (&size) * 0.5f;
  graphene_box_get_center (&bbox, &center);

  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera),
                                   center.x, center.y, center.z + radius * 2.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  g_object_unref (loader);
}

static void test_gltf_waterbottle (GthreeScene **s, GthreeCamera **c) { setup_gltf_test (s, c, "WaterBottle.glb"); }
static void test_gltf_soldier (GthreeScene **s, GthreeCamera **c) { setup_gltf_test (s, c, "Soldier.glb"); }
static void test_gltf_robot (GthreeScene **s, GthreeCamera **c) { setup_gltf_test (s, c, "RobotExpressive.glb"); }

/* Morph target test */
static void
test_morph_target (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.1, 100));
  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 0, 3.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));

  graphene_vec3_t white;
  GthreeDirectionalLight *light = gthree_directional_light_new (graphene_vec3_init (&white, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (light));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (graphene_vec3_init (&white, 0.3, 0.3, 0.3));
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  g_autoptr(GthreeGeometry) geom = gthree_geometry_new_sphere (0.5, 16, 12);

  GthreeAttribute *pos_attr = gthree_geometry_get_position (geom);
  int n_verts = gthree_attribute_get_count (pos_attr);

  g_autoptr(GthreeAttributeArray) morph_array = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_FLOAT, n_verts, 3);
  g_autoptr(GthreeAttribute) morph_attr = gthree_attribute_new_with_array ("morph0", morph_array, FALSE);

  for (int i = 0; i < n_verts; i++)
    {
      float x, y, z;
      gthree_attribute_get_xyz (pos_attr, i, &x, &y, &z);
      float len = sqrtf (x*x + y*y + z*z);
      float scale = 0.3f * sinf (y * 10.0f);
      gthree_attribute_set_xyz (morph_attr, i,
                                 x + x/len * scale,
                                 y + y/len * scale,
                                 z + z/len * scale);
    }

  gthree_geometry_add_morph_attribute (geom, "position", morph_attr);

  float influences[] = { 0.0, 0.5, 1.0 };
  for (int i = 0; i < 3; i++)
    {
      g_autoptr(GthreeMeshPhongMaterial) mat = gthree_mesh_phong_material_new ();
      graphene_vec3_t green;
      gthree_mesh_phong_material_set_color (mat, graphene_vec3_init (&green, 0.2, 0.7, 0.3));
      gthree_mesh_material_set_morph_targets (GTHREE_MESH_MATERIAL (mat), TRUE);

      GthreeMesh *mesh = gthree_mesh_new (geom, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), (i - 1) * 1.3, 0, 0);

      GArray *targets = gthree_mesh_get_morph_targets (mesh);
      if (targets && targets->len > 0)
        g_array_index (targets, float, 0) = influences[i];

      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (mesh));
    }
}

/* Skeletal animation: 3 poses of the Soldier model */
static void
test_skeletal_animation (GthreeScene **scene, GthreeCamera **camera)
{
  *scene = gthree_scene_new ();
  *camera = GTHREE_CAMERA (gthree_perspective_camera_new (45, 4.0/3.0, 0.01, 1000));

  graphene_vec3_t white;
  graphene_vec3_init (&white, 1, 1, 1);
  GthreeAmbientLight *ambient = gthree_ambient_light_new (&white);
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), 0.5);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir = gthree_directional_light_new (&white, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir), 1, 2, 3);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (dir));

  float times[] = { 0.0, 0.3, 0.7 };

  for (int i = 0; i < 3; i++)
    {
      GthreeLoader *loader = load_gltf_model ("Soldier.glb");
      if (!loader)
        return;

      GthreeScene *model_scene = gthree_loader_get_scene (loader, 0);
      GthreeGroup *group = gthree_group_new ();
      gthree_object_set_position_xyz (GTHREE_OBJECT (group), (i - 1) * 1.2, 0, 0);

      reparent_children (GTHREE_OBJECT (model_scene), GTHREE_OBJECT (group));

      if (gthree_loader_get_n_animations (loader) > 0)
        {
          GthreeAnimationClip *clip = gthree_loader_get_animation (loader, 0);
          GthreeAnimationMixer *mixer = gthree_animation_mixer_new (GTHREE_OBJECT (group));
          GthreeAnimationAction *action = gthree_animation_mixer_clip_action (mixer, clip, NULL);
          gthree_animation_action_set_loop_mode (action, GTHREE_LOOP_MODE_ONCE, 1);
          gthree_animation_action_play (action);
          gthree_animation_mixer_update (mixer, times[i]);
          gthree_object_update_matrix_world (GTHREE_OBJECT (group), TRUE);
          g_object_unref (mixer);
        }

      gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (group));
      g_object_unref (loader);
    }

  gthree_object_set_position_xyz (GTHREE_OBJECT (*camera), 0, 1, 4);
  gthree_object_look_at_xyz (GTHREE_OBJECT (*camera), 0, 0.8, 0);
  gthree_object_add_child (GTHREE_OBJECT (*scene), GTHREE_OBJECT (*camera));
}

void
register_advanced_tests (void)
{
  register_test_full ("clipping", test_clipping, clipping_renderer_setup);
  register_test ("sprites", test_sprites);
  register_test ("envmap-refraction", test_envmap_refraction);
  register_test ("gltf-waterbottle", test_gltf_waterbottle);
  register_test ("gltf-soldier", test_gltf_soldier);
  register_test ("gltf-robot", test_gltf_robot);
  register_test ("morph-target", test_morph_target);
  register_test ("skeletal-animation", test_skeletal_animation);
}
