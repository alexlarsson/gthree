#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreeScene *scene;
static GthreePerspectiveCamera *camera;
static GthreePointLight *point_light;

static GthreeMaterial *materials[16] = { NULL };
static GthreeGeometry *geometries[16] = { NULL };
static int n_materials = 0;
static int anim_material1, anim_material2;


GList *objects;

static GdkPixbuf *
generate_texture()
{
  GdkPixbuf *pixbuf;
  int width, height, rowstride, x, y;
  guchar *pixels;

  width = 256;
  height = 256;

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);

  x = y = 0;
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          if (x < 128 && y < 128)
            {
              pixels[y * rowstride + x * 4 + 0] = 255;
              pixels[y * rowstride + x * 4 + 1] = 0;
              pixels[y * rowstride + x * 4 + 2] = 0;
            }
          else if (x >= 128 && y < 128)
            {
              pixels[y * rowstride + x * 4 + 0] = 0;
              pixels[y * rowstride + x * 4 + 1] = 255;
              pixels[y * rowstride + x * 4 + 2] = 0;
            }
          else if (x < 128 && y >= 128)
            {
              pixels[y * rowstride + x * 4 + 0] = 0;
              pixels[y * rowstride + x * 4 + 1] = 0;
              pixels[y * rowstride + x * 4 + 2] = 255;
            }
          else
            {
              pixels[y * rowstride + x * 4 + 0] = 255;
              pixels[y * rowstride + x * 4 + 1] = 255;
              pixels[y * rowstride + x * 4 + 2] = 0;
            }

          pixels[y * rowstride + x * 4 + 3] = x ^ (x == 0 ? y + 1 : y);
        }
    }

  return pixbuf;
}

static void
init_scene (void)
{
  GthreeGeometry *floor_geometry, *geometry, *geometry_light;
  GthreeMeshBasicMaterial *material_wireframe, *material_light, *material_basic;
  GPtrArray *multi_materials;
  GthreeMeshNormalMaterial *material_normal;
  GthreeMeshDepthMaterial *material_depth;
  GthreeMeshLambertMaterial *material_lambert;
  GthreeMeshPhongMaterial *material_phong;
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;
  GthreeTexture *texture;
  GdkPixbuf *pixbuf;
  GthreeMesh *floor;
  int i, n_faces;
  GthreeMesh *particle_light;

  scene = gthree_scene_new ();

  pixbuf = generate_texture ();

  texture = gthree_texture_new (pixbuf);

  geometry = gthree_geometry_new_sphere (70, 32, 16);

  camera = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera), 0, 200, 2000);

  material_lambert = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material_lambert, light_grey ());
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_phong = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material_phong, light_grey ());
  gthree_mesh_phong_material_set_flat_shading (material_phong, TRUE);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_phong = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material_phong, light_grey ());
  gthree_mesh_phong_material_set_specular_color (material_phong, dark_green ());
  gthree_mesh_phong_material_set_shininess (material_phong, 30);
  gthree_mesh_phong_material_set_flat_shading (material_phong, TRUE);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_phong = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material_phong, light_grey ());
  gthree_mesh_phong_material_set_specular_color (material_phong, dark_green ());
  gthree_mesh_phong_material_set_shininess (material_phong, 30);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_lambert = gthree_mesh_lambert_material_new ();
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_lambert), TRUE);
  gthree_mesh_lambert_material_set_map (material_lambert, texture);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_normal = gthree_mesh_normal_material_new ();
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_normal);

  material_basic = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_basic, orange ());
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_basic), TRUE);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (material_basic), GTHREE_BLEND_ADDITIVE, 0, 0 , 0);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);

  material_basic = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_basic, red ());
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_basic), TRUE);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (material_basic), GTHREE_BLEND_SUBTRACTIVE, 0, 0 , 0);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);

  material_phong = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material_phong, light_grey ());
  gthree_mesh_phong_material_set_specular_color (material_phong, dark_green ());
  gthree_mesh_phong_material_set_shininess (material_phong, 30);
  gthree_mesh_phong_material_set_map (material_phong, texture);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_phong), TRUE);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_normal = gthree_mesh_normal_material_new ();
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_normal);

  material_basic = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material_basic), TRUE);
  gthree_mesh_basic_material_set_color (material_basic, orange ());
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);

  material_depth = gthree_mesh_depth_material_new ();
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_depth);

  material_lambert = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material_lambert, medium_grey ());
  gthree_mesh_lambert_material_set_emissive_color (material_lambert, red ());
  anim_material1 = n_materials;
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_phong = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material_phong, black ());
  gthree_mesh_phong_material_set_specular_color (material_phong, medium_grey ());
  gthree_mesh_phong_material_set_emissive_color(material_phong, red ());
  gthree_mesh_phong_material_set_shininess (material_phong, 10);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_phong), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (material_phong), 0.9);
  anim_material2 = n_materials;
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_basic = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_map (material_basic, texture);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_basic), TRUE);
  geometries[n_materials] = geometry;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);

  n_faces = gthree_geometry_get_vertex_count (geometry) / 3;
  for (i = 0; i < n_faces; i += 2)
    gthree_geometry_add_group (geometry, 3*i, 3*2,
                               g_random_int_range (0, n_materials));


  multi_materials = g_ptr_array_new_with_free_func (g_object_unref);
  for (i = 0; i < n_materials; i++)
    g_ptr_array_add (multi_materials, g_object_ref (materials[i]));

  geometries[n_materials] = geometry;
  materials[n_materials++] = NULL; // Multi materials

  floor_geometry = gthree_geometry_new_box (1000, 10, 1000,
                                            40, 1, 40);

  material_wireframe = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material_wireframe), TRUE);
  gthree_mesh_basic_material_set_color (material_wireframe, grey ());

  floor = gthree_mesh_new (floor_geometry, GTHREE_MATERIAL (material_wireframe));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (floor));
  gthree_object_set_position_xyz (GTHREE_OBJECT (floor), 0, -75, 0);


  for (i = 0; i < n_materials; i++)
    {
      GthreeMesh *sphere = gthree_mesh_new (geometries[i], materials[i]);
      if (materials[i] == NULL)
        gthree_mesh_set_materials (sphere, multi_materials);
      gthree_object_set_position_xyz (GTHREE_OBJECT (sphere),
                                      (i % 4 ) * 200 - 400,
                                      0,
                                      (i / 4) * 200 - 200);
      gthree_object_set_rotation_xyz (GTHREE_OBJECT (sphere),
                                      g_random_double_range (0, 360),
                                      g_random_double_range (0, 360),
                                      g_random_double_range (0, 360));

      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (sphere));

      objects = g_list_prepend (objects, sphere);
    }

  ambient_light = gthree_ambient_light_new (dark_grey ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (4, 8, 8);
  material_light = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_color (material_light, white ());

  point_light = gthree_point_light_new (white (), 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (point_light));

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (point_light), GTHREE_OBJECT (particle_light));

  directional_light = gthree_directional_light_new (white (), 0.125);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light),
                                  1, 1, -1);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  const graphene_euler_t *old_rot;
  graphene_euler_t rot;
  GList *l;
  gint64 frame_time;
  static gint64 first_frame_time = 0;
  float angle;
  graphene_vec3_t color;
