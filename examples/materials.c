#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>

static GthreeScene *scene;
static GthreePerspectiveCamera *camera;
static GthreeMesh *particle_light;

static GthreeMaterial *materials[14] = { NULL };
static GthreeGeometry *geometries[14] = { NULL };
static int n_materials = 0;
static int anim_material1, anim_material2;

static GdkRGBA grey = {0.4, 0.4, 0.4, 1.0};
static GdkRGBA black = {0, 0, 0, 1.0};
static GdkRGBA white = {1, 1, 1, 1.0};
static GdkRGBA red = {1, 0, 0, 1.0};
static GdkRGBA dark_green = {0, 0.6, 0, 1.0};
static GdkRGBA medium_grey = {0.4, 0.4, 0.4, 1.0};
static GdkRGBA dark_grey = {0.07, 0.07, 0.07, 1.0};
static GdkRGBA very_dark_grey = {0.012, 0.012, 0.012, 1.0};
static GdkRGBA light_grey = {0.87, 0.87, 0.87, 1.0};
static GdkRGBA orange = {1, 0.67, 0, 1.0};

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
          pixels[y * rowstride + x * 4 + 0] = 255;
          pixels[y * rowstride + x * 4 + 1] = 255;
          pixels[y * rowstride + x * 4 + 2] = 255;
          pixels[y * rowstride + x * 4 + 3] = x ^ (x == 0 ? y + 1 : y);
        }
    }

  return pixbuf;
}

static void
init_scene (void)
{
  GthreeGeometry *floor_geometry, *geometry_smooth, *geometry_light, *geometry_flat, *geometry_pieces;
  GthreeBasicMaterial *material_wireframe, *material_light, *material_basic;
  GthreeMultiMaterial *multi_material;
  GthreeNormalMaterial *material_normal;
  GthreeLambertMaterial *material_lambert;
  GthreePhongMaterial *material_phong;
  GthreeAmbientLight *ambient_light;
  GthreePointLight *point_light;
  GthreeDirectionalLight *directional_light;
  GthreeTexture *texture;
  GdkPixbuf *pixbuf;
  GthreeMesh *floor;
  int i;
  graphene_point3d_t pos = { 0, 0, 0};

  scene = gthree_scene_new ();

  pixbuf = generate_texture ();

  texture = gthree_texture_new (pixbuf);

  geometry_smooth = gthree_geometry_new_sphere (70, 32, 16);
  geometry_flat = gthree_geometry_new_sphere (70, 32, 16);
  geometry_pieces = gthree_geometry_new_sphere (70, 32, 16);

  camera = gthree_perspective_camera_new (45, 1, 1, 2000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 200, 2000));

  material_lambert = gthree_lambert_material_new ();
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_lambert), TRUE);
  gthree_basic_material_set_map (GTHREE_BASIC_MATERIAL (material_lambert), texture);
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_lambert = gthree_lambert_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material_lambert), &light_grey);
  gthree_basic_material_set_shading_type (GTHREE_BASIC_MATERIAL (material_lambert),
                                          GTHREE_SHADING_FLAT);
  geometries[n_materials] = geometry_flat;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_phong = gthree_phong_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material_phong), &light_grey);
  gthree_phong_material_set_ambient_color (material_phong, &very_dark_grey);
  gthree_phong_material_set_specular_color (material_phong, &dark_green);
  gthree_phong_material_set_shininess (material_phong, 30);
  gthree_basic_material_set_shading_type (GTHREE_BASIC_MATERIAL (material_phong),
                                          GTHREE_SHADING_FLAT);
  geometries[n_materials] = geometry_flat;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_normal = gthree_normal_material_new ();
  geometries[n_materials] = geometry_flat;
  materials[n_materials++] = GTHREE_MATERIAL (material_normal);

  material_basic = gthree_basic_material_new ();
  gthree_basic_material_set_color (material_basic, &orange);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_basic), TRUE);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (material_basic), GTHREE_BLEND_ADDITIVE, 0, 0 , 0);
  geometries[n_materials] = geometry_flat;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);
  /*
  material_basic = gthree_basic_material_new ();
  gthree_basic_material_set_color (material_basic, &red);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_basic), TRUE);
  gthree_material_set_blend_mode (GTHREE_MATERIAL (material_basic), GTHREE_BLEND_SUBTRACTIVE, 0, 0 , 0);
  geometries[n_materials] = geometry_flat;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);
  */

  material_lambert = gthree_lambert_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material_lambert), &light_grey);
  gthree_basic_material_set_shading_type (GTHREE_BASIC_MATERIAL (material_lambert),
                                          GTHREE_SHADING_SMOOTH);
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  material_phong = gthree_phong_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material_phong), &light_grey);
  gthree_phong_material_set_ambient_color (material_phong, &very_dark_grey);
  gthree_phong_material_set_specular_color (material_phong, &dark_green);
  gthree_phong_material_set_shininess (material_phong, 30);
  gthree_basic_material_set_shading_type (GTHREE_BASIC_MATERIAL (material_phong),
                                          GTHREE_SHADING_SMOOTH);
  gthree_basic_material_set_map (GTHREE_BASIC_MATERIAL (material_phong), texture);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_phong), TRUE);
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_normal = gthree_normal_material_new ();
  gthree_normal_material_set_shading_type (GTHREE_NORMAL_MATERIAL (material_normal),
                                          GTHREE_SHADING_SMOOTH);
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_normal);

  material_basic = gthree_basic_material_new ();
  gthree_material_set_is_wireframe (GTHREE_MATERIAL (material_basic), TRUE);
  gthree_basic_material_set_color (material_basic, &orange);
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);

