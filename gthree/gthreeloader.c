#include <math.h>

#include "gthreeloader.h"
#include "gthreeattribute.h"
#include "gthreemeshstandardmaterial.h"
#include "gthreeperspectivecamera.h"
#include "gthreemeshbasicmaterial.h"
#include "gthreemesh.h"
#include "gthreeskinnedmesh.h"
#include "gthreescene.h"
#include "gthreebone.h"
#include "gthreegroup.h"
#include "gthreeenums.h"
#include "gthreeprivate.h"
#include <json-glib/json-glib.h>

/* TODO:
 * object.set_matrix need to decompose position, etc. or we can't expect e.g. get_position to work.
 * Orthographic cameras
 * Handle primitive draw_mode != triangles
 * Try grouping primitives into geometry groups if possible
 * Handle skin weights
 * Handle morph targets
 * Handle animations
 * Handle sparse accessors
 * Handle line materials
 * handle data: uris
 */

typedef struct {
  gboolean is_bone;
} NodeInfo;

typedef struct {
  NodeInfo *node_infos;
  GPtrArray *buffers;
  GPtrArray *buffer_views;
  GPtrArray *images;
  GPtrArray *accessors;
  GPtrArray *nodes;
  GPtrArray *meshes;
  GPtrArray *scenes;
  GPtrArray *samplers;
  GPtrArray *textures;
  GPtrArray *materials;
  GPtrArray *cameras;
  GPtrArray *skins;

  GHashTable *final_materials_hash;
  GPtrArray *final_materials;

  GthreeMaterial *default_material;
  int scene;
} GthreeLoaderPrivate;

G_DEFINE_QUARK (gthree-loader-error-quark, gthree_loader_error)
G_DEFINE_TYPE_WITH_PRIVATE (GthreeLoader, gthree_loader, G_TYPE_OBJECT)

typedef struct {
  guint buffer;
  guint byte_offset;
  GBytes *bytes;  /* combines buffer + byte offset/length +  the actual buffer GBytes */
  guint byte_length;
  guint byte_stride;
  guint target;
  // name
  // extensions
  // extras

  /* Set later for reuse when we know the attribute type to user */
  GthreeAttributeArray *array;

} BufferView;

typedef struct {
  GthreeAttributeArray *array;
  gboolean normalized;
  int item_size;    // in array->type size units
  int item_offset;  // in array->type size units
  int count;
} Accessor;

typedef struct {
  gboolean perspective;

  // perspective
  float aspect_ratio;
  float yfov;

  // ortog
  float xmag;
  float ymag;

  //shared
  float zfar;
  float znear;

} Camera;

typedef struct {
  int inverse_bind_matrices;
  int skeleton;
  GArray *joints;
} Skin;


typedef struct {
  GthreeMaterial *orig_material;
  gboolean use_vertex_tangents;
  gboolean use_vertex_colors;
  gboolean use_skinning;
  gboolean use_morph_targets;
  gboolean use_morph_normals;
} MaterialCacheKey;

static void
material_cache_key_free (MaterialCacheKey *cache_key)
{
  g_object_unref (cache_key->orig_material);
  g_free (cache_key);
}

static gboolean
material_cache_key_equal (MaterialCacheKey *a,
                          MaterialCacheKey *b)
{
  return
    a->orig_material == b->orig_material &&
    a->use_vertex_tangents == b->use_vertex_tangents &&
    a->use_vertex_colors == b->use_vertex_colors &&
    a->use_skinning == b->use_skinning &&
    a->use_morph_targets == b->use_morph_targets &&
    a->use_morph_normals == b->use_morph_normals;
}

static MaterialCacheKey *
material_cache_key_clone (MaterialCacheKey *cache_key)
{
  MaterialCacheKey *clone = g_new0 (MaterialCacheKey, 1);
  *clone = *cache_key;
  g_object_ref (clone->orig_material);

  return clone;
}

static guint
material_cache_key_hash (MaterialCacheKey *key)
{
  return
    g_direct_hash (key->orig_material) |
    key->use_vertex_tangents ? 0x0001 : 0 |
    key->use_vertex_colors   ? 0x0002 : 0 |
    key->use_skinning        ? 0x0004 : 0 |
    key->use_morph_targets   ? 0x0008 : 0 |
    key->use_morph_normals   ? 0x0010 : 0;
}

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;
  int mode;
} Primitive;

typedef struct {
  GPtrArray *primitives;
} Mesh;

typedef struct {
  GthreeFilter mag_filter;
  GthreeFilter min_filter;
  GthreeWrapping wrap_s;
  GthreeWrapping wrap_t;
} Sampler;