#ifdef USE_GTK4
  float r, g, b;
#else
  double r, g, b;
#endif

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;
  angle = (frame_time - first_frame_time) / 4000000.0;

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  cos (angle) * 1000,
                                  200,
                                  sin (angle) * 1000);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera), 0, 0, 0);

  for (l = objects; l != NULL; l = l->next)
    {
      GthreeObject *object = l->data;
      old_rot = gthree_object_get_rotation (object);

      graphene_euler_init (&rot,
                           graphene_euler_get_x (old_rot) + 1.0,
                           graphene_euler_get_y (old_rot) + 0.5,
                           0);
      gthree_object_set_rotation (object, &rot);
    }

  if (anim_material1)
    {
      gtk_hsv_to_rgb (0.54, 1, 0.35 * (0.5 + 0.5 * sin (35 * angle)),
                      &r, &g, &b);
      graphene_vec3_init (&color, r, g, b);
      gthree_mesh_lambert_material_set_emissive_color (GTHREE_MESH_LAMBERT_MATERIAL (materials[anim_material1]), &color);
    }

  if (anim_material2)
    {
      gtk_hsv_to_rgb (0.04, 1, 0.35 * (0.5 + 0.5 * cos (35 * angle)),
                      &r, &g, &b);
      graphene_vec3_init (&color, r, g, b);
      gthree_mesh_phong_material_set_emissive_color (GTHREE_MESH_PHONG_MATERIAL (materials[anim_material2]), &color);
    }

  gthree_object_set_position_xyz (GTHREE_OBJECT (point_light),
                                  sin (angle * 7) * 300,
                                  cos (angle * 5) * 400,
                                  cos (angle * 3) * 300);


  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height)
{
  gthree_perspective_camera_set_aspect (camera, (float)width / (float)(height));
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area;
  gboolean done = FALSE;

  window = examples_init ("Materials", &box, &done);

  init_scene ();
  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_box_append (GTK_BOX (box), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
