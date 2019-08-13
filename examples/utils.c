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
