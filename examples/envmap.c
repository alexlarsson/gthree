#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

/* In this example we draw two scenes, with the first one being the
   sky box. We could just add the skybox as a regular box, but then we
   would need to clear the background, and then overdraw it, and then
   increase the camera clip to very large values to ensure its
   visible, which is not ideal.
*/
GthreeScene *scene, *scene_cube;
GthreeMesh *obj1, *obj2, *obj3;
static GthreePerspectiveCamera *camera;
static GthreePerspectiveCamera *camera_cube;

static gboolean
skybox_render (GtkGLArea    *gl_area,
               GdkGLContext *context)
{
  GthreeArea *area = GTHREE_AREA (gl_area);

  gthree_renderer_set_autoclear (gthree_area_get_renderer (area), TRUE);
  gthree_renderer_set_autoclear_color (gthree_area_get_renderer (area), TRUE);
  gthree_renderer_render (gthree_area_get_renderer (area),
                          scene_cube,
                          GTHREE_CAMERA (camera_cube));
  gthree_renderer_set_autoclear (gthree_area_get_renderer (area), FALSE);
  gthree_renderer_set_autoclear_color (gthree_area_get_renderer (area), FALSE);


  return FALSE;
}

static void
area_realize (GtkWidget *widget)
{
  GthreeArea *area = GTHREE_AREA (widget);
  GthreeRenderer *renderer;

  renderer = gthree_area_get_renderer (area);

  gthree_renderer_set_autoclear (renderer, FALSE);
  gthree_renderer_set_autoclear_color (renderer, FALSE);
}

GthreeScene *
init_scene (void)
{
  GthreeGeometry *geometry;
  GthreeMesh *skybox;
  GthreeLambertMaterial *material, *material2;
  GthreeShader *shader;
  GthreeUniforms *uniforms;
  GthreeUniform *uni;
  GthreeShaderMaterial *shader_material;
  GthreeCubeTexture *reflectionCube, *refractionCube;
  GdkPixbuf *pixbufs[6];
  GthreeAmbientLight *ambient_light;
  GthreePointLight *point_light;
  graphene_point3d_t pos;

  examples_load_cube_pixbufs ("cube/SwedishRoyalCastle", pixbufs);

  reflectionCube = gthree_cube_texture_new_from_array (pixbufs);
  refractionCube = gthree_cube_texture_new_from_array (pixbufs);
  gthree_texture_set_mapping (GTHREE_TEXTURE (refractionCube), GTHREE_MAPPING_CUBE_REFRACTION);

  material = gthree_lambert_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material), &white);
  gthree_lambert_material_set_ambient_color (material, &light_grey);
  gthree_basic_material_set_env_map (GTHREE_BASIC_MATERIAL (material), GTHREE_TEXTURE (reflectionCube));

  material2 = gthree_lambert_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material2), &yellow);
  gthree_lambert_material_set_ambient_color (material2, &white);
  gthree_basic_material_set_refraction_ratio (GTHREE_BASIC_MATERIAL (material2), 0.99);
  gthree_basic_material_set_env_map (GTHREE_BASIC_MATERIAL (material2), GTHREE_TEXTURE (refractionCube));

  scene = gthree_scene_new ();
  scene_cube = gthree_scene_new ();

  geometry = gthree_geometry_new_sphere (40, 32, 16);
  obj1 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position (GTHREE_OBJECT (obj1),
                              graphene_point3d_init (&pos,
                                                     -80,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj1));

  geometry = gthree_geometry_new_sphere (40, 32, 16);
  obj2 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material2));
  gthree_object_set_position (GTHREE_OBJECT (obj2),
                              graphene_point3d_init (&pos,
                                                     0,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj2));

  geometry = gthree_geometry_new_box (70, 70, 70, 1, 1, 1);
  obj3 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position (GTHREE_OBJECT (obj3),
                              graphene_point3d_init (&pos,
                                                     80,
                                                     0,
                                                     0));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (obj3));

  shader = gthree_clone_shader_from_library ("cube");
  uniforms = gthree_shader_get_uniforms (shader);
  uni = gthree_uniforms_lookup_from_string (uniforms, "tCube");
  g_assert (uni != NULL);
  gthree_uniform_set_texture (uni, GTHREE_TEXTURE (reflectionCube));

  shader_material = gthree_shader_material_new (shader);
  gthree_material_set_depth_write (GTHREE_MATERIAL (shader_material), FALSE);
  gthree_material_set_side (GTHREE_MATERIAL (shader_material), GTHREE_SIDE_BACK);

  geometry = gthree_geometry_new_box (100, 100, 100, 1, 1, 1);
  skybox = gthree_mesh_new (geometry, GTHREE_MATERIAL (shader_material));
  gthree_object_add_child (GTHREE_OBJECT (scene_cube), GTHREE_OBJECT (skybox));

  ambient_light = gthree_ambient_light_new (&white);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  point_light = gthree_point_light_new (&white, 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  gthree_object_set_position (GTHREE_OBJECT (point_light),
                              graphene_point3d_init (&pos,
                                                     1000,
                                                     800,
                                                     400));

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  graphene_euler_t euler;
  graphene_point3d_t pos;
  float relative_time, camera_angle, camera_height;
  int cursor_x, cursor_y;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  /* Control camera with mouse */
  gtk_widget_get_pointer (widget, &cursor_x, &cursor_y);
  camera_angle = (cursor_x)  * 2.0 * G_PI / gtk_widget_get_allocated_width (widget) - G_PI / 2.0;
  camera_height = (((float)cursor_y / gtk_widget_get_allocated_height (widget)) - 0.5) * 500;

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos,
                                                     cos (camera_angle) * 500,
                                                     camera_height,
                                                     sin (camera_angle) * 500));
  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&pos, 0, 0, 0));

  gthree_object_set_quaternion (GTHREE_OBJECT (camera_cube),
                                gthree_object_get_quaternion (GTHREE_OBJECT (camera)));


  gthree_object_set_rotation (GTHREE_OBJECT (obj3),
                              graphene_euler_init (&euler,
                                                   0.5 * relative_time,
                                                   0.2 * relative_time,
                                                   0.0 * relative_time
                                                   ));

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
  gthree_perspective_camera_set_aspect (camera_cube, (float)width / (float)(height));
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;
  graphene_point3d_t pos;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Environment map");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (box), 6);
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show (box);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_container_add (GTK_CONTAINER (box), hbox);
  gtk_widget_show (hbox);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 5000);
  camera_cube = gthree_perspective_camera_new (30, 1, 1, 100);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_add_child (GTHREE_OBJECT (scene_cube), GTHREE_OBJECT (camera_cube));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 500));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect (area, "render", G_CALLBACK (skybox_render), NULL);
  g_signal_connect (area, "realize", G_CALLBACK (area_realize), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