#if TODO
  materials.push( new THREE.MeshDepthMaterial() );
#endif

  // TODO: Animate emissive
  material_lambert = gthree_lambert_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material_lambert), &medium_grey);
  gthree_basic_material_set_shading_type (GTHREE_BASIC_MATERIAL (material_lambert),
                                          GTHREE_SHADING_SMOOTH);
  gthree_lambert_material_set_emissive_color(material_lambert, &red);
  gthree_lambert_material_set_ambient_color(material_lambert, &black);
  anim_material1 = n_materials;
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_lambert);

  // TODO: Animate emissive
  material_phong = gthree_phong_material_new ();
  gthree_basic_material_set_color (GTHREE_BASIC_MATERIAL (material_phong), &black);
  gthree_phong_material_set_ambient_color (material_phong, &black);
  gthree_phong_material_set_specular_color (material_phong, &medium_grey);
  gthree_phong_material_set_emissive_color(material_phong, &red);
  gthree_phong_material_set_shininess (material_phong, 10);
  gthree_basic_material_set_shading_type (GTHREE_BASIC_MATERIAL (material_phong),
                                          GTHREE_SHADING_SMOOTH);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_phong), TRUE);
  gthree_material_set_opacity (GTHREE_MATERIAL (material_phong), 0.9);
  anim_material2 = n_materials;
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_phong);

  material_basic = gthree_basic_material_new ();
  gthree_basic_material_set_map (material_basic, texture);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (material_basic), TRUE);
  geometries[n_materials] = geometry_smooth;
  materials[n_materials++] = GTHREE_MATERIAL (material_basic);

  for (i = 0; i < gthree_geometry_get_n_faces (geometry_pieces); i++)
    {
      GthreeFace *face = gthree_geometry_get_face (geometry_pieces, i);
      gthree_face_set_material_index (face, g_random_int_range (0, n_materials));
    }
  multi_material = gthree_multi_material_new ();
  for (i = 0; i < n_materials; i++)
    gthree_multi_material_set_index (multi_material, i, materials[i]);

  geometries[n_materials] = geometry_pieces;
  materials[n_materials++] = GTHREE_MATERIAL (multi_material);

  floor_geometry = gthree_geometry_new_box (1000, 10, 1000,
                                            40, 1, 40);

  material_wireframe = gthree_basic_material_new ();
  gthree_material_set_is_wireframe (GTHREE_MATERIAL (material_wireframe), TRUE);
  gthree_basic_material_set_color (material_wireframe, &grey);

  floor = gthree_mesh_new (floor_geometry, GTHREE_MATERIAL (material_wireframe));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (floor));
  gthree_object_set_position (GTHREE_OBJECT (floor),
                              graphene_point3d_init (&pos, 0, -75, 0));


  for (i = 0; i < n_materials; i++)
    {
      GthreeMesh *sphere = gthree_mesh_new (geometries[i], materials[i]);
      gthree_object_set_position (GTHREE_OBJECT (sphere),
                                  graphene_point3d_init (&pos,
                                                         (i % 4 ) * 200 - 400,
                                                         0,
                                                         (i / 4) * 200 - 200));
      gthree_object_set_rotation (GTHREE_OBJECT (sphere),
                                  graphene_point3d_init (&pos,
                                                         g_random_double_range (0, 2*G_PI),
                                                         g_random_double_range (0, 2*G_PI),
                                                         g_random_double_range (0, 2*G_PI)));

      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (sphere));

      objects = g_list_prepend (objects, sphere);
    }

  ambient_light = gthree_ambient_light_new (&dark_grey);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  geometry_light = gthree_geometry_new_sphere (4, 8, 8);
  material_light = gthree_basic_material_new ();
  gthree_basic_material_set_color (material_light, &white);

  particle_light = gthree_mesh_new (geometry_light, GTHREE_MATERIAL (material_light));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (particle_light));

  point_light = gthree_point_light_new (&white, 1, 0);
  gthree_object_add_child (GTHREE_OBJECT (particle_light), GTHREE_OBJECT (point_light));

  directional_light = gthree_directional_light_new (&white, 0.125);
  gthree_object_set_position (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     1, 1, -1));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  graphene_point3d_t pos;
  graphene_point3d_t rot;
  GList *l;
  gint64 frame_time;
  float angle;
  GdkRGBA color;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  angle = frame_time / 4000000.0;

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos,
                                                     cos (angle) * 1000,
                                                     200,
                                                     sin (angle) * 1000));
  if (1)
    gthree_object_set_rotation (GTHREE_OBJECT (camera),
                                graphene_point3d_init (&pos, 0, G_PI/2 - angle, 0));
  else
    gthree_object_look_at (GTHREE_OBJECT (camera),
                           graphene_point3d_init (&pos, 0, 0, 0));

  for (l = objects; l != NULL; l = l->next)
    {
      GthreeObject *object = l->data;
      rot = *gthree_object_get_rotation (object);
      rot.x += 0.01;
      rot.y += 0.005;
      gthree_object_set_rotation (object, &rot);
    }

  if (anim_material1)
    {
      gtk_hsv_to_rgb (0.54, 1, 0.35 * (0.5 + 0.5 * sin (35 * angle)),
                      &color.red, &color.green, &color.blue);
      gthree_lambert_material_set_emissive_color (GTHREE_LAMBERT_MATERIAL (materials[anim_material1]), &color);
    }

  if (anim_material2)
    {
      gtk_hsv_to_rgb (0.04, 1, 0.35 * (0.5 + 0.5 * cos (35 * angle)),
                      &color.red, &color.green, &color.blue);
      gthree_phong_material_set_emissive_color (GTHREE_PHONG_MATERIAL (materials[anim_material2]), &color);
    }

  gthree_object_set_position (GTHREE_OBJECT (particle_light),
                              graphene_point3d_init (&pos,
                                                     sin (angle * 7) * 300,
                                                     cos (angle * 5) * 400,
                                                     cos (angle * 3) * 300));


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
  GtkWidget *window, *box, *hbox, *button, *area;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Model");
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

  init_scene ();
  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);
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