static void
accessor_free (Accessor *accessor)
{
  if (accessor->array)
    gthree_attribute_array_unref (accessor->array);
  g_free (accessor);
}

static void
sampler_free (Sampler *sampler)
{
  g_free (sampler);
}

static void
camera_free (Camera *camera)
{
  g_free (camera);
}

static void
skin_free (Skin *skin)
{
  if (skin->joints)
    g_array_unref (skin->joints);
  g_free (skin);
}

static void
buffer_view_free (BufferView *view)
{
  if (view->array)
    gthree_attribute_array_unref (view->array);
  if (view->bytes)
    g_bytes_unref (view->bytes);
  g_free (view);
}

static void
primitive_free (Primitive *primitive)
{
  g_clear_object (&primitive->geometry);
  g_clear_object (&primitive->material);
  g_free (primitive);
}

static void
mesh_free (Mesh *mesh)
{
  if (mesh->primitives)
    g_ptr_array_unref (mesh->primitives);
  g_free (mesh);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (BufferView, buffer_view_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Primitive, primitive_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Mesh, mesh_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Accessor, accessor_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Sampler, sampler_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Camera, camera_free);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (Skin, skin_free);

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
  if (json_object_has_member (json, "byteStride"))
    view->byte_stride = json_object_get_int_member (json, "byteStride");

  view->target = 0;
  if (json_object_has_member (json, "target"))
    view->target = json_object_get_int_member (json, "target");

  view->bytes = g_bytes_new_from_bytes (g_ptr_array_index (priv->buffers, view->buffer),
                                        view->byte_offset, view->byte_length);

  return g_steal_pointer (&view);
}

static void
gthree_loader_init (GthreeLoader *loader)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  GdkRGBA magenta= {1, 0, 1, 1};

  priv->buffers = g_ptr_array_new_with_free_func ((GDestroyNotify)g_bytes_unref);
  priv->buffer_views = g_ptr_array_new_with_free_func ((GDestroyNotify)buffer_view_free);
  priv->images = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->accessors = g_ptr_array_new_with_free_func ((GDestroyNotify)accessor_free);
  priv->nodes = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->meshes = g_ptr_array_new_with_free_func ((GDestroyNotify)mesh_free);
  priv->scenes = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->samplers = g_ptr_array_new_with_free_func ((GDestroyNotify)sampler_free);
  priv->textures = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->materials = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->cameras = g_ptr_array_new_with_free_func ((GDestroyNotify)camera_free);
  priv->skins = g_ptr_array_new_with_free_func ((GDestroyNotify)skin_free);

  priv->final_materials = g_ptr_array_new_with_free_func ((GDestroyNotify)g_object_unref);
  priv->final_materials_hash = g_hash_table_new_full ((GHashFunc)material_cache_key_hash,
                                                      (GEqualFunc)material_cache_key_equal,
                                                      (GDestroyNotify)material_cache_key_free,
                                                      NULL);

  priv->default_material = GTHREE_MATERIAL (gthree_mesh_basic_material_new ());
  gthree_mesh_basic_material_set_color (GTHREE_BASIC_MATERIAL (priv->default_material), &magenta);
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
  g_ptr_array_unref (priv->samplers);
  g_ptr_array_unref (priv->textures);
  g_ptr_array_unref (priv->materials);
  g_ptr_array_unref (priv->cameras);
  g_ptr_array_unref (priv->skins);

  if (priv->node_infos)
    g_free (priv->node_infos);

  g_ptr_array_unref (priv->final_materials);
  g_hash_table_unref (priv->final_materials_hash);

  g_object_unref (priv->default_material);

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


static void
parse_matrix (JsonArray *matrix_j, graphene_matrix_t *m)
{
  float floats[16];
  int i;

  for (i = 0; i < 16; i++)
    floats[i] = (float)json_array_get_double_element  (matrix_j, i);

  graphene_matrix_init_from_float (m, floats);
}

static void
parse_point3d (JsonArray *point_j, graphene_point3d_t *p)
{
  p->x = json_array_get_double_element  (point_j, 0);
  p->y = json_array_get_double_element  (point_j, 1);
  p->z = json_array_get_double_element  (point_j, 2);
}

static void
parse_vec2 (JsonArray *vec_j, graphene_vec2_t *v)
{
  graphene_vec2_init (v,
                      json_array_get_double_element  (vec_j, 0),
                      json_array_get_double_element  (vec_j, 1));
}

