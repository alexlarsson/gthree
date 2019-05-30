#include <math.h>

#include "gthreeloader.h"
#include "gthreeattribute.h"
#include "gthreemeshstandardmaterial.h"
#include "gthreemeshbasicmaterial.h"
#include "gthreemesh.h"
#include "gthreescene.h"
#include "gthreegroup.h"
#include "gthreeprivate.h"
#include <json-glib/json-glib.h>

typedef struct {
  GPtrArray *buffers;
  GPtrArray *buffer_views;
  GPtrArray *images;
  GPtrArray *accessors;
  GPtrArray *nodes;
  GPtrArray *meshes;
  GPtrArray *scenes;
  int scene;
} GthreeLoaderPrivate;

G_DEFINE_QUARK (gthree-loader-error-quark, gthree_loader_error)
G_DEFINE_TYPE_WITH_PRIVATE (GthreeLoader, gthree_loader, G_TYPE_OBJECT)

typedef struct {
  GBytes *bytes;
  guint buffer;
  guint byte_offset;
  guint byte_length;
  guint byte_stride;
  guint target;
  // name
  // extensions
  // extras
} BufferView;

typedef struct {
  GPtrArray *primitives;
} Mesh;

static void
buffer_view_free (BufferView *view)
{
  if (view->bytes)
    g_bytes_unref (view->bytes);
  g_free (view);
}

static void
mesh_free (Mesh *mesh)
{
  if (mesh->primitives)
    g_ptr_array_unref (mesh->primitives);
  g_free (mesh);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (BufferView, buffer_view_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Mesh, mesh_free);

static BufferView *
buffer_view_new (GthreeLoader *loader, JsonObject *json, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  g_autoptr(BufferView) view = g_new0 (BufferView, 1);

  if (!json_object_has_member (json, "buffer"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "BufferView lacks buffer");
      return NULL;
    }
  view->buffer = json_object_get_int_member (json, "buffer");
  if (view->buffer >= priv->buffers->len)
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "BufferView refers to non-existing buffer");
      return NULL;
    }
  view->byte_offset = 0;
  if (json_object_has_member (json, "byteOffset"))
    view->byte_offset = json_object_get_int_member (json, "byteOffset");

  if (!json_object_has_member (json, "byteLength"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "BufferView lacks byteLength");
      return NULL;
    }
  view->byte_length = json_object_get_int_member (json, "byteLength");

  view->byte_stride = 0;
  if (json_object_has_member (json, "bufferStride"))
    view->byte_stride = json_object_get_int_member (json, "byteStride");

  view->target = 0;
  if (json_object_has_member (json, "target"))
    view->byte_stride = json_object_get_int_member (json, "target");

  view->bytes = g_bytes_new_from_bytes (g_ptr_array_index (priv->buffers, view->buffer),
                                        view->byte_offset, view->byte_length);

  return g_steal_pointer (&view);
}

static void
gthree_loader_init (GthreeLoader *loader)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);

  priv->buffers = g_ptr_array_new_with_free_func ((GDestroyNotify)g_bytes_unref);
  priv->buffer_views = g_ptr_array_new_with_free_func ((GDestroyNotify)buffer_view_free);
  priv->images = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->accessors = g_ptr_array_new_with_free_func ((GDestroyNotify)gthree_attribute_array_unref);
  priv->nodes = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->meshes = g_ptr_array_new_with_free_func ((GDestroyNotify)mesh_free);
  priv->scenes = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
}

static void
gthree_loader_finalize (GObject *obj)
{
  GthreeLoader *loader = GTHREE_LOADER (obj);
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);

  g_ptr_array_unref (priv->buffers);
  g_ptr_array_unref (priv->buffer_views);
  g_ptr_array_unref (priv->images);
  g_ptr_array_unref (priv->accessors);
  g_ptr_array_unref (priv->nodes);
  g_ptr_array_unref (priv->meshes);
  g_ptr_array_unref (priv->scenes);

  G_OBJECT_CLASS (gthree_loader_parent_class)->finalize (obj);
}

static void
gthree_loader_class_init (GthreeLoaderClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_loader_finalize;
}

static gboolean
decode_is_glb_header (GBytes *data, guint32 *version, guint32 *length)
{
  gsize data_len;
  const guint8 *data_ptr = g_bytes_get_data (data, &data_len);
  const guint32 *data_ptr32 = (guint32 *) data_ptr;

  if (data_len < 12)
    return FALSE;

  if (data_ptr[0] != 'g' ||
      data_ptr[1] != 'l' ||
      data_ptr[2] != 'T' ||
      data_ptr[3] != 'F')
    return FALSE;

  *version = GUINT32_FROM_LE (data_ptr32[1]);
  *length = GUINT32_FROM_LE (data_ptr32[2]);

  return TRUE;
}

