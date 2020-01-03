#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreePerspectiveCamera *camera;
GthreeObject *intersected;
graphene_vec3_t intersected_color;

cairo_surface_t *texture_surface;
GthreeTexture *texture;

void
update_surface (float u, float v)
{
  cairo_t *cr = cairo_create (texture_surface);

  // Flip y for OpenGL
  cairo_scale (cr, 1, -1);
  cairo_translate (cr, 0, -256);

  cairo_set_source_rgb (cr, 0, 1, 0);
  cairo_paint (cr);

  // box primitive has v == 0 at the bottom, so flip y here.
  cairo_arc (cr, 256 * u, 256 - 256 * v, 20, 0, 2 * M_PI);
  cairo_close_path (cr);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, 6);
  cairo_stroke (cr);

  cairo_destroy (cr);

  gthree_texture_set_needs_update (texture);
}

static void
init_texture (void)
{
  texture_surface =  cairo_image_surface_create (CAIRO_FORMAT_RGB24, 256, 256);

  cairo_t *cr = cairo_create (texture_surface);
  cairo_set_source_rgb (cr, 0, 1, 0);
  cairo_paint (cr);
  cairo_destroy (cr);

  texture = gthree_texture_new_from_surface (texture_surface);
  gthree_texture_set_flip_y (texture, FALSE); // We'll just draw upside down to avoid performance penalty
}

GthreeScene *
init_scene (void)
{
  graphene_vec3_t color, pos, scale;
  graphene_euler_t rotation;
  int i;

  init_texture ();

  g_autoptr(GthreeGeometry) geometry = gthree_geometry_new_box (20, 42, 20, 1, 1, 1);
  g_autoptr(GthreeDirectionalLight) directional_light = NULL;
  g_autoptr(GthreeAmbientLight) ambient_light = NULL;

  scene = gthree_scene_new ();

  for (i = 0; i < 2000; i++)
    {
      g_autoptr(GthreeMesh) mesh = gthree_mesh_new (geometry, NULL);
      g_object_set_data (G_OBJECT (mesh), "index", GINT_TO_POINTER(i));

      graphene_vec3_init (&color,
                          g_random_double (),
                          g_random_double (),
                          g_random_double ());
      graphene_vec3_init (&pos,
                          g_random_double_range (-400, 400),
                          g_random_double_range (-400, 400),
                          g_random_double_range (-400, 400));
      graphene_euler_init (&rotation,
                           g_random_double_range (0, 360),
                           g_random_double_range (0, 360),
                           g_random_double_range (0, 360));
      graphene_vec3_init (&scale,
                          g_random_double_range (0.5, 1.5),
                          g_random_double_range (0.5, 1.5),
                          g_random_double_range (0.5, 1.5));

      for (int j = 0; j < 6; j++)
        {
          g_autoptr(GthreeMeshLambertMaterial) material =  gthree_mesh_lambert_material_new ();

          gthree_mesh_lambert_material_set_color (material, &color);

          gthree_mesh_set_material (mesh, j, GTHREE_MATERIAL (material));
        }

      gthree_object_set_position (GTHREE_OBJECT (mesh), &pos);
      gthree_object_set_scale (GTHREE_OBJECT (mesh), &scale);
      gthree_object_set_rotation (GTHREE_OBJECT (mesh), &rotation);

      gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));
    }

  directional_light = gthree_directional_light_new (white (), 1.0);
  graphene_vec3_init (&pos, 1, 1, 1);
  gthree_object_set_position (GTHREE_OBJECT (directional_light), &pos);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  ambient_light = gthree_ambient_light_new (dark_grey ());
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  return scene;
}

static int cursor_x, cursor_y;

static void
motion_cb (GtkEventControllerMotion *controller,
           gdouble                   x,
           gdouble                   y,
           gpointer                  user_data)
{
  cursor_x = x;
  cursor_y = y;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float relative_time;
  graphene_vec3_t pos;
  graphene_point3d_t point;
  float x, y;
  g_autoptr(GthreeRaycaster) raycaster = gthree_raycaster_new ();
  g_autoptr(GPtrArray) intersections = NULL;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  float radius = 100;
  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_vec3_init (&pos,
                                                  radius * sinf (relative_time / 200),
                                                  radius * sinf (relative_time / 200),
                                                  radius * cosf (relative_time / 200)));

  gthree_object_look_at (GTHREE_OBJECT (camera),
                         graphene_point3d_init (&point,
                                                0, 0, 0));
  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), FALSE);

  x = ((float)cursor_x / gtk_widget_get_allocated_width (widget)) * 2 - 1;
  y = -((float)cursor_y / gtk_widget_get_allocated_height (widget)) * 2 + 1;

  gthree_raycaster_set_from_camera  (raycaster, GTHREE_CAMERA (camera), x, y);

  intersections = gthree_raycaster_intersect_object (raycaster, GTHREE_OBJECT (scene), TRUE, NULL);
  if (intersected)
    {
      for (int j = 0; j < 6; j++)
        {
          GthreeMaterial *material = gthree_mesh_get_material (GTHREE_MESH (intersected), j);
          gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), &intersected_color);
          gthree_mesh_lambert_material_set_map (GTHREE_MESH_LAMBERT_MATERIAL (material), NULL);
        }
      intersected = NULL;
    }

  if (intersections->len > 0)
    {
      GthreeRayIntersection *intersection = g_ptr_array_index (intersections, 0);
      intersected = intersection->object;

      update_surface (graphene_vec2_get_x (&intersection->uv),
                      graphene_vec2_get_y (&intersection->uv));

      intersected_color = *gthree_mesh_lambert_material_get_color (GTHREE_MESH_LAMBERT_MATERIAL (gthree_mesh_get_material (GTHREE_MESH (intersected), 0)));
      for (int j = 0; j < 6; j++)
        {
          GthreeMaterial *material = gthree_mesh_get_material (GTHREE_MESH (intersected), j);
          if (j == intersection->material_index)
            {
              gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), white ());
              gthree_mesh_lambert_material_set_map (GTHREE_MESH_LAMBERT_MATERIAL (material), texture);
            }
          else
            {
              gthree_mesh_lambert_material_set_color (GTHREE_MESH_LAMBERT_MATERIAL (material), red ());
              gthree_mesh_lambert_material_set_map (GTHREE_MESH_LAMBERT_MATERIAL (material), NULL);
            }
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
  graphene_point3d_t pos;
  GtkEventController *motion;

  window = examples_init ("Interactive", &box);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (70, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (box), area);
  gtk_widget_show (area);

  motion = motion_controller_for (GTK_WIDGET (area));
  g_signal_connect (motion, "motion", (GCallback)motion_cb, NULL);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
