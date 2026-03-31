#include <stdlib.h>
#include <string.h>
#include <epoxy/gl.h>
#include <gtk/gtk.h>
#include <gthree/gthree.h>
#include <gthree/gthreearea.h>
#include "test-common.h"

#define TEST_WIDTH 400
#define TEST_HEIGHT 300
#define MAX_TESTS 64

static TestCase tests[MAX_TESTS];
static int n_tests = 0;

void
register_test (const char *name, TestSetupFunc setup)
{
  tests[n_tests].name = name;
  tests[n_tests].setup = setup;
  tests[n_tests].renderer_setup = NULL;
  n_tests++;
}

void
register_test_full (const char *name, TestSetupFunc setup, TestRendererFunc renderer_setup)
{
  tests[n_tests].name = name;
  tests[n_tests].setup = setup;
  tests[n_tests].renderer_setup = renderer_setup;
  n_tests++;
}

GthreeTexture *
test_load_texture (const char *filename)
{
  g_autofree char *path = g_build_filename (SRCDIR, "textures", filename, NULL);
  g_autoptr(GError) error = NULL;
  g_autoptr(GdkPixbuf) pixbuf = gdk_pixbuf_new_from_file (path, &error);
  if (!pixbuf)
    {
      g_warning ("Failed to load %s: %s", path, error->message);
      return NULL;
    }
  GthreeTexture *texture = gthree_texture_new (pixbuf);
  gthree_texture_set_encoding (texture, GTHREE_ENCODING_FORMAT_SRGB);
  return texture;
}

GthreeGeometry *
test_geometry_box (void)
{
  return gthree_geometry_new_box (1, 1, 1, 1, 1, 1);
}

GthreeGeometry *
test_geometry_sphere (void)
{
  return gthree_geometry_new_sphere (1.0, 32, 16);
}

GthreeGeometry *
test_geometry_plane (void)
{
  return gthree_geometry_new_plane (3, 3, 1, 1);
}

GthreeGeometry *
test_geometry_torus_knot (void)
{
  return gthree_geometry_new_torus_knot (0.6, 0.2, 128, 32, 2, 3);
}

static GdkPixbuf *
make_checker_pixbuf (int r, int g, int b)
{
  int size = 16;
  int cell = 4;
  GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, size, size);
  guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
  int stride = gdk_pixbuf_get_rowstride (pixbuf);
  for (int y = 0; y < size; y++)
    for (int x = 0; x < size; x++)
      {
        guchar *p = pixels + y * stride + x * 3;
        if (((x / cell) + (y / cell)) % 2)
          { p[0] = r; p[1] = g; p[2] = b; }
        else
          { p[0] = 0; p[1] = 0; p[2] = 0; }
      }
  return pixbuf;
}

GthreeCubeTexture *
test_cube_texture_colored (void)
{
  GdkPixbuf *faces[6];
  faces[0] = make_checker_pixbuf (255,   0,   0);
  faces[1] = make_checker_pixbuf (  0, 255, 255);
  faces[2] = make_checker_pixbuf (  0, 255,   0);
  faces[3] = make_checker_pixbuf (255,   0, 255);
  faces[4] = make_checker_pixbuf (  0,   0, 255);
  faces[5] = make_checker_pixbuf (255, 255,   0);

  GthreeCubeTexture *tex = gthree_cube_texture_new_from_array (faces);
  gthree_texture_set_encoding (GTHREE_TEXTURE (tex), GTHREE_ENCODING_FORMAT_SRGB);
  for (int i = 0; i < 6; i++)
    g_object_unref (faces[i]);

  return tex;
}

static TestCase *
find_test (const char *name)
{
  for (int i = 0; i < n_tests; i++)
    if (strcmp (tests[i].name, name) == 0)
      return &tests[i];
  return NULL;
}

static void
list_tests (void)
{
  g_print ("Available tests:\n");
  for (int i = 0; i < n_tests; i++)
    g_print ("  %s\n", tests[i].name);
}

typedef struct {
  GthreeScene *scene;
  GthreeCamera *camera;
  char *output_file;
  int frame_count;
  gboolean done;
  gboolean interactive;
  TestRendererFunc renderer_setup;
  gboolean renderer_configured;
} RenderData;

static gboolean
on_render (GtkGLArea *gl_area, GdkGLContext *context, gpointer user_data)
{
  RenderData *data = user_data;
  GthreeArea *area = GTHREE_AREA (gl_area);

  if (!data->renderer_configured && data->renderer_setup)
    {
      data->renderer_setup (gthree_area_get_renderer (area));
      data->renderer_configured = TRUE;
    }

  data->frame_count++;

  if (data->frame_count < 3)
    {
      gtk_widget_queue_draw (GTK_WIDGET (area));
      return FALSE;
    }

  if (data->output_file && !data->done)
    {
      int width = gtk_widget_get_width (GTK_WIDGET (area));
      int height = gtk_widget_get_height (GTK_WIDGET (area));

      if (width > 0 && height > 0)
        {
          guchar *pixels = g_malloc (width * height * 4);
          glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

          guchar *flipped = g_malloc (width * height * 4);
          for (int y = 0; y < height; y++)
            memcpy (flipped + y * width * 4,
                    pixels + (height - 1 - y) * width * 4,
                    width * 4);

          GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (flipped, GDK_COLORSPACE_RGB, TRUE, 8,
                                                          width, height, width * 4, NULL, NULL);
          g_autoptr(GError) error = NULL;
          if (!gdk_pixbuf_savev (pixbuf, data->output_file, "png", NULL, NULL, &error))
            g_warning ("Failed to save %s: %s", data->output_file, error->message);
          else
            g_print ("Saved %s\n", data->output_file);

          g_object_unref (pixbuf);
          g_free (pixels);
          g_free (flipped);
        }
    }

  if (!data->interactive)
    data->done = TRUE;

  return FALSE;
}