static gboolean
get_glb_chunks (GBytes *data, GBytes **json_out, GBytes **bin_out, GError **error)
{
  gsize data_len;
  const guint8 *data_ptr = g_bytes_get_data (data, &data_len);
  const guint32 *data_ptr32;
  guint32 chunk_length, chunk_type, json_chunk_offset, bin_chunk_offset;
  g_autoptr(GBytes) json = NULL;
  g_autoptr(GBytes) bin = NULL;

  if (data_len < 12 + 8)
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Short file read finding json chunk");
      return FALSE;
    }

  json_chunk_offset = 12;
  data_ptr32 = (guint32 *)(data_ptr + json_chunk_offset);
  chunk_length = GUINT32_FROM_LE (data_ptr32[0]);
  chunk_type = GUINT32_FROM_LE (data_ptr32[1]);

  if (chunk_type != 0x4E4F534A)
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "First GLB chunk is not json");
      return FALSE;
    }

  if (data_len < json_chunk_offset + 8 + chunk_length)
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Short file read in json chunk");
      return FALSE;
    }

  json = g_bytes_new_from_bytes (data, json_chunk_offset + 8, chunk_length);

  bin_chunk_offset = json_chunk_offset + 8 + chunk_length;

  if (data_len > bin_chunk_offset)
    {
      if (data_len < bin_chunk_offset + 8)
        {
          g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Short file read finding binary chunk");
          return FALSE;
        }

      data_ptr32 = (guint32 *)(data_ptr + bin_chunk_offset);
      chunk_length = GUINT32_FROM_LE (data_ptr32[0]);
      chunk_type = GUINT32_FROM_LE (data_ptr32[1]);

      if (data_len < bin_chunk_offset + 8 + chunk_length)
        {
          g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Short file read in bin chunk");
          return FALSE;
        }

      bin = g_bytes_new_from_bytes (data, bin_chunk_offset + 8, chunk_length);
    }

   *json_out = g_steal_pointer (&json);
   *bin_out = g_steal_pointer (&bin);
   return TRUE;
}

static gboolean
parse_buffers (GthreeLoader *loader, JsonObject *root, GBytes *bin_chunk, GFile *base_path, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *buffers_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "buffers"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No buffers specified");
      return FALSE;
    }
  buffers_j = json_object_get_array_member (root, "buffers");
  len = json_array_get_length (buffers_j);

  for (i = 0; i < len; i++)
    {
      JsonObject *buffer_j = json_array_get_object_element (buffers_j, i);
      gint64 byte_length = -1;
      const char *uri = NULL;
      g_autoptr(GBytes) bytes = NULL;

      if (json_object_has_member (buffer_j, "byteLength"))
        byte_length = json_object_get_int_member (buffer_j, "byteLength");

      if (json_object_has_member (buffer_j, "uri"))
        uri = json_object_get_string_member (buffer_j, "uri");

      if (uri == NULL)
        {
          if (i == 0 && bin_chunk != NULL)
            {
              if (byte_length == -1)
                {
                  g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Binary chunk size missing");
                  return FALSE;
                }
              if (g_bytes_get_size (bin_chunk) < byte_length)
                {
                  g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Binary chunk to short");
                  return FALSE;
                }
              bytes = g_bytes_new_from_bytes (bin_chunk, 0, byte_length);
            }
          else
            {
              g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Missing url from buffer %d", i);
              return FALSE;
            }
        }
      else
        {
          g_autoptr(GFile) file = NULL;
          g_autoptr(GBytes) file_bytes = NULL;

          if (base_path)
            file = g_file_resolve_relative_path (base_path, uri);
          else
            file = g_file_new_for_commandline_arg (uri);

          file_bytes = g_file_load_bytes (file, NULL, NULL, error);
          if (file_bytes == NULL)
            return FALSE;

          if (byte_length > 0)
            bytes = g_bytes_new_from_bytes (file_bytes, 0, byte_length);
          else
            bytes = g_bytes_ref (file_bytes);
        }

      g_ptr_array_add (priv->buffers, g_steal_pointer (&bytes));
    }

  return TRUE;
}