static void
parse_color (JsonArray *color_j, GdkRGBA *c)
{
  c->red = json_array_get_double_element  (color_j, 0);
  c->green = json_array_get_double_element (color_j, 1);
  c->blue = json_array_get_double_element (color_j, 2);
  if (json_array_get_length (color_j) > 3)
    c->alpha = json_array_get_double_element (color_j, 3);
  else
    c->alpha = 1.0;
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


static gboolean
supports_extension (const char *extension)
{
  return FALSE;
}

static void
init_node_info (GthreeLoader *loader, JsonObject *root)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *nodes_j = NULL;
  guint len;

  if (!json_object_has_member (root, "nodes"))
    return;

  nodes_j = json_object_get_array_member (root, "nodes");
  len = json_array_get_length (nodes_j);

  priv->node_infos = g_new0 (NodeInfo, len);
}

static gboolean
parse_asset (GthreeLoader *loader, JsonObject *root, GError **error)
{
  JsonObject *asset = NULL;
  const char *version;

  if (!json_object_has_member (root, "asset"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No asset field, is this really a GLTF file?");
      return FALSE;
    }

  asset = json_object_get_object_member (root, "asset");

  if (!json_object_has_member (asset, "version"))
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No GLTF version specified");
      return FALSE;
    }

  version = json_object_get_string_member (asset, "version");
  if (strcmp (version, "2.0") != 0)
    {
      g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL,
                   "Unsupported GLTF version %s", version);
      return FALSE;
    }

  if (json_object_has_member (asset, "extensionsRequired"))
    {
      JsonArray *required_extensions = json_object_get_array_member (asset, "extensionsRequired");
      int i, len = json_array_get_length (required_extensions);
      for (i = 0; i < len; i++)
        {
          const char *required_extension = json_array_get_string_element (required_extensions, i);
          if (!supports_extension (required_extension))
            {
              g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL,
                           "Unsupported required GLTF extension %s", required_extension);
              return FALSE;
            }
        }
    }
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
      gint64 buffer_view = -1;
      gint64 byte_offset = 0;
      gint64 component_type, item_size, count;
      GthreeAttributeType attribute_type;
      int attribute_type_size;
      const char *type = NULL;
      gboolean normalized = FALSE;
      g_autoptr(Accessor) accessor = g_new0 (Accessor, 1);

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

      accessor->normalized = normalized;

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
      attribute_type_size = gthree_attribute_type_length (attribute_type);

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


      if (buffer_view < 0)
        {
          /* Empty array with the right type */
          accessor->array = gthree_attribute_array_new (attribute_type, count, item_size);
          accessor->item_size = item_size;
          accessor->item_offset = 0;
        }
      else
        {
          BufferView *view;

          if (buffer_view >= priv->buffer_views->len)
            {
              g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "No such buffer view %d", (int)buffer_view);
              return FALSE;
            }

          view = g_ptr_array_index (priv->buffer_views, buffer_view);

          if (view->array && gthree_attribute_array_get_attribute_type (view->array) == attribute_type)
            {
              /* We already have an array for this buffer view, and it matches the type, its either
                 a pure re-use, or an interleaved array we can reuse. */
              accessor->array = gthree_attribute_array_ref (view->array);
              accessor->item_size = item_size;
              accessor->item_offset = byte_offset / attribute_type_size;
              accessor->count = count;
            }
          else
            {
              int item_size_in_shared_array;
              int count_shared_array;

              if (view->byte_stride == 0) // 0 == densely packed
                item_size_in_shared_array = item_size;
              else
                item_size_in_shared_array = view->byte_stride / attribute_type_size;

              count_shared_array = view->byte_length / (attribute_type_size * item_size_in_shared_array);


              /* Create an array for the entire bufferview now that we know the type, then store
                 that for later use and use a subset of it here. */

              accessor->array = gthree_attribute_array_new (attribute_type, count_shared_array, item_size_in_shared_array);
              accessor->item_size = item_size;
              accessor->item_offset = byte_offset / attribute_type_size;
              accessor->count = count;

              memcpy (gthree_attribute_array_peek_uint8 (accessor->array),
                      (char *)g_bytes_get_data (view->bytes, NULL),
                      item_size_in_shared_array * count_shared_array * gthree_attribute_type_length (attribute_type));

              if (view->array == NULL)
                view->array = gthree_attribute_array_ref (accessor->array);
            }
        }

      g_ptr_array_add (priv->accessors, g_steal_pointer (&accessor));
    }

  return TRUE;
}


static GthreeWrapping
convert_wrapping (int wrapping)
{
  if (wrapping == 33071)
    return GTHREE_WRAPPING_CLAMP;
  if (wrapping == 33648)
    return GTHREE_WRAPPING_MIRRORED;
  if (wrapping == 10497)
    return GTHREE_WRAPPING_REPEAT;

  g_warning ("unknown wrapping %d\n", wrapping);
  return GTHREE_WRAPPING_REPEAT;
}