static gboolean
tick_cb (GtkWidget *widget, GdkFrameClock *clock, gpointer user_data)
{
  gtk_widget_queue_draw (widget);
  return G_SOURCE_CONTINUE;
}

static gboolean
timeout_cb (gpointer user_data)
{
  RenderData *data = user_data;
  data->done = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
on_close_request (GtkWindow *window, gpointer user_data)
{
  RenderData *data = user_data;
  data->done = TRUE;
  return TRUE;
}

static int
run_test (TestCase *test, const char *output_file, gboolean interactive, int timeout_ms)
{
  GthreeScene *scene = NULL;
  GthreeCamera *camera = NULL;
  GtkWidget *window, *area;
  RenderData data = {0};

  test->setup (&scene, &camera);
  if (!scene || !camera)
    {
      g_warning ("Test %s failed to create scene/camera", test->name);
      return 1;
    }

  data.renderer_setup = test->renderer_setup;

  data.output_file = g_strdup (output_file);
  data.interactive = interactive;

  window = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), test->name);
  gtk_window_set_default_size (GTK_WINDOW (window), TEST_WIDTH, TEST_HEIGHT);

  area = gthree_area_new (scene, camera);
  g_signal_connect (area, "render", G_CALLBACK (on_render), &data);
  gtk_window_set_child (GTK_WINDOW (window), area);
  gtk_widget_add_tick_callback (area, tick_cb, NULL, NULL);

  g_signal_connect (window, "close-request", G_CALLBACK (on_close_request), &data);

  gtk_window_present (GTK_WINDOW (window));

  if (interactive && timeout_ms > 0)
    g_timeout_add (timeout_ms, timeout_cb, &data);
  else if (!interactive)
    g_timeout_add (5000, timeout_cb, &data);

  while (!data.done)
    g_main_context_iteration (NULL, TRUE);

  gtk_window_destroy (GTK_WINDOW (window));
  /* Drain pending events so the window actually closes */
  while (g_main_context_pending (NULL))
    g_main_context_iteration (NULL, FALSE);

  g_free (data.output_file);
  g_object_unref (scene);

  return 0;
}

extern void register_basic_tests (void);
extern void register_material_tests (void);
extern void register_light_tests (void);
extern void register_feature_tests (void);
extern void register_pbr_tests (void);
extern void register_advanced_tests (void);

int
test_main (int argc, char *argv[])
{
  gboolean do_list = FALSE;
  gboolean interactive = FALSE;
  const char *output_dir = NULL;
  int timeout_ms = 0;
  g_autoptr(GPtrArray) test_names = g_ptr_array_new ();

  gtk_init ();

  register_basic_tests ();
  register_material_tests ();
  register_light_tests ();
  register_feature_tests ();
  register_pbr_tests ();
  register_advanced_tests ();

  for (int i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "--list") == 0)
        do_list = TRUE;
      else if (strcmp (argv[i], "--interactive") == 0 || strcmp (argv[i], "-i") == 0)
        interactive = TRUE;
      else if (strcmp (argv[i], "--timeout") == 0 || strcmp (argv[i], "-t") == 0)
        {
          if (i + 1 < argc)
            timeout_ms = atoi (argv[++i]);
        }
      else if (strcmp (argv[i], "--output-dir") == 0 || strcmp (argv[i], "-o") == 0)
        {
          if (i + 1 < argc)
            output_dir = argv[++i];
        }
      else if (strcmp (argv[i], "--all") == 0)
        {
          for (int j = 0; j < n_tests; j++)
            g_ptr_array_add (test_names, (gpointer)tests[j].name);
        }
      else
        g_ptr_array_add (test_names, argv[i]);
    }

  if (do_list)
    {
      list_tests ();
      return 0;
    }

  if (test_names->len == 0)
    {
      g_print ("Usage: %s [--list] [-i] [-t MS] [-o DIR] [--all] TEST...\n", argv[0]);
      return 1;
    }

  for (guint i = 0; i < test_names->len; i++)
    {
      const char *name = g_ptr_array_index (test_names, i);
      TestCase *tc = find_test (name);
      if (!tc)
        {
          g_warning ("Unknown test: %s", name);
          list_tests ();
          return 1;
        }

      g_autofree char *output_file = NULL;
      if (output_dir)
        output_file = g_strdup_printf ("%s/%s.png", output_dir, name);

      run_test (tc, output_file, interactive, timeout_ms);
    }

  return 0;
}