static gboolean
parse_buffer_views (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *buffer_views_j;
  guint len;
  int i;

  if (!json_object_has_member (root, "bufferViews"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No bufferViews specified");
      return FALSE;
    }
  buffer_views_j = json_object_get_array_member (root, "bufferViews");
  len = json_array_get_length (buffer_views_j);

  for (i = 0; i < len; i++)
    {
      JsonObject *buffer_view_j = json_array_get_object_element (buffer_views_j, i);
      g_autoptr(BufferView) buffer_view = NULL;

      buffer_view = buffer_view_new (loader, buffer_view_j, error);
      if (buffer_view == NULL)
        return FALSE;

      g_ptr_array_add (priv->buffer_views, g_steal_pointer (&buffer_view));
    }

  return TRUE;
}

static gboolean
parse_accessors (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *accessors_j;
  guint len;
  int i;

  if (!json_object_has_member (root, "accessors"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No accessors specified");
      return FALSE;
    }
  accessors_j = json_object_get_array_member (root, "accessors");
  len = json_array_get_length (accessors_j);

  for (i = 0; i < len; i++)
    {
      JsonObject *accessor_j = json_array_get_object_element (accessors_j, i);
      g_autoptr(GthreeAttributeArray) array = NULL;
      gint64 buffer_view = -1;
      gint64 byte_offset = 0;
      gint64 component_type, item_size, count;
      GthreeAttributeType attribute_type;
      const char *type = NULL;
      gboolean normalized = FALSE;

      if (json_object_has_member (accessor_j, "bufferView"))
        buffer_view =  json_object_get_int_member (accessor_j, "bufferView");
      if (json_object_has_member (accessor_j, "byteOffset"))
        byte_offset =  json_object_get_int_member (accessor_j, "byteOffset");
      component_type =  json_object_get_int_member (accessor_j, "componentType");
      if (json_object_has_member (accessor_j, "normalized"))
        normalized =  json_object_get_boolean_member (accessor_j, "normalized");
      count = json_object_get_int_member (accessor_j, "count");
      type = json_object_get_string_member (accessor_j, "type");

      if (json_object_has_member (accessor_j, "sparse"))
        g_warning ("Ignoring sparse accessor");

      if (normalized) // TODO: Research this, should we move the property to attribute array, or store elsewhere in the loader?
        g_warning ("Ignoring normalized tag, because we can't store it on the attribute array");

      switch (component_type)
        {
        case 5120: // BYTE
          attribute_type = GTHREE_ATTRIBUTE_TYPE_INT8;
          break;
        case 5121: // UNSIGNED_BYTE
          attribute_type = GTHREE_ATTRIBUTE_TYPE_UINT8;
          break;
        case 5122: // SHORT
          attribute_type = GTHREE_ATTRIBUTE_TYPE_INT16;
          break;
        case 5123: // UNSIGNED_SHORT
          attribute_type = GTHREE_ATTRIBUTE_TYPE_UINT16;
          break;
        case 5125: // UNSIGNED_INT
          attribute_type = GTHREE_ATTRIBUTE_TYPE_UINT32;
          break;
        case 5126: // FLOAT
          attribute_type = GTHREE_ATTRIBUTE_TYPE_FLOAT;
          break;
        default:
          {
            g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Unsupported accessor component type %d", (int)component_type);
            return FALSE;
          }
        }

      if (strcmp (type, "SCALAR") == 0)
        item_size = 1;
      else if (strcmp (type, "VEC2") == 0)
        item_size = 2;
      else if (strcmp (type, "VEC3") == 0)
        item_size = 3;
      else if (strcmp (type, "VEC4") == 0)
        item_size = 4;
      else if (strcmp (type, "MAT2") == 0)
        item_size = 4;
      else if (strcmp (type, "MAT3") == 0)
        item_size = 9;
      else if (strcmp (type, "MAT4") == 0)
        item_size = 16;
      else
        {
          g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Unsupported type %s", type);
          return FALSE;
        }

      // TODO: The above has multiple options for same count, so we throw away info. does this matter?

      array = gthree_attribute_array_new (attribute_type, item_size * count, item_size);

      if (buffer_view >= 0)
        {
          BufferView *view;
          if (buffer_view >= priv->buffer_views->len)
            {
              g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No such buffer view %d", (int)buffer_view);
              return FALSE;
            }
          view = g_ptr_array_index (priv->buffer_views, buffer_view);

          memcpy (gthree_attribute_array_peek_uint8 (array),
                  g_bytes_get_data (view->bytes, NULL) + byte_offset,
                  item_size * count * gthree_attribute_type_length (attribute_type));
        }

      g_ptr_array_add (priv->accessors, g_steal_pointer (&array));
    }

  return TRUE;
}

static gboolean
parse_images (GthreeLoader *loader, JsonObject *root, GFile *base_path, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *images_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "images"))
    return TRUE;

  images_j = json_object_get_array_member (root, "images");
  len = json_array_get_length (images_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *image_j = json_array_get_object_element (images_j, i);
      g_autoptr(GdkPixbuf) pixbuf = NULL;
      g_autoptr(GBytes) bytes = NULL;
      g_autoptr(GInputStream) in = NULL;

      if (json_object_has_member (image_j, "uri"))
        {
          g_autoptr(GFile) file = NULL;
          const char *uri;

          uri = json_object_get_string_member (image_j, "uri");
          if (base_path)
            file = g_file_resolve_relative_path (base_path, uri);
          else
            file = g_file_new_for_commandline_arg (uri);

          bytes = g_file_load_bytes (file, NULL, NULL, error);
          if (bytes == NULL)
            return FALSE;
        }
      else if (json_object_has_member (image_j, "bufferView"))
        {
          BufferView *view;
          gint64 v = json_object_get_int_member (image_j, "bufferView");

          if (v >= priv->buffer_views->len)
            {
              g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No buffer view %d in image %d", (int)v, (int)i);
              return FALSE;
            }
          view = g_ptr_array_index (priv->buffer_views, v);
          bytes = g_bytes_ref (view->bytes);
        }
      else
        {
          g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Missing url or bufferView from image buffer %d", i);
          return FALSE;
        }

      in = g_memory_input_stream_new_from_bytes (bytes);
      pixbuf = gdk_pixbuf_new_from_stream (in, NULL, error);
      if (pixbuf == NULL)
        return FALSE;

      g_ptr_array_add (priv->images, g_steal_pointer (&pixbuf));
    }

  return TRUE;
}

