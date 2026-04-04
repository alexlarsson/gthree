#include <draco/compression/decode.h>
#include <draco/mesh/mesh.h>

extern "C" {
#include "gthreedraco.h"
}

static GthreeAttributeType
draco_data_type_to_gthree (draco::DataType dt)
{
  switch (dt)
    {
    case draco::DT_INT8:    return GTHREE_ATTRIBUTE_TYPE_INT8;
    case draco::DT_UINT8:   return GTHREE_ATTRIBUTE_TYPE_UINT8;
    case draco::DT_INT16:   return GTHREE_ATTRIBUTE_TYPE_INT16;
    case draco::DT_UINT16:  return GTHREE_ATTRIBUTE_TYPE_UINT16;
    case draco::DT_UINT32:  return GTHREE_ATTRIBUTE_TYPE_UINT32;
    case draco::DT_FLOAT32: return GTHREE_ATTRIBUTE_TYPE_FLOAT;
    default:                return GTHREE_ATTRIBUTE_TYPE_FLOAT;
    }
}

static const char *
gltf_attribute_name_to_gthree (const char *name)
{
  if (g_str_equal (name, "POSITION"))   return "position";
  if (g_str_equal (name, "NORMAL"))     return "normal";
  if (g_str_equal (name, "TANGENT"))    return "tangent";
  if (g_str_equal (name, "TEXCOORD_0")) return "uv";
  if (g_str_equal (name, "TEXCOORD_1")) return "uv2";
  if (g_str_equal (name, "COLOR_0"))    return "color";
  if (g_str_equal (name, "WEIGHTS_0"))  return "skinWeight";
  if (g_str_equal (name, "JOINTS_0"))   return "skinIndex";
  return name;
}

gboolean
gthree_draco_decode (const guint8   *data,
                     gsize           len,
                     GthreeGeometry *geometry,
                     JsonObject     *draco_ext,
                     JsonObject     *gltf_attributes,
                     GError        **error)
{
  draco::DecoderBuffer buffer;
  buffer.Init (reinterpret_cast<const char *> (data), len);

  draco::Decoder decoder;
  auto status_or = decoder.DecodeMeshFromBuffer (&buffer);
  if (!status_or.ok ())
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "Draco decode failed: %s",
                   status_or.status ().error_msg ());
      return FALSE;
    }

  auto mesh = std::move (status_or).value ();
  int num_points = mesh->num_points ();
  int num_faces = mesh->num_faces ();

  JsonObject *draco_attributes = json_object_get_object_member (draco_ext, "attributes");

  /* Decode vertex attributes */
  g_autoptr(GList) members = json_object_get_members (draco_attributes);
  for (GList *l = members; l != NULL; l = l->next)
    {
      const char *attr_name = static_cast<const char *> (l->data);

      /* Only add attributes that the primitive actually declares */
      if (!json_object_has_member (gltf_attributes, attr_name))
        continue;

      int draco_id = json_object_get_int_member (draco_attributes, attr_name);
      const draco::PointAttribute *attr = mesh->GetAttributeByUniqueId (draco_id);
      if (!attr)
        {
          g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                       "Draco attribute %s (id %d) not found", attr_name, draco_id);
          return FALSE;
        }

      int num_components = attr->num_components ();
      GthreeAttributeType gtype = draco_data_type_to_gthree (attr->data_type ());
      int type_size = gthree_attribute_type_length (gtype);

      GthreeAttributeArray *array = gthree_attribute_array_new (gtype, num_points, num_components);
      guint8 *out = gthree_attribute_array_peek_uint8 (array);

      for (draco::PointIndex i (0); i < num_points; ++i)
        {
          attr->GetValue (attr->mapped_index (i),
                          out + i.value () * num_components * type_size);
        }

      const char *gthree_name = gltf_attribute_name_to_gthree (attr_name);
      GthreeAttribute *attribute =
        gthree_attribute_new_with_array_interleaved (gthree_name, array,
                                                     attr->normalized (),
                                                     num_components, 0,
                                                     num_points);
      gthree_geometry_add_attribute (geometry, gthree_name, attribute);
      gthree_attribute_array_unref (array);
    }

  /* Decode face indices */
  if (num_faces > 0)
    {
      int num_indices = num_faces * 3;
      GthreeAttributeArray *index_array =
        gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_UINT32, num_indices, 1);
      guint32 *indices = reinterpret_cast<guint32 *> (
        gthree_attribute_array_peek_uint8 (index_array));

      for (draco::FaceIndex fi (0); fi < num_faces; ++fi)
        {
          const auto &face = mesh->face (fi);
          int base = fi.value () * 3;
          indices[base + 0] = face[0].value ();
          indices[base + 1] = face[1].value ();
          indices[base + 2] = face[2].value ();
        }

      GthreeAttribute *index_attr =
        gthree_attribute_new_with_array_interleaved ("index", index_array,
                                                     FALSE, 1, 0, num_indices);
      gthree_geometry_set_index (geometry, index_attr);
      gthree_attribute_array_unref (index_array);
    }

  return TRUE;
}