static GthreeFilter
convert_filter (int filter)
{
  if (filter == 9728)
    return GTHREE_FILTER_NEAREST;
  if (filter == 9729)
    return GTHREE_FILTER_LINEAR;
  if (filter == 9984)
    return GTHREE_FILTER_NEAREST_MIPMAP_NEAREST;
  if (filter == 9985)
    return GTHREE_FILTER_LINEAR_MIPMAP_NEAREST;
  if (filter == 9986)
    return GTHREE_FILTER_NEAREST_MIPMAP_LINEAR;
  if (filter == 9987)
    return GTHREE_FILTER_LINEAR_MIPMAP_LINEAR;

  g_warning ("unknown filter %d\n", filter);
  return GTHREE_FILTER_NEAREST;
}

static gboolean
parse_samplers (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *samplers_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "samplers"))
    return TRUE;

  samplers_j = json_object_get_array_member (root, "samplers");
  len = json_array_get_length (samplers_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *sampler_j = json_array_get_object_element (samplers_j, i);
      g_autoptr(Sampler) sampler = g_new0 (Sampler, 1);

      sampler->mag_filter = GTHREE_FILTER_LINEAR;
      sampler->min_filter = GTHREE_FILTER_LINEAR_MIPMAP_LINEAR;
      sampler->wrap_s = GTHREE_WRAPPING_REPEAT;
      sampler->wrap_t = GTHREE_WRAPPING_REPEAT;

      if (json_object_has_member (sampler_j, "magFilter"))
        sampler->mag_filter = convert_filter (json_object_get_int_member (sampler_j, "magFilter"));
      if (json_object_has_member (sampler_j, "minFilter"))
        sampler->min_filter = convert_filter (json_object_get_int_member (sampler_j, "minFilter"));
      if (json_object_has_member (sampler_j, "wrapS"))
        sampler->wrap_s = convert_wrapping (json_object_get_int_member (sampler_j, "wrapS"));
      if (json_object_has_member (sampler_j, "wrapT"))
        sampler->wrap_t = convert_wrapping (json_object_get_int_member (sampler_j, "wrapT"));

      g_ptr_array_add (priv->samplers, g_steal_pointer (&sampler));
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

static gboolean
parse_textures (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *textures_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "textures"))
    return TRUE;

  textures_j = json_object_get_array_member (root, "textures");
  len = json_array_get_length (textures_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *texture_j = json_array_get_object_element (textures_j, i);
      g_autoptr(GthreeTexture) texture = NULL;
      int sampler_idx, source_idx;
      Sampler default_sampler = { GTHREE_FILTER_LINEAR, GTHREE_FILTER_LINEAR, GTHREE_WRAPPING_REPEAT, GTHREE_WRAPPING_REPEAT};
      Sampler *sampler;
      GdkPixbuf *image;

      if (json_object_has_member(texture_j, "sampler"))
        {
          sampler_idx = json_object_get_int_member (texture_j, "sampler");
          sampler = g_ptr_array_index (priv->samplers, sampler_idx);
        }
      else
        {
          sampler = &default_sampler;
        }

      source_idx = json_object_get_int_member (texture_j, "source");

      image = g_ptr_array_index (priv->images, source_idx);

      texture = gthree_texture_new (image);
      gthree_texture_set_wrap_s (texture, sampler->wrap_s);
      gthree_texture_set_wrap_t (texture, sampler->wrap_t);
      gthree_texture_set_mag_filter (texture, sampler->mag_filter);
      gthree_texture_set_min_filter (texture, sampler->min_filter);
      gthree_texture_set_flip_y (texture, FALSE);

      g_ptr_array_add (priv->textures, g_steal_pointer (&texture));
    }

  return TRUE;
}

static GthreeTexture *
parse_texture_ref (GthreeLoader *loader, JsonObject *texture_def)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  gint64 index = json_object_get_int_member (texture_def, "index");

  return g_object_ref (g_ptr_array_index (priv->textures, index));
}