static void
parse_matrix (JsonArray *matrix_j, graphene_matrix_t *m)
{
  float floats[16];
  int i;

  // Graphene and glTL differs here:
  // glTL:
  //   A floating-point 4x4 transformation matrix stored in column-major order
  // graphene:
  //   The matrix is treated as row-major, i.e. the x, y, z, and w vectors
  //   are rows, and elements of each vector are a column:
  // So we have to transpos here

  for (i = 0; i < 16; i++)
    floats[i] = (float)json_array_get_double_element  (matrix_j, i);

  graphene_matrix_init_from_float (m, floats);
  graphene_matrix_transpose (m, m);
}

static void
parse_point3d (JsonArray *point_j, graphene_point3d_t *p)
{
  p->x = json_array_get_double_element  (point_j, 0);
  p->y = json_array_get_double_element  (point_j, 1);
  p->z = json_array_get_double_element  (point_j, 2);
}

static void
parse_quaternion (JsonArray *quat_j, graphene_quaternion_t *q)
{
  graphene_quaternion_init (q,
                            json_array_get_double_element  (quat_j, 0),
                            json_array_get_double_element  (quat_j, 1),
                            json_array_get_double_element  (quat_j, 2),
                            json_array_get_double_element  (quat_j, 3));
}

static const char *
gltl_attribute_name_to_gthree (const char *attr_name)
{
  if (strcmp (attr_name, "POSITION") == 0)
      return "position";
  else if (strcmp (attr_name, "NORMAL") == 0)
      return "normal";
  else if (strcmp (attr_name, "TANGENT") == 0)
      return "tangent";
  else if (strcmp (attr_name, "TEXCOORD_0") == 0)
      return "uv";
  else if (strcmp (attr_name, "TEXCOORD_1") == 0)
      return "uv2";
  else if (strcmp (attr_name, "COLOR_0") == 0)
      return "color";
  else if (strcmp (attr_name, "WEIGHTS_0") == 0)
      return "skinWeight";
  else if (strcmp (attr_name, "JOINTS_0") == 0)
      return "skinIndex";
  else
    return attr_name;
}

