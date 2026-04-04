#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "orbitcontrols.h"

static GthreeScene *scene;
static GthreePerspectiveCamera *camera;
static GthreeCubeCamera *cube_camera;
static GthreeRenderTarget *cube_render_target;
static GthreeMesh *sphere, *cube, *torus;
static GthreeMeshStandardMaterial *sphere_material;
static GthreeDirectionalLight *dir_light;
static GthreeOrbitControls *orbit;

static void
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeMeshStandardMaterial *material2;
  GthreeCubeTexture *background_cube;
  GdkPixbuf *pixbufs[6];
  GthreeAmbientLight *ambient_light;

  scene = gthree_scene_new ();

  examples_load_cube_pixbufs ("cube/Park2", pixbufs);
  background_cube = gthree_cube_texture_new_from_array (pixbufs);
  gthree_texture_set_encoding (GTHREE_TEXTURE (background_cube), GTHREE_ENCODING_FORMAT_SRGB);
  gthree_scene_set_background_texture (scene, GTHREE_TEXTURE (background_cube));

  cube_render_target = gthree_render_target_new_cube (256);

  cube_camera = gthree_cube_camera_new (1, 1000, cube_render_target);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube_camera));

  sphere_material = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_env_map (sphere_material,
                                             gthree_render_target_get_texture (cube_render_target));
  gthree_mesh_standard_material_set_roughness (sphere_material, 0.05);
  gthree_mesh_standard_material_set_metalness (sphere_material, 1.0);

  geometry = gthree_geometry_new_icosahedron (15, 8);
  sphere = gthree_mesh_new (geometry, GTHREE_MATERIAL (sphere_material));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (sphere));

  material2 = gthree_mesh_standard_material_new ();
  gthree_mesh_standard_material_set_env_map (material2, GTHREE_TEXTURE (background_cube));
  gthree_mesh_standard_material_set_roughness (material2, 0.1);
  gthree_mesh_standard_material_set_metalness (material2, 0.0);

  geometry = gthree_geometry_new_box (15, 15, 15, 1, 1, 1);
  cube = gthree_mesh_new (geometry, GTHREE_MATERIAL (material2));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (cube));

  geometry = gthree_geometry_new_torus_knot (8, 3, 128, 16, 2, 3);
  torus = gthree_mesh_new (geometry, GTHREE_MATERIAL (material2));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (torus));

  ambient_light = gthree_ambient_light_new (white ());
  gthree_light_set_intensity (GTHREE_LIGHT (ambient_light), 0.5);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  dir_light = gthree_directional_light_new (white (), 4.0);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir_light), 50, 50, 50);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (dir_light));
}

static gboolean
tick (GtkWidget     *widget,
     GdkFrameClock *frame_clock,
     gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float time;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  time = (frame_time - first_frame_time) / (float) G_USEC_PER_SEC;

  gthree_object_set_position_xyz (GTHREE_OBJECT (cube),
                                  cosf (time) * 30,
                                  sinf (time) * 30,
                                  sinf (time) * 30);
  gthree_object_set_rotation_xyz (GTHREE_OBJECT (cube),
                                  time * 1.2,
                                  time * 1.8,
                                  0);

  gthree_object_set_position_xyz (GTHREE_OBJECT (torus),
                                  cosf (time + 10) * 30,
                                  sinf (time + 10) * 30,
                                  sinf (time + 10) * 30);
  gthree_object_set_rotation_xyz (GTHREE_OBJECT (torus),
                                  time * 1.2,
                                  time * 1.8,
                                  0);

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

static gboolean
render_area (GtkGLArea    *gl_area,
             GdkGLContext *context)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (GTHREE_AREA (gl_area));

  gthree_cube_camera_update (cube_camera, renderer, scene);

  gthree_renderer_set_render_target (renderer, NULL, 0, 0);
  gthree_renderer_render (renderer, scene, GTHREE_CAMERA (camera));

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area;
  gboolean done = FALSE;

  window = examples_init ("Dynamic Cube Reflection", &box, &done);

  init_scene ();

  camera = gthree_perspective_camera_new (60, 1, 1, 1000);
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 0, 75);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect (area, "render", G_CALLBACK (render_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  orbit = gthree_orbit_controls_new (GTHREE_OBJECT (camera), GTK_WIDGET (area));
  gthree_orbit_controls_set_auto_rotate (orbit, TRUE);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