static gboolean
parse_materials (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *materials_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "materials"))
    return TRUE;

  materials_j = json_object_get_array_member (root, "materials");
  len = json_array_get_length (materials_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *material_j = json_array_get_object_element (materials_j, i);
      g_autoptr(GthreeMeshStandardMaterial) material = NULL;
      JsonObject *pbr = NULL;
      GdkRGBA color = {1.0, 1.0, 1.0, 1.0};
      double metallic_factor = 1.0, roughness_factor = 1.0;
      const char *alpha_mode = "OPAQUE";

      if (json_object_has_member (material_j, "pbrMetallicRoughness"))
        pbr = json_object_get_object_member (material_j, "pbrMetallicRoughness");

      if (pbr && json_object_has_member (pbr, "baseColorFactor"))
        parse_color (json_object_get_array_member (pbr, "baseColorFactor"), &color);
      if (pbr && json_object_has_member (pbr, "metallicFactor"))
        metallic_factor = json_object_get_double_member (pbr, "metallicFactor");
      if (pbr && json_object_has_member (pbr, "roughnessFactor"))
        roughness_factor = json_object_get_double_member (pbr, "roughnessFactor");

      material = gthree_mesh_standard_material_new ();

      gthree_mesh_standard_material_set_color (material, &color);
      gthree_material_set_opacity (GTHREE_MATERIAL (material), color.alpha);
      gthree_mesh_standard_material_set_roughness (material, roughness_factor);
      gthree_mesh_standard_material_set_metalness (material, metallic_factor);

      if (pbr)
        {
          if (json_object_has_member (pbr, "baseColorTexture"))
            {
              g_autoptr(GthreeTexture) texture = parse_texture_ref (loader, json_object_get_object_member (pbr, "baseColorTexture"));
              gthree_mesh_standard_material_set_map (material, texture);
            }

          if (json_object_has_member (pbr, "metallicRoughnessTexture"))
            {
              g_autoptr(GthreeTexture) texture = parse_texture_ref (loader, json_object_get_object_member (pbr, "metallicRoughnessTexture"));
              gthree_mesh_standard_material_set_roughness_map (material, texture);
              gthree_mesh_standard_material_set_metalness_map (material, texture);
            }
        }

      if (json_object_has_member (material_j, "normalTexture"))
        {
          JsonObject *texture_j = json_object_get_object_member (material_j, "normalTexture");
          g_autoptr(GthreeTexture) texture = parse_texture_ref (loader, texture_j);
          graphene_vec2_t normal_scale;

          gthree_mesh_standard_material_set_normal_map (material, texture);

          if (json_object_has_member (texture_j, "scale"))
            parse_vec2 (json_object_get_array_member (texture_j, "scale"), &normal_scale);
          else
            graphene_vec2_init (&normal_scale, 1, 1);

          gthree_mesh_standard_material_set_normal_map_scale (material, &normal_scale);
        }

      if (json_object_has_member (material_j, "occlusionTexture"))
        {
          JsonObject *texture_j = json_object_get_object_member (material_j, "occlusionTexture");
          g_autoptr(GthreeTexture) texture = parse_texture_ref (loader, texture_j);
          double intensity = 1.0;

          gthree_mesh_standard_material_set_ao_map (material, texture);

          if (json_object_get_member (texture_j, "strength"))
            intensity = json_object_get_double_member (texture_j, "strength");
          gthree_mesh_standard_material_set_ao_map_intensity (material, intensity);
        }

      if (json_object_has_member (material_j, "emissiveFactor"))
        {
          GdkRGBA e_color;

          parse_color (json_object_get_array_member (material_j, "emissiveFactor"), &e_color);
          gthree_mesh_standard_material_set_emissive_color (material, &e_color);
        }

      if (json_object_has_member (material_j, "emissiveTexture"))
        {
          JsonObject *texture_j = json_object_get_object_member (material_j, "emissiveTexture");
          g_autoptr(GthreeTexture) texture = parse_texture_ref (loader, texture_j);

          gthree_mesh_standard_material_set_emissive_map (material, texture);
        }

      if (json_object_has_member (material_j, "doubleSided") &&
          json_object_get_boolean_member (material_j, "doubleSided"))
        gthree_material_set_side (GTHREE_MATERIAL (material), GTHREE_SIDE_DOUBLE);

      if (json_object_has_member (material_j, "alphaMode"))
        alpha_mode = json_object_get_string_member (material_j, "alphaMode");

      if (g_strcmp0 (alpha_mode, "BLEND") == 0)
        {
          gthree_material_set_is_transparent (GTHREE_MATERIAL (material), TRUE);
        }
      else
        {
          gthree_material_set_is_transparent (GTHREE_MATERIAL (material), FALSE);
          if (g_strcmp0 (alpha_mode, "MASK") == 0)
            {
              double alpha_test = 0.5;
              if (json_object_has_member (material_j, "alphaCutoff"))
                alpha_test = json_object_get_double_member (material_j, "alphaCutoff");
              gthree_material_set_alpha_test (GTHREE_MATERIAL (material), alpha_test);
            }
        }

      g_ptr_array_add (priv->materials, g_steal_pointer (&material));
    }

  return TRUE;
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
      mesh->primitives = g_ptr_array_new_with_free_func ((GDestroyNotify)primitive_free);

      primitives = json_object_get_array_member (mesh_j, "primitives");
      primitives_len = json_array_get_length (primitives);
      for (j = 0; j < primitives_len; j++)
        {
          JsonObject *primitive_j = json_array_get_object_element (primitives, j);
          JsonObject *attributes = json_object_get_object_member (primitive_j, "attributes");
          g_autoptr(Primitive) primitive = g_new0 (Primitive, 1);
          int mode = 4;
          int material = -1;
          g_autoptr(GList) members = json_object_get_members (attributes);
          GList *l;

          primitive->geometry = gthree_geometry_new ();

          for (l = members; l != NULL; l = l->next)
            {
              const char *attr_name = l->data;
              gint64 accessor_index = json_object_get_int_member (attributes, attr_name);
              const char *gthree_name = gltl_attribute_name_to_gthree (attr_name);
              Accessor *accessor = g_ptr_array_index (priv->accessors, accessor_index);
              GthreeAttribute *attribute;

              attribute = gthree_attribute_new_with_array_interleaved (gthree_name,
                                                                       accessor->array,
                                                                       accessor->normalized,
                                                                       accessor->item_size,
                                                                       accessor->item_offset,
                                                                       accessor->count);
              gthree_geometry_add_attribute (primitive->geometry, attribute);
            }

          if (json_object_has_member (primitive_j, "indices"))
            {
              int index_index = json_object_get_int_member (primitive_j, "indices");
              g_autoptr(GthreeAttribute) attribute = NULL;
              Accessor *accessor = g_ptr_array_index (priv->accessors, index_index);

              attribute = gthree_attribute_new_with_array_interleaved ("index",
                                                                       accessor->array,
                                                                       accessor->normalized,
                                                                       accessor->item_size,
                                                                       accessor->item_offset,
                                                                       accessor->count);
              gthree_geometry_set_index (primitive->geometry, attribute);
            }

          if (json_object_has_member (primitive_j, "mode"))
            mode = json_object_get_int_member (primitive_j, "mode");

          if (json_object_has_member (primitive_j, "material"))
            material = json_object_get_int_member (primitive_j, "material");

          if (material != -1)
            primitive->material = g_object_ref (g_ptr_array_index (priv->materials, material));
          else
            primitive->material = g_object_ref (priv->default_material);

          if (mode != 4)
            g_warning ("mode is not TRIANGLES, unsupported");

          g_ptr_array_add (mesh->primitives, g_steal_pointer (&primitive));
        }

      g_ptr_array_add (priv->meshes, g_steal_pointer (&mesh));
    }

  return TRUE;
}