static gboolean
parse_meshes (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *meshes_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "meshes"))
    return TRUE;

  meshes_j = json_object_get_array_member (root, "meshes");
  len = json_array_get_length (meshes_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *mesh_j = json_array_get_object_element (meshes_j, i);
      JsonArray *primitives;
      int primitives_len, j;
      g_autoptr(Mesh) mesh = g_new0 (Mesh, 1);
      mesh->primitives = g_ptr_array_new_with_free_func (g_object_unref);

      primitives = json_object_get_array_member (mesh_j, "primitives");
      primitives_len = json_array_get_length (primitives);
      for (j = 0; j < primitives_len; j++)
        {
          JsonObject *primitive = json_array_get_object_element (primitives, j);
          JsonObject *attributes = json_object_get_object_member (primitive, "attributes");
          g_autoptr(GthreeGeometry) geometry = gthree_geometry_new ();
          int mode = 4;
          int material = -1;
          g_autoptr(GList) members = json_object_get_members (attributes);
          GList *l;

          for (l = members; l != NULL; l = l->next)
            {
              const char *attr_name = l->data;
              gint64 accessor_index = json_object_get_int_member (attributes, attr_name);
              const char *gthree_name = gltl_attribute_name_to_gthree (attr_name);
              GthreeAttributeArray *array = g_ptr_array_index (priv->accessors, accessor_index);
              g_autoptr(GthreeAttribute) attribute = NULL;
              gboolean normalized = FALSE;

              // TODO: Get normalized from the attribute array (was in accessors definition)
              // TODO: How does this work with interleaved arrays? seems an attribute can't define an offset?
              attribute = gthree_attribute_new_with_array (gthree_name, array, normalized);
              gthree_geometry_add_attribute (geometry, attribute);
            }

          if (json_object_has_member (primitive, "index"))
            {
              int index_index = json_object_get_int_member (primitive, "index");
              g_autoptr(GthreeAttribute) attribute = NULL;
              GthreeAttributeArray *array = g_ptr_array_index (priv->accessors, index_index);

              attribute = gthree_attribute_new_with_array ("index", array, FALSE);
              gthree_geometry_set_index (geometry, attribute);
            }

          if (json_object_has_member (primitive, "mode"))
            mode = json_object_get_int_member (primitive, "mode");

          if (json_object_has_member (primitive, "material"))
            material = json_object_get_int_member (primitive, "material");

          if (mode != 4)
            g_warning ("mode is not TRIANGLES, unsupported");

          // TODO: Save material & mode here for later
          g_ptr_array_add (mesh->primitives, g_steal_pointer (&geometry));
        }

      g_ptr_array_add (priv->meshes, g_steal_pointer (&mesh));
    }

  return TRUE;
}


static gboolean
parse_nodes (GthreeLoader *loader, JsonObject *root, GFile *base_path, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *nodes_j = NULL;
  guint len;
  int i;
  graphene_point3d_t scale = { 1.f, 1.f, 1.f }, translate = { 0, 0, 0};
  graphene_quaternion_t rotate;

  if (!json_object_has_member (root, "nodes"))
    return TRUE;

  nodes_j = json_object_get_array_member (root, "nodes");
  len = json_array_get_length (nodes_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *node_j = json_array_get_object_element (nodes_j, i);
      g_autoptr(GthreeObject) node = NULL;

      // TODO: handle cameras, skin, weights (morph targets)

      node = (GthreeObject *)g_object_ref_sink (gthree_group_new ());

      if (json_object_has_member (node_j, "mesh"))
        {
          gint64 mesh_id = json_object_get_int_member (node_j, "mesh");
          Mesh *mesh = g_ptr_array_index (priv->meshes, mesh_id);
          int j;

          for (j = 0; j < mesh->primitives->len; j++)
            {
              GthreeGeometry *geometry = g_ptr_array_index (mesh->primitives, j);
              //g_autoptr(GthreeMaterial) material = GTHREE_MATERIAL (gthree_mesh_standard_material_new ())
              g_autoptr(GthreeMaterial) material = GTHREE_MATERIAL (gthree_mesh_basic_material_new ());
              GdkRGBA magenta= {1, 0, 1, 1};

              gthree_mesh_basic_material_set_color (GTHREE_BASIC_MATERIAL (material), &magenta);

              // TODO: Find real material
              GthreeMesh *mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));

              gthree_object_add_child (node, GTHREE_OBJECT (mesh));
            }

        }

      if (json_object_has_member (node_j, "camera"))
        {
          // TODO
        }

      if (json_object_has_member (node_j, "skin"))
        {
          // TODO
        }

      graphene_quaternion_init_identity (&rotate);

      if (json_object_has_member (node_j, "matrix"))
        {
          JsonArray *matrix_j = json_object_get_array_member (node_j, "matrix");
          graphene_matrix_t m;

          parse_matrix (matrix_j, &m);

          g_warning ("Not supporting matrix transformations atm");
        }
      else
        {
          if (json_object_has_member (node_j, "translation"))
            {
              JsonArray *translation_j = json_object_get_array_member (node_j, "translation");
              parse_point3d (translation_j, &translate);
            }
          if (json_object_has_member (node_j, "scale"))
            {
              JsonArray *scale_j = json_object_get_array_member (node_j, "scale");
              parse_point3d (scale_j, &scale);
            }
          if (json_object_has_member (node_j, "rotation"))
            {
              JsonArray *rotation_j = json_object_get_array_member (node_j, "rotation");
              parse_quaternion (rotation_j, &rotate);
            }
        }

      gthree_object_set_position (node, &translate);
      gthree_object_set_quaternion (node, &rotate);
      gthree_object_set_scale (node, &scale);

      g_ptr_array_add (priv->nodes, g_steal_pointer (&node));
    }

  nodes_j = json_object_get_array_member (root, "nodes");
  len = json_array_get_length (nodes_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *node_j = json_array_get_object_element (nodes_j, i);
      GthreeObject *parent = g_ptr_array_index (priv->nodes, i);

      if (json_object_has_member (node_j, "children"))
        {
          JsonArray *children = json_object_get_array_member (node_j, "children");
          int children_len = json_array_get_length (children);
          int j;
          for (j = 0; j < children_len; j++)
            {
              gint64 index = json_array_get_int_element (children, j);
              GthreeObject *child = g_ptr_array_index (priv->nodes, index);

              gthree_object_add_child (parent, child);
            }
        }
    }

  return TRUE;
}

