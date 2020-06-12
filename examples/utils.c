#include "utils.h"

const graphene_vec3_t *black (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0, 0, 0);
}

const graphene_vec3_t *white (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 1, 1, 1);
}

const graphene_vec3_t *red (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 1, 0, 0);
}

const graphene_vec3_t *green (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0, 1, 0);
}

const graphene_vec3_t *blue (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0, 0, 1);
}

const graphene_vec3_t *yellow (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 1, 1, 0);
}

const graphene_vec3_t *cyan (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0, 1, 1);
}

const graphene_vec3_t *magenta (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 1, 0, 1);
}

const graphene_vec3_t *very_dark_grey (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0.012, 0.012, 0.012);
}

const graphene_vec3_t *dark_grey (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0.07, 0.07, 0.07);
}

const graphene_vec3_t *medium_grey (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0.4, 0.4, 0.4);
}

const graphene_vec3_t *grey (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0.5, 0.5, 0.5);
}

const graphene_vec3_t *light_grey (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0.87, 0.87, 0.87);
}

const graphene_vec3_t *dark_green (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 0, 0.6, 0);
}

const graphene_vec3_t *orange (void)
{
  static graphene_vec3_t v;
  return graphene_vec3_init (&v, 1, 0.67, 0);
}

GdkPixbuf *
examples_load_pixbuf (const char *file)
{
  GdkPixbuf *pixbuf;
  char *full;

  full = g_build_filename ("/org/gnome/gthree-examples/textures/", file, NULL);

  pixbuf = gdk_pixbuf_new_from_resource (full, NULL);
  if (pixbuf == NULL)
    g_error ("could not load %s", file);

  g_free (full);

  return pixbuf;
}

void
examples_load_cube_pixbufs (const char *dir,
                            GdkPixbuf *pixbufs[6])
{
  char *files[] = {"px.jpg", "nx.jpg",
                   "py.jpg", "ny.jpg",
                   "pz.jpg", "nz.jpg"};
  int i;

  for (i = 0 ; i < 6; i++)
    {
      char *file = g_build_filename (dir, files[i], NULL);
      pixbufs[i] = examples_load_pixbuf (file);
    }
}

GthreeGeometry *
examples_load_geometry (const char *name)
{
  GthreeGeometry *geometry;
  char *file;
  GError *error = NULL;
  GBytes *bytes;

  file = g_build_filename ("/org/gnome/gthree-examples/models/", name, NULL);
  bytes = g_resources_lookup_data (file, G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
  if (bytes == NULL)
    g_error ("can't load model %s: %s", name, error->message);

  geometry = gthree_load_geometry_from_json (g_bytes_get_data (bytes, NULL), &error);
  if (geometry == NULL)
    g_error ("can't parse json: %s", error->message);

  g_bytes_unref (bytes);

  return geometry;
}

GthreeLoader *
examples_load_gltl (const char *name, GError **error)
{
  GthreeLoader *loader;
  char *file;
  g_autoptr(GBytes) bytes = NULL;

  file = g_build_filename ("/org/gnome/gthree-examples/models/", name, NULL);
  bytes = g_resources_lookup_data (file, G_RESOURCE_LOOKUP_FLAGS_NONE, error);
  if (bytes == NULL)
    return FALSE;

  loader = gthree_loader_parse_gltf (bytes, NULL, error);
  if (loader == NULL)
    return NULL;

  return loader;
}

void
examples_quit_cb (GtkWidget *widget,
                  gpointer   data)
{
  gboolean *done = data;

  *done = TRUE;

  g_main_context_wakeup (NULL);
}

GtkWidget *
examples_init (const char *title,
               GtkWidget **box,
               gboolean   *done)
{
  GtkWidget *window, *outer_box, *button;
  g_autofree char *full_title = NULL;

#ifdef USE_GTK4
  gtk_init ();
#else
  gtk_init (NULL, NULL);
#endif

#ifdef USE_GTK4
  window = gtk_window_new ();
#else
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif

  full_title = g_strdup_printf ("%s - %s", title,
#ifdef USE_GTK4
                                "gtk 4"
#else
                                "gtk 3"
#endif
                                );
  gtk_window_set_title (GTK_WINDOW (window), full_title);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
#ifdef USE_GTK3
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
#endif
  g_signal_connect (window, "destroy", G_CALLBACK (examples_quit_cb), done);

  outer_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (outer_box), 6);
#ifdef USE_GTK4
  gtk_widget_set_margin_start (GTK_WIDGET (outer_box), 12);
  gtk_widget_set_margin_end (GTK_WIDGET (outer_box), 12);
  gtk_widget_set_margin_top (GTK_WIDGET (outer_box), 12);
  gtk_widget_set_margin_bottom (GTK_WIDGET (outer_box), 12);
#endif

#ifdef USE_GTK4
  gtk_window_set_child (GTK_WINDOW (window), outer_box);
#else
  gtk_container_add (GTK_CONTAINER (window), outer_box);
#endif

  gtk_widget_show (outer_box);

  *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (*box), 6);
  gtk_box_append (GTK_BOX (outer_box), *box);
  gtk_widget_show (*box);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_box_append (GTK_BOX (outer_box), button);

  g_signal_connect_swapped (button, "clicked",
#ifdef USE_GTK4
                            G_CALLBACK (gtk_window_destroy),
#else
                            G_CALLBACK (gtk_widget_destroy),
#endif
                            window);
  gtk_widget_show (button);

  return window;
}