static gboolean
parse_cameras (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *cameras_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "cameras"))
    return TRUE;

  cameras_j = json_object_get_array_member (root, "cameras");
  len = json_array_get_length (cameras_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *camera_j = json_array_get_object_element (cameras_j, i);
      g_autoptr(Camera) camera = g_new0 (Camera, 1);
      const char *type = json_object_get_string_member (camera_j, "type");

      if (strcmp (type, "perspective") == 0)
        {
          JsonObject *perspective = json_object_get_object_member (camera_j, "perspective");

          camera->perspective = TRUE;

          camera->yfov = json_object_get_double_member (perspective, "yfov");
          camera->znear = json_object_get_double_member (perspective, "znear");

          camera->aspect_ratio = 1;
          if (json_object_has_member (perspective, "aspectRatio"))
            camera->aspect_ratio = json_object_get_double_member (perspective, "aspectRatio");

          camera->zfar = 2e6;
          if (json_object_has_member (perspective, "zfar"))
            camera->zfar = json_object_get_double_member (perspective, "zfar");
        }
      else if (strcmp (type, "orthographic") == 0)
        {
          JsonObject *orthographic = json_object_get_object_member (camera_j, "orthographic");

          camera->perspective = FALSE;

          camera->xmag = json_object_get_double_member (orthographic, "xmag");
          camera->ymag = json_object_get_double_member (orthographic, "ymag");
          camera->znear = json_object_get_double_member (orthographic, "znear");
          camera->zfar = json_object_get_double_member (orthographic, "zfar");
        }
      else
        {
          g_set_error (error, GTHREE_LOADER_ERROR, GTHREE_LOADER_ERROR_FAIL, "Unsupported camera type %s", type);
          return FALSE;
        }



      g_ptr_array_add (priv->cameras, g_steal_pointer (&camera));
    }

  return TRUE;
}

