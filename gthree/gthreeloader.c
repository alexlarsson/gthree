#include <math.h>

#include "gthreeloader.h"
#include <json-glib/json-glib.h>

typedef struct {
  GthreeGeometry *geometry;
  GList *materials;
} GthreeLoaderPrivate;

G_DEFINE_QUARK (gthree-loader-error-quark, gthree_loader_error)
G_DEFINE_TYPE_WITH_PRIVATE (GthreeLoader, gthree_loader, G_TYPE_OBJECT)

static void
gthree_loader_init (GthreeLoader *loader)
{
}

static void
gthree_loader_finalize (GObject *obj)
{
  GthreeLoader *loader = GTHREE_LOADER (obj);
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);

  g_clear_object (&priv->geometry);
  g_list_free_full (priv->materials, g_object_unref);

  G_OBJECT_CLASS (gthree_loader_parent_class)->finalize (obj);
}

static void
gthree_loader_class_init (GthreeLoaderClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_loader_finalize;
}


GthreeLoader *
gthree_loader_new_from_json (const char *data, GFile *texture_path, GError **error)
{
  JsonParser *parser;
  GthreeLoader *loader = NULL;

  parser = json_parser_new ();

  if (!json_parser_load_from_data (parser, data, -1, error))
    {
      g_object_unref (parser);
      return NULL;
    }

  return loader;
}

GthreeGeometry *
gthree_load_geometry_from_json (const char *data, GError **error)
{
  g_autoptr(JsonParser) parser = json_parser_new ();
  JsonNode *root;
  JsonObject *root_obj, *metadata;

  if (!json_parser_load_from_data (parser, data, -1, error))
    return NULL;

  root = json_parser_get_root (parser);
  if (!JSON_NODE_HOLDS_OBJECT(root))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "no root object");
      return NULL;
    }

  root_obj = json_node_get_object (root);

  if (!json_object_has_member (root_obj, "metadata"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "not a thee.js json file (no metadata)");
      return NULL;
    }
  metadata = json_object_get_object_member (root_obj, "metadata");
  if (!json_object_has_member (metadata, "type") ||
      g_strcmp0 (json_object_get_string_member (metadata, "type"), "BufferGeometry"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "not a BufferGeometry file");
      return NULL;
    }

  return gthree_geometry_parse_json (root_obj);
}
