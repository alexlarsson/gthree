#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

#define INSTANCE_COUNT 1024
#define GRID_SIZE 32

GthreeScene *scene;
GthreeInstancedMesh *instanced_mesh;
GthreeAnimationMixer *mixer;
GthreeMesh *dummy;
float time_offsets[INSTANCE_COUNT];

static void
realize_area (GthreeArea *area)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (area);
  gthree_renderer_set_shadow_map_enabled (renderer, TRUE);
}

GthreeScene *
init_scene (void)
{
  GthreeLoader *loader;
  GthreeScene *loader_scene;
  GthreeGeometry *ground_geometry;
  GthreeMeshStandardMaterial *ground_material;
  GthreeMesh *ground;
  GthreeDirectionalLight *light;
  GthreeHemisphereLight *hemi;
  GthreeAnimationAction *action;
  graphene_vec3_t color, sky_color, ground_color;
  graphene_euler_t rotation;
  int i, x, y;

  scene = gthree_scene_new ();

  gthree_scene_set_background_color (scene, graphene_vec3_init (&color, 0.6, 0.87, 1.0));

  GthreeFog *fog = gthree_fog_new_linear (graphene_vec3_init (&color, 0.6, 0.87, 1.0), 5000, 10000);
  gthree_scene_set_fog (scene, fog);

  light = gthree_directional_light_new (graphene_vec3_init (&color, 1, 1, 1), 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (light), 200, 1000, 50);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (light), TRUE);

  GthreeLightShadow *shadow = gthree_light_get_shadow (GTHREE_LIGHT (light));
  GthreeCamera *shadow_camera = gthree_light_shadow_get_camera (shadow);
  if (GTHREE_IS_ORTHOGRAPHIC_CAMERA (shadow_camera))
    {
      gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -5000);
      gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 5000);
      gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), 5000);
      gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_camera), -5000);
    }
  gthree_camera_set_far (shadow_camera, 2000);
  gthree_light_shadow_set_bias (shadow, -0.01);

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (light));

  hemi = gthree_hemisphere_light_new (graphene_vec3_init (&sky_color, 0.6, 0.87, 1.0),
                                      graphene_vec3_init (&ground_color, 0.4, 0.6, 0.2),
                                      1.0 / 3.0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (hemi));

  ground_geometry = gthree_geometry_new_plane (100000, 100000, 1, 1);
  ground_material = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_color (ground_material, graphene_vec3_init (&color, 0.4, 0.6, 0.2));
  gthree_material_set_depth_write (GTHREE_MATERIAL (ground_material), TRUE);
  ground = gthree_mesh_new (ground_geometry, GTHREE_MATERIAL (ground_material));
  gthree_object_set_rotation (GTHREE_OBJECT (ground),
                              graphene_euler_init (&rotation, -90, 0, 0));
  gthree_object_set_receive_shadow (GTHREE_OBJECT (ground), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ground));

  loader = examples_load_gltl ("Horse.glb");

  loader_scene = gthree_loader_get_scene (loader, 0);

  g_autoptr(GList) meshes = gthree_object_find_by_type (GTHREE_OBJECT (loader_scene), GTHREE_TYPE_MESH);
  g_assert (meshes != NULL);
  dummy = GTHREE_MESH (g_object_ref (meshes->data));

  GthreeGeometry *geometry = gthree_mesh_get_geometry (dummy);

  GthreeMeshPhongMaterial *phong_material = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (phong_material, graphene_vec3_init (&color, 1.0, 1.0, 1.0));
  gthree_mesh_phong_material_set_flat_shading (phong_material, TRUE);
  gthree_mesh_material_set_morph_targets (GTHREE_MESH_MATERIAL (phong_material), TRUE);
  gthree_material_set_vertex_colors (GTHREE_MATERIAL (phong_material), TRUE);
  GthreeMaterial *material = GTHREE_MATERIAL (phong_material);

  instanced_mesh = gthree_instanced_mesh_new (geometry, material, INSTANCE_COUNT);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (instanced_mesh), TRUE);

  i = 0;
  for (x = 0; x < GRID_SIZE; x++)
    {
      for (y = 0; y < GRID_SIZE; y++)
        {
          graphene_matrix_t matrix;
          graphene_vec3_t instance_color;
          float px, pz;

          px = 5000 - 300 * x + 200 * g_random_double ();
          pz = 5000 - 300 * y;

          graphene_matrix_init_translate (&matrix,
                                          &GRAPHENE_POINT3D_INIT (px, 0, pz));

          gthree_instanced_mesh_set_matrix_at (instanced_mesh, i, &matrix);

          graphene_vec3_init (&instance_color,
                              g_random_double_range (0.3, 1.0),
                              g_random_double_range (0.1, 0.6),
                              g_random_double_range (0.0, 0.2));
          gthree_instanced_mesh_set_color_at (instanced_mesh, i, &instance_color);

          time_offsets[i] = g_random_double_range (0, 3);

          i++;
        }
    }

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (instanced_mesh));

  mixer = gthree_animation_mixer_new (GTHREE_OBJECT (loader_scene));
  action = gthree_animation_mixer_clip_action (mixer,
                                                gthree_loader_get_animation (loader, 0),
                                                NULL);
  gthree_animation_action_play (action);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
     GdkFrameClock *frame_clock,
     gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float elapsed;
  GthreePerspectiveCamera *camera = user_data;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  elapsed = (frame_time - first_frame_time) / (float)G_USEC_PER_SEC;

  float r = 3000;
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  sin (elapsed / 10.0) * r,
                                  1500 + 1000 * cos (elapsed / 5.0),
                                  cos (elapsed / 10.0) * r);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera), 0, 0, 0);

  if (instanced_mesh)
    {
      for (int i = 0; i < INSTANCE_COUNT; i++)
        {
          gthree_animation_mixer_set_time (mixer, elapsed + time_offsets[i]);
          gthree_instanced_mesh_set_morph_at (instanced_mesh, i, dummy);
        }
    }

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
  gboolean done = FALSE;

  window = examples_init ("Instancing + Morph Targets", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (60, 1, 100, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect (area, "realize", G_CALLBACK (realize_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, camera, NULL);

  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