static gboolean
parse_skins (GthreeLoader *loader, JsonObject *root, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *skins_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "skins"))
    return TRUE;

  skins_j = json_object_get_array_member (root, "skins");
  len = json_array_get_length (skins_j);
  for (i = 0; i < len; i++)
    {
      JsonObject *skin_j = json_array_get_object_element (skins_j, i);
      g_autoptr(Skin) skin = g_new0 (Skin, 1);
      skin->inverse_bind_matrices = -1;
      skin->skeleton = -1;

      skin->joints = g_array_new (FALSE, TRUE, sizeof (int));
      if (json_object_has_member (skin_j, "joints"))
        {
          JsonArray *joints = json_object_get_array_member (skin_j, "joints");
          int joints_len = json_array_get_length (joints);

          for (int j = 0; j < joints_len; j++)
            {
              int joint_index = json_array_get_int_element (joints, j);
              NodeInfo *node_info = &priv->node_infos[joint_index];

              g_array_append_val (skin->joints, joint_index);

              node_info->is_bone = TRUE;
            }
        }

      if (json_object_has_member (skin_j, "inverseBindMatrices"))
        skin->inverse_bind_matrices = json_object_get_int_member (skin_j, "inverseBindMatrices");

      if (json_object_has_member (skin_j, "skeleton"))
        skin->skeleton = json_object_get_int_member (skin_j, "skeleton");

      g_ptr_array_add (priv->skins, g_steal_pointer (&skin));
    }

  return TRUE;
}

static float
rad_to_deg (float rad)
{
  return rad * 180.0 / G_PI;
}