static gboolean
parse_scenes (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *scenes_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "scenes"))
    return TRUE;

  scenes_j = json_object_get_array_member (root, "scenes");
  len = json_array_get_length (scenes_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *scene_j = json_array_get_object_element (scenes_j, i);
      g_autoptr(GthreeScene) scene = gthree_scene_new ();

      if (json_object_has_member (scene_j, "nodes"))
        {
          JsonArray *children = json_object_get_array_member (scene_j, "nodes");
          int children_len = json_array_get_length (children);
          int j;
          for (j = 0; j < children_len; j++)
            {
              gint64 index = json_array_get_int_element (children, j);
              GthreeObject *child = g_ptr_array_index (priv->nodes, index);

              gthree_object_add_child (GTHREE_OBJECT (scene), child);
            }
        }

      g_ptr_array_add (priv->scenes, g_steal_pointer (&scene));
    }

  return TRUE;
}



GthreeLoader *
gthree_loader_parse_gltf (GBytes *data, GFile *base_path, GError **error)
{
  g_autoptr(JsonParser) parser = NULL;
  g_autoptr(JsonNode) root_node = NULL;
  JsonObject *root;
  g_autoptr(GthreeLoader) loader = NULL;
  guint32 glb_version;
  guint32 json_length;
  g_autoptr(GBytes) json = NULL;
  g_autoptr(GBytes) bin = NULL;

  if (decode_is_glb_header (data, &glb_version, &json_length))
    {
      if (glb_version != 2)
        {
          g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Unsupported glTL GLB version %d", glb_version);
          return NULL;
        }

      if (!get_glb_chunks (data, &json, &bin, error))
        return NULL;

    }
  else /* Assume json format */
    {
      json = g_bytes_ref (data);
    }

  parser = json_parser_new ();

  if (!json_parser_load_from_data (parser, g_bytes_get_data (json, NULL), g_bytes_get_size (json), error))
    return NULL;

  root_node = json_parser_steal_root (parser);
  root = json_node_get_object (root_node);

  loader = g_object_new (gthree_loader_get_type (), NULL);

  if (!parse_buffers (loader, root, bin, base_path, error))
    return NULL;

  if (!parse_buffer_views (loader, root, error))
    return NULL;

  if (!parse_accessors (loader, root, error))
    return NULL;

  if (!parse_images (loader, root, base_path, error))
    return NULL;

  if (!parse_meshes (loader, root, error))
    return NULL;

  if (!parse_nodes (loader, root, base_path, error))
    return NULL;

  if (!parse_scenes (loader, root, error))
    return NULL;

  return g_steal_pointer (&loader);
}

int
gthree_loader_get_n_scenes (GthreeLoader *loader)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);

  return priv->scenes->len;
}

GthreeScene *
gthree_loader_get_scene (GthreeLoader *loader,
                         int index)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  GthreeScene *scene = g_ptr_array_index (priv->scenes, index);

  return scene;
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