GtkEventController *
motion_controller_for (GtkWidget *widget)
{
  GtkEventController *motion;

#ifdef USE_GTK4
  motion = gtk_event_controller_motion_new ();
  gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (motion));
#else
  motion = gtk_event_controller_motion_new (widget);
  gtk_widget_add_events (widget, GDK_POINTER_MOTION_MASK);
#endif

  return motion;
}


GtkEventController *
scroll_controller_for (GtkWidget *widget)
{
  GtkEventController *controller;

#ifdef USE_GTK4
  controller = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
  gtk_widget_add_controller (widget, controller);
#else
  controller = gtk_event_controller_scroll_new (widget, GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
  gtk_widget_add_events (widget, GDK_SCROLL_MASK|GDK_SMOOTH_SCROLL_MASK);
#endif

  return controller;
}

GtkEventController *
click_controller_for (GtkWidget *widget)
{
  GtkEventController *controller;

#ifdef USE_GTK4
  controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_widget_add_controller (widget, controller);
#else
  controller = GTK_EVENT_CONTROLLER (gtk_gesture_multi_press_new (widget));
  gtk_widget_add_events (widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
#endif

  return controller;
}

GtkEventController *
drag_controller_for (GtkWidget *widget)
{
  GtkEventController *controller;

#ifdef USE_GTK4
  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  gtk_widget_add_controller (widget, controller);
#else
  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new (widget));
  gtk_widget_add_events (widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
#endif

  return controller;
}

void
rgb_init_from_hsl (graphene_vec3_t *color,
                   double   hue,
                   double   saturation,
                   double   lightness)
{
  gdouble original_hue;
  gdouble m1, m2;
  float red, green, blue;

  if (hue >= 0)
    hue = fmod (hue, 360);
  else
    hue = fmod (hue, 360) + 360;
  saturation = CLAMP (saturation, 0, 1);
  lightness = CLAMP (lightness, 0, 1);

  original_hue = hue;

  if (lightness <= 0.5)
    m2 = lightness * (1 + saturation);
  else
    m2 = lightness + saturation - lightness * saturation;
  m1 = 2 * lightness - m2;

  if (saturation == 0)
    {
      red = lightness;
      green = lightness;
      blue = lightness;
    }
  else
    {
      hue = original_hue + 120;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;

      if (hue < 60)
        red = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        red = m2;
      else if (hue < 240)
        red = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        red = m1;

      hue = original_hue;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;

      if (hue < 60)
        green = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        green = m2;
      else if (hue < 240)
        green = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        green = m1;

      hue = original_hue - 120;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;

      if (hue < 60)
        blue = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        blue = m2;
      else if (hue < 240)
        blue = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        blue = m1;
    }

  graphene_vec3_init (color, red, green, blue);
}

void
quaternion_from_unit_vectors (graphene_quaternion_t *q,
                              const graphene_vec3_t *from,
                              const graphene_vec3_t *to)
{
  // assumes direction vectors from and to are normalized
  float EPS = 0.000001;
  float r = graphene_vec3_dot (from, to) + 1.0;
  graphene_vec3_t cross;

  if (r < EPS)
    {
      r = 0;
      if (fabs (graphene_vec3_get_x (from)) > fabs (graphene_vec3_get_z (from)))
        graphene_quaternion_init (q,
                                  -graphene_vec3_get_y (from),
                                  graphene_vec3_get_x (from),
                                  0,
                                  r);
      else
        graphene_quaternion_init (q,
                                  0,
                                  -graphene_vec3_get_z (from),
                                  graphene_vec3_get_y (from),
                                  r);
    }
  else
    {
      graphene_vec3_cross (from, to, &cross);
      graphene_quaternion_init (q,
                                graphene_vec3_get_x (&cross),
                                graphene_vec3_get_y (&cross),
                                graphene_vec3_get_z (&cross),
                                r);
  }

  graphene_quaternion_normalize (q, q);
}

void
vec3_apply_quaternion (const graphene_vec3_t *v,
                       const graphene_quaternion_t *q,
                       graphene_vec3_t *res)
{
  float x, y, z;
  float qx, qy, qz, qw;
  graphene_vec4_t qv;

  x = graphene_vec3_get_x (v);
  y = graphene_vec3_get_y (v);
  z = graphene_vec3_get_z (v);

  graphene_quaternion_to_vec4 (q, &qv);

  qx = graphene_vec4_get_x (&qv);
  qy = graphene_vec4_get_y (&qv);
  qz = graphene_vec4_get_z (&qv);
  qw = graphene_vec4_get_w (&qv);

  // calculate quat * vector

  float ix = qw * x + qy * z - qz * y;
  float iy = qw * y + qz * x - qx * z;
  float iz = qw * z + qx * y - qy * x;
  float iw = - qx * x - qy * y - qz * z;

  // calculate result * inverse quat

  graphene_vec3_init (res,
                      ix * qw + iw * - qx + iy * - qz - iz * - qy,
                      iy * qw + iw * - qy + iz * - qx - ix * - qz,
                      iz * qw + iw * - qz + ix * - qy - iy * - qx);
}

graphene_vec3_t *
vec3_init_from_spherical (graphene_vec3_t *v,
                          const Spherical *s)
{
  float radius = s->radius;
  float sinPhiRadius = sinf (s->phi) * radius;
  graphene_vec3_init (v,
                      sinPhiRadius * sinf (s->theta),
                      cosf (s->phi) * radius,
                      sinPhiRadius * cosf (s->theta));
  return v;
}

Spherical *
spherical_init (Spherical *s,
                float radius,
                float phi,
                float theta)
{
  s->radius = radius;
  s->phi = phi;
  s->theta = theta;
  return s;
}

void
spherical_set_from_vec3 (Spherical *s,
                         const graphene_vec3_t *v)
{
  float radius = graphene_vec3_length (v);
  float theta = 0;
  float phi = 0;

  if (radius != 0)
    {
      theta = atan2f (graphene_vec3_get_x (v), graphene_vec3_get_z (v));
      phi = acosf (CLAMP (graphene_vec3_get_y (v) / radius, -1.0, 1.0));
    }
  s->radius = radius;
  s->phi = phi;
  s->theta = theta;
}

// restrict phi to be betwee EPS and PI-EPS
void
spherical_make_safe (Spherical *s)
{
  float EPS = 0.000001;

  s->phi = fmaxf (EPS, fminf (G_PI - EPS, s->phi));
}