static gboolean
parse_nodes (GthreeLoader *loader, JsonObject *root, GFile *base_path, GError **error)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  JsonArray *nodes_j = NULL;
  guint len;
  int i;

  if (!json_object_has_member (root, "nodes"))
    return TRUE;

  nodes_j = json_object_get_array_member (root, "nodes");
  len = json_array_get_length (nodes_j);

  /* First create all all base nodes with the right type and local transform */
  for (i = 0; i < len; i++)
    {
      JsonObject *node_j = json_array_get_object_element (nodes_j, i);
      g_autoptr(GthreeObject) node = NULL;
      graphene_point3d_t scale = { 1.f, 1.f, 1.f }, translate = { 0, 0, 0};
      graphene_quaternion_t rotate;

      if (priv->node_infos[i].is_bone)
        node = (GthreeObject *)g_object_ref_sink (gthree_bone_new ());
      else
        node = (GthreeObject *)g_object_ref_sink (gthree_group_new ());

      graphene_quaternion_init_identity (&rotate);

      if (json_object_has_member (node_j, "matrix"))
        {
          JsonArray *matrix_j = json_object_get_array_member (node_j, "matrix");
          graphene_matrix_t m;

          parse_matrix (matrix_j, &m);

          gthree_object_set_matrix_auto_update (node, FALSE);
          gthree_object_set_matrix (node, &m);
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

          gthree_object_set_position (node, &translate);
          gthree_object_set_quaternion (node, &rotate);
          gthree_object_set_scale (node, &scale);
        }

      g_ptr_array_add (priv->nodes, g_steal_pointer (&node));
    }

  /* Then apply the hierarchy */
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

  /* Then create extra objects like meshes and cameras */
  for (i = 0; i < len; i++)
    {
      JsonObject *node_j = json_array_get_object_element (nodes_j, i);
      GthreeObject *node = g_ptr_array_index (priv->nodes, i);

      // TODO: weights (morph targets)

      if (json_object_has_member (node_j, "mesh"))
        {
          gint64 mesh_id = json_object_get_int_member (node_j, "mesh");
          Mesh *mesh = g_ptr_array_index (priv->meshes, mesh_id);
          Skin *skin = NULL;
          int j;

          if (json_object_has_member (node_j, "skin"))
            {
              gint64 skin_id = json_object_get_int_member (node_j, "skin");

              // This is used below for each primitive mesh
              skin = g_ptr_array_index (priv->skins, skin_id);
            }

          for (j = 0; j < mesh->primitives->len; j++)
            {
              Primitive *primitive = g_ptr_array_index (mesh->primitives, j);
              GthreeMaterial *base_material = primitive->material;
              GthreeMaterial *material;
              MaterialCacheKey cache_key = { base_material };
              GthreeMesh *mesh;

              // TODO: Enable this when it works
              //cache_key.use_skinning = skin != NULL;

              cache_key.use_vertex_tangents =
                gthree_geometry_has_attribute (primitive->geometry,
                                               gthree_attribute_name_get_for_static ("tangent"));
              cache_key.use_vertex_colors =
                gthree_geometry_has_attribute (primitive->geometry,
                                               gthree_attribute_name_get_for_static ("color"));

              material = g_hash_table_lookup (priv->final_materials_hash, &cache_key);
              if (material == NULL)
                {
                  MaterialCacheKey *cache_key_copy = material_cache_key_clone (&cache_key);

                  material = gthree_material_clone (base_material);
                  if (cache_key.use_vertex_colors)
                    gthree_material_set_vertex_colors (material, TRUE);
                  if (cache_key.use_skinning)
                    gthree_mesh_material_set_skinning (GTHREE_MESH_MATERIAL (material), TRUE);
                  //TODO: if (cache_key.use_vertex_tangents)

                  g_hash_table_insert (priv->final_materials_hash, cache_key_copy, material);
                  g_ptr_array_add (priv->final_materials, material);
                }

              /* TODO: more cache keys
               * var useFlatShading = geometry.attributes.normal === undefined;
               * var useSkinning = mesh.isSkinnedMesh === true;
               * var useMorphTargets = Object.keys( geometry.morphAttributes ).length > 0;
               * var useMorphNormals = useMorphTargets && geometry.morphAttributes.normal !== undefined;
               */

              if (skin != NULL)
                {
                  GthreeSkeleton *skeleton;
                  g_autofree GthreeBone **bones = g_new0 (GthreeBone *, skin->joints->len);
                  g_autofree graphene_matrix_t *bone_inverses = g_new0 (graphene_matrix_t, skin->joints->len);
                  int b;

                  for (b = 0; b < skin->joints->len; b++)
                    {
                      bones[b] = g_ptr_array_index (priv->nodes, g_array_index (skin->joints, int, b));
                      graphene_matrix_init_identity (&bone_inverses[b]);
                    }

                  mesh = (GthreeMesh *)gthree_skinned_mesh_new (primitive->geometry, g_object_ref (material));

                  if (skin->inverse_bind_matrices >= 0)
                    {
                      Accessor *accessor = g_ptr_array_index (priv->accessors, skin->inverse_bind_matrices);

                      if (accessor->count != skin->joints->len)
                        g_warning ("Wrong inverse bind matrices size");
                      else
                        {
                          for (b = 0; b < skin->joints->len; b++)
                            gthree_attribute_array_get_matrix (accessor->array, b, 0, &bone_inverses[b]);
                        }
                    }

                  // From three.js, see #15319
                  gthree_skinned_mesh_normalize_skin_weights (GTHREE_SKINNED_MESH (mesh));

                  skeleton = gthree_skeleton_new (bones, skin->joints->len, bone_inverses);
                  gthree_skinned_mesh_bind (GTHREE_SKINNED_MESH (mesh), skeleton,
                                            gthree_object_get_world_matrix (GTHREE_OBJECT (mesh)));
                }
              else
                mesh = gthree_mesh_new (primitive->geometry, g_object_ref (material));

              gthree_object_add_child (node, GTHREE_OBJECT (mesh));
            }

        }

      if (json_object_has_member (node_j, "camera"))
        {
          gint64 camera_id = json_object_get_int_member (node_j, "camera");
          Camera *camera = g_ptr_array_index (priv->cameras, camera_id);
          GthreeCamera *camera_node = NULL;

          if (camera->perspective)
            camera_node = (GthreeCamera *)gthree_perspective_camera_new (rad_to_deg (camera->yfov), camera->aspect_ratio,
                                                                         camera->znear, camera->zfar);
          else
            g_warning ("Unsupported orthographic camera");

          if (camera_node)
            gthree_object_add_child (node, GTHREE_OBJECT (camera_node));
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

  init_node_info (loader, root);

  if (!parse_asset (loader, root, error))
    return NULL;

  if (!parse_buffers (loader, root, bin, base_path, error))
    return NULL;

  if (!parse_buffer_views (loader, root, error))
    return NULL;

  if (!parse_accessors (loader, root, error))
    return NULL;

  if (!parse_samplers (loader, root, error))
    return NULL;

  if (!parse_images (loader, root, base_path, error))
    return NULL;

  if (!parse_textures (loader, root, error))
    return NULL;

  if (!parse_materials (loader, root, error))
    return NULL;

  if (!parse_meshes (loader, root, error))
    return NULL;

  if (!parse_cameras (loader, root, error))
    return NULL;

  if (!parse_skins (loader, root, error))
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

int
gthree_loader_get_n_materials (GthreeLoader *loader)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);

  return priv->final_materials->len;
}

GthreeMaterial *
gthree_loader_get_material (GthreeLoader *loader,
                            int index)
{
  GthreeLoaderPrivate *priv = gthree_loader_get_instance_private (loader);
  GthreeMaterial *material = g_ptr_array_index (priv->final_materials, index);

  return material;
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


