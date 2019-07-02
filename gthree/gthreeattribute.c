#include <math.h>
#include <epoxy/gl.h>

#include "gthreeattribute.h"
#include "gthreeprivate.h"
#include "gthreeenums.h"

struct _GthreeAttributeArray {
  int ref_count;
  GthreeAttributeType type;
  int stride; /* in nr of type items */
  int count;  /* in nr of stride items */
  int version;
  /* TODO: move this to per-array? it can be in different ranges when reusing arrays interleaved or stacked */
  int update_range_offset;
  int update_range_count;
  gboolean dynamic;

  /* realized state */

  guint gl_buffer;
  gboolean dirty;

  guint8 data[0];
};

static gsize attribute_type_size[] = { 8, 4, 4, 4, 2, 2, 1, 1};
static int attribute_type_gl[] = {
   GL_DOUBLE,
   GL_FLOAT,
   GL_UNSIGNED_INT,
   GL_INT,
   GL_UNSIGNED_SHORT,
   GL_SHORT,
   GL_UNSIGNED_BYTE,
   GL_BYTE
};

int
gthree_attribute_type_length (GthreeAttributeType type)
{
  return attribute_type_size[type];
}

GthreeAttributeArray *
gthree_attribute_array_new   (GthreeAttributeType   type,
                              int                   count,
                              int                   stride)
{
  GthreeAttributeArray *array;
  gsize len = count * stride;

  g_assert (type < 8);

  array = g_malloc0 (sizeof (GthreeAttributeArray) + attribute_type_size[type] * len);
  array->ref_count = 1;
  array->type = type;
  array->count = count;
  array->stride = stride;
  array->update_range_count = -1;

  return array;
}

GthreeAttributeArray *
gthree_attribute_array_new_from_float (float                *data,
                                       int                   count,
                                       int                   item_size)
{
  GthreeAttributeArray *array;

  array = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_FLOAT, count, item_size);
  gthree_attribute_array_copy_float  (array, 0, 0, data, item_size, item_size, count);
  return array;
}

GthreeAttributeArray *
gthree_attribute_array_new_from_uint16 (guint16              *data,
                                        int                   count,
                                        int                   item_size)
{
  GthreeAttributeArray *array;

  array = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_UINT16, count, item_size);
  gthree_attribute_array_copy_uint16  (array, 0, 0, data, item_size, item_size, count);
  return array;
}

GthreeAttributeArray *
gthree_attribute_array_new_from_uint32 (guint32              *data,
                                        int                   count,
                                        int                   item_size)
{
  GthreeAttributeArray *array;

  array = gthree_attribute_array_new (GTHREE_ATTRIBUTE_TYPE_UINT32, count, item_size);
  gthree_attribute_array_copy_uint32  (array, 0, 0, data, item_size, item_size, count);
  return array;
}

GthreeAttributeArray *gthree_attribute_array_reshape (GthreeAttributeArray *array,
                                                      guint                 index,
                                                      guint                 offset,
                                                      guint                 count,
                                                      guint                 item_size,
                                                      gboolean              share_if_possible)
{
  GthreeAttributeArray *reshaped;
  guint len, remaining_len, subset_len, element_size;

  if (share_if_possible &&
      index == 0 && offset == 0 && count == array->count && item_size == array->stride)
    return gthree_attribute_array_ref (array);

  element_size = attribute_type_size[array->type];

  len = array->count * array->stride;
  remaining_len = len - (index * array->stride + offset);
  subset_len = item_size * count;
  g_assert (subset_len <= remaining_len);

  reshaped = gthree_attribute_array_new (array->type, count, item_size);
  memcpy (reshaped->data,
          array->data + (index * array->stride + offset) * element_size,
          subset_len * element_size);
  return reshaped;
}

GthreeAttributeArray *
gthree_attribute_array_ref (GthreeAttributeArray *array)
{
  g_assert (array->ref_count > 0);
  array->ref_count++;
  return array;
}

void
gthree_attribute_array_unref (GthreeAttributeArray *array)
{
  g_assert (array->gl_buffer == 0);
  g_assert (array->ref_count > 0);
  array->ref_count--;
  if (array->ref_count == 0)
    g_free (array);
}

GthreeAttributeType
gthree_attribute_array_get_attribute_type (GthreeAttributeArray *array)
{
  return array->type;
}

int
gthree_attribute_array_get_len (GthreeAttributeArray *array)
{
  return array->count * array->stride;
}

int
gthree_attribute_array_get_count (GthreeAttributeArray *array)
{
  return array->count;
}

int
gthree_attribute_array_get_stride (GthreeAttributeArray *array)
{
  return array->stride;
}

static void
gthree_attribute_array_destroy_buffer (GthreeAttributeArray *array)
{
  if (array->gl_buffer != 0)
    {
      glDeleteBuffers (1, &array->gl_buffer);
      array->gl_buffer = 0;
    }
}

static void
gthree_attribute_array_create_buffer (GthreeAttributeArray *array, int buffer_type)
{
  int usage = array->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
  int element_size = attribute_type_size[array->type];

  if (array->gl_buffer == 0)
    glGenBuffers (1, &array->gl_buffer);

  glBindBuffer (buffer_type, array->gl_buffer);

  glBufferData (buffer_type, gthree_attribute_array_get_len (array) * element_size, &array->data[0], usage);
  array->dirty = FALSE;
}

static void
gthree_attribute_array_update_buffer (GthreeAttributeArray *array, int buffer_type)
{
  int usage = array->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
  int element_size = attribute_type_size[array->type];

  glBindBuffer (buffer_type, array->gl_buffer);
  if (!array->dynamic)
    {
      glBufferData (buffer_type, gthree_attribute_array_get_len (array) * element_size, &array->data[0], usage);
    }
  else if (array->update_range_count == -1)
    {
      // Not using update ranges
      glBufferSubData (buffer_type, 0,
                       gthree_attribute_array_get_len (array) * element_size, &array->data[0]);
    }
  else
    {
      glBufferSubData (buffer_type, array->update_range_offset * element_size,
                       array->update_range_count * element_size,
                       ((guint8 *)&array->data[0]) + array->update_range_offset * element_size);
      array->update_range_count = -1; // reset range
    }

  array->dirty = FALSE;
}

guint8 *
gthree_attribute_array_peek_uint8 (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT8 || GTHREE_ATTRIBUTE_TYPE_INT8);
  return (guint8*)&array->data[0];
}

guint8 *
gthree_attribute_array_peek_uint8_at (GthreeAttributeArray *array,
                                      int index,
                                      int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_uint8 (array) + n;
}

gint8 *
gthree_attribute_array_peek_int8 (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT8 || GTHREE_ATTRIBUTE_TYPE_INT8);
  return (gint8*)&array->data[0];
}

gint8 *
gthree_attribute_array_peek_int8_at (GthreeAttributeArray *array,
                                     int index,
                                     int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_int8 (array) + n;
}

gint16 *
gthree_attribute_array_peek_int16 (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT16 || GTHREE_ATTRIBUTE_TYPE_INT16);
  return (gint16*)&array->data[0];
}

gint16 *
gthree_attribute_array_peek_int16_at (GthreeAttributeArray *array,
                                      int index,
                                      int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_int16 (array) + n;
}

guint16 *
gthree_attribute_array_peek_uint16 (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT16 || GTHREE_ATTRIBUTE_TYPE_INT16);
  return (guint16*)&array->data[0];
}

guint16 *
gthree_attribute_array_peek_uint16_at (GthreeAttributeArray *array,
                                       int index,
                                       int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_uint16 (array) + n;
}

gint32 *
gthree_attribute_array_peek_int32 (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT32 || GTHREE_ATTRIBUTE_TYPE_INT32);
  return (gint32*)&array->data[0];
}

gint32 *
gthree_attribute_array_peek_int32_at (GthreeAttributeArray *array,
                                      int index,
                                      int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_int32 (array) + array->stride * index;
}

guint32 *
gthree_attribute_array_peek_uint32 (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT32 || GTHREE_ATTRIBUTE_TYPE_INT32);
  return (guint32*)&array->data[0];
}

guint32 *
gthree_attribute_array_peek_uint32_at (GthreeAttributeArray *array,
                                      int index,
                                      int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_uint32 (array) + n;
}

float *
gthree_attribute_array_peek_float (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_FLOAT);
  return (float*)&array->data[0];
}

graphene_point3d_t *
gthree_attribute_array_peek_point3d   (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_FLOAT);
  return (graphene_point3d_t*)&array->data[0];
}

graphene_point3d_t *
gthree_attribute_array_peek_point3d_at (GthreeAttributeArray *array,
                                        int index,
                                        int offset)
{
  return (graphene_point3d_t *)gthree_attribute_array_peek_float_at (array, index, offset);
}

float *
gthree_attribute_array_peek_float_at (GthreeAttributeArray *array,
                                      int index,
                                      int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_float (array) + n;
}

float
gthree_attribute_array_get_float_at (GthreeAttributeArray *array,
                                     int                   index,
                                     int                   offset)
{
  return *gthree_attribute_array_peek_float_at (array, index, offset);
}

double *
gthree_attribute_array_peek_double (GthreeAttributeArray *array)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_DOUBLE);
  return (double*)&array->data[0];
}

double *
gthree_attribute_array_peek_double_at (GthreeAttributeArray *array,
                                       int index,
                                       int offset)
{
  int n = array->stride * index + offset;
  g_assert (n < array->count * array->stride);

  return gthree_attribute_array_peek_double (array) + n;
}


void
gthree_attribute_array_set_x (GthreeAttributeArray *array,
                              guint                 index,
                              guint                 offset,
                              float                 x)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[0] = x;
}

void
gthree_attribute_array_set_y (GthreeAttributeArray *array,
                              guint                 index,
                              guint                 offset,
                              float                 y)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[1] = y;
}

void
gthree_attribute_array_set_z (GthreeAttributeArray *array,
                              guint                 index,
                              guint                 offset,
                              float                 z)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[2] = z;
}

void
gthree_attribute_array_set_w (GthreeAttributeArray *array,
                              guint                 index,
                              guint                 offset,
                              float                 w)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[3] = w;
}

void
gthree_attribute_array_set_xy (GthreeAttributeArray *array,
                               guint                 index,
                               guint                 offset,
                               float                 x,
                               float                 y)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[0] = x;
  p[1] = y;
}

void
gthree_attribute_array_set_xyz (GthreeAttributeArray *array,
                                guint                 index,
                                guint                 offset,
                                float                 x,
                                float                 y,
                                float                 z)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[0] = x;
  p[1] = y;
  p[2] = z;
}

void
gthree_attribute_array_get_xyz (GthreeAttributeArray *array,
                                guint                 index,
                                guint                 offset,
                                float                *x,
                                float                *y,
                                float                *z)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  *x = p[0];
  *y = p[1];
  *z = p[2];
}

void
gthree_attribute_array_set_xyzw (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 float                 x,
                                 float                 y,
                                 float                 z,
                                 float                 w)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  p[0] = x;
  p[1] = y;
  p[2] = z;
  p[3] = w;
}

void
gthree_attribute_array_get_xyzw (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 float                *x,
                                 float                *y,
                                 float                *z,
                                 float                *w)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  *x = p[0];
  *y = p[1];
  *z = p[2];
  *w = p[3];
}

void
gthree_attribute_array_set_point3d (GthreeAttributeArray *array,
                                    guint                 index,
                                    guint                 offset,
                                    graphene_point3d_t   *point)
{
  gthree_attribute_array_set_xyz (array, index, offset,
                                  point->x,
                                  point->y,
                                  point->z);
}

void
gthree_attribute_array_set_vec2 (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 graphene_vec2_t      *vec2)
{
  gthree_attribute_array_set_xy (array, index, offset,
                                 graphene_vec2_get_x (vec2),
                                 graphene_vec2_get_y (vec2));
}

void
gthree_attribute_array_set_vec3 (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 graphene_vec3_t      *vec3)
{
  gthree_attribute_array_set_xyz (array, index, offset,
                                  graphene_vec3_get_x (vec3),
                                  graphene_vec3_get_y (vec3),
                                  graphene_vec3_get_z (vec3));
}

void
gthree_attribute_array_set_vec4 (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 graphene_vec4_t      *vec4)
{
  gthree_attribute_array_set_xyzw (array, index, offset,
                                   graphene_vec4_get_x (vec4),
                                   graphene_vec4_get_y (vec4),
                                   graphene_vec4_get_z (vec4),
                                   graphene_vec4_get_w (vec4));
}

void
gthree_attribute_array_get_vec4 (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 graphene_vec4_t      *vec4)
{
  float x, y, z, w;
  gthree_attribute_array_get_xyzw (array, index, offset, &x, &y, &z, &w);
  graphene_vec4_init (vec4, x, y, z, w);
}

void
gthree_attribute_array_get_matrix (GthreeAttributeArray *array,
                                   guint                 index,
                                   guint                 offset,
                                   graphene_matrix_t    *matrix)
{
  float *p = gthree_attribute_array_peek_float_at (array, index, offset);
  graphene_matrix_init_from_float (matrix, p);
}

void
gthree_attribute_array_set_rgb (GthreeAttributeArray *array,
                                guint                 index,
                                guint                 offset,
                                GdkRGBA              *color)
{
  gthree_attribute_array_set_xyz (array, index, offset,
                                  color->red,
                                  color->green,
                                  color->blue);
}

void
gthree_attribute_array_set_rgba (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 GdkRGBA              *color)
{
  gthree_attribute_array_set_xyzw (array, index, offset,
                                   color->red,
                                   color->green,
                                   color->blue,
                                   color->alpha);
}

void
gthree_attribute_array_set_uint8 (GthreeAttributeArray *array,
                                   guint                 index,
                                   guint                 offset,
                                   gint8                 value)
{
  guint8 *p = gthree_attribute_array_peek_uint8_at (array, index, offset);
  *p = value;
}

guint8
gthree_attribute_array_get_uint8 (GthreeAttributeArray *array,
                                  guint                 index,
                                  guint                 offset)
{
  guint8 *p = gthree_attribute_array_peek_uint8_at (array, index, offset);
  return *p;
}

void
gthree_attribute_array_set_uint16 (GthreeAttributeArray *array,
                                   guint                 index,
                                   guint                 offset,
                                   gint16                value)
{
  guint16 *p = gthree_attribute_array_peek_uint16_at (array, index, offset);
  *p = value;
}

guint16
gthree_attribute_array_get_uint16 (GthreeAttributeArray *array,
                                   guint                 index,
                                   guint                 offset)
{
  guint16 *p = gthree_attribute_array_peek_uint16_at (array, index, offset);
  return *p;
}

void
gthree_attribute_array_set_uint32 (GthreeAttributeArray *array,
                                   guint                 index,
                                   guint                 offset,
                                   guint32               value)
{
  guint32 *p = gthree_attribute_array_peek_uint32_at (array, index, offset);
  *p = value;
}

guint32
gthree_attribute_array_get_uint32 (GthreeAttributeArray *array,
                                  guint                 index,
                                  guint                 offset)
{
  guint32 *p = gthree_attribute_array_peek_uint32_at (array, index, offset);
  return *p;
}

void
gthree_attribute_array_set_uint (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset,
                                 guint                 value)
{
  if (array->type == GTHREE_ATTRIBUTE_TYPE_UINT32 || array->type == GTHREE_ATTRIBUTE_TYPE_INT32)
    gthree_attribute_array_set_uint32 (array, index, offset, value);
  else if (array->type == GTHREE_ATTRIBUTE_TYPE_UINT16 || array->type == GTHREE_ATTRIBUTE_TYPE_INT16)
    gthree_attribute_array_set_uint16 (array, index, offset, value);
  else
    gthree_attribute_array_set_uint8 (array, index, offset, value);
}

guint
gthree_attribute_array_get_uint (GthreeAttributeArray *array,
                                 guint                 index,
                                 guint                 offset)
{
  if (array->type == GTHREE_ATTRIBUTE_TYPE_UINT32 || array->type == GTHREE_ATTRIBUTE_TYPE_INT32)
    return gthree_attribute_array_get_uint32 (array, index, offset);
  else if (array->type == GTHREE_ATTRIBUTE_TYPE_UINT16 || array->type == GTHREE_ATTRIBUTE_TYPE_INT16)
    return gthree_attribute_array_get_uint16 (array, index, offset);
  else
    return gthree_attribute_array_get_uint8 (array, index, offset);
}

void
gthree_attribute_array_get_point3d (GthreeAttributeArray *array,
                                    guint                 index,
                                    guint                 offset,
                                    graphene_point3d_t   *point)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_FLOAT);

  *point = *(graphene_point3d_t *)gthree_attribute_array_peek_float_at (array, index, offset);
}

void
gthree_attribute_array_get_elements_as_float (GthreeAttributeArray *array,
                                              guint                 index,
                                              guint                 offset,
                                              float                *dest,
                                              guint                 n_elements)
{
  guint i;

  switch (array->type)
    {
    case GTHREE_ATTRIBUTE_TYPE_FLOAT:
      {
        float *floats = gthree_attribute_array_peek_float_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = floats[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_DOUBLE:
      {
        double *values = gthree_attribute_array_peek_double_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT32:
      {
        guint32 *values = gthree_attribute_array_peek_uint32_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT32:
      {
        gint32 *values = gthree_attribute_array_peek_int32_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT16:
      {
        guint16 *values = gthree_attribute_array_peek_uint16_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT16:
      {
        gint16 *values = gthree_attribute_array_peek_int16_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT8:
      {
        guint8 *values = gthree_attribute_array_peek_uint8_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT8:
      {
        gint8 *values = gthree_attribute_array_peek_int8_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (float)values[i];
        break;
      }
    }
}

void
gthree_attribute_array_set_elements_from_float (GthreeAttributeArray *array,
                                                guint                 index,
                                                guint                 offset,
                                                float                *src,
                                                guint                 n_elements)
{
  guint i;

  switch (array->type)
    {
    case GTHREE_ATTRIBUTE_TYPE_FLOAT:
      {
        float *floats = gthree_attribute_array_peek_float_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          floats[i] = src[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_DOUBLE:
      {
        double *dest = gthree_attribute_array_peek_double_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (double)src[i];
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT32:
      {
        guint32 *dest = gthree_attribute_array_peek_uint32_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (guint32)roundf(src[i]);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT32:
      {
        gint32 *dest = gthree_attribute_array_peek_int32_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (gint32)roundf(src[i]);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT16:
      {
        guint16 *dest = gthree_attribute_array_peek_uint16_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (guint16)roundf(src[i]);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT16:
      {
        gint16 *dest = gthree_attribute_array_peek_int16_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (gint16)roundf(src[i]);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT8:
      {
        guint8 *dest = gthree_attribute_array_peek_uint8_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (guint8)roundf(src[i]);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT8:
      {
        gint8 *dest = gthree_attribute_array_peek_int8_at (array, index, offset);
        for (i = 0; i < n_elements; i++)
          dest[i] = (gint8)roundf(src[i]);
        break;
      }
    }
}

static void
gthree_attribute_array_copy_raw (GthreeAttributeArray *array,
                                 guint index,
                                 guint offset,
                                 gpointer source,
                                 guint source_stride,
                                 guint n_elements,
                                 guint n_items)
{
  guint i;
  guint8 *src, *dst;
  gsize copy_len_bytes, dst_stride_bytes, source_stride_bytes;
  gsize element_size;

  element_size = attribute_type_size[array->type];
  copy_len_bytes = n_elements * element_size;

  source_stride_bytes = element_size * source_stride;
  dst_stride_bytes = element_size * array->stride;
  dst = (guint8*)&array->data[0] + index * dst_stride_bytes + offset * element_size;

  src = (guint8*)source;

  if (source_stride_bytes == dst_stride_bytes && copy_len_bytes == source_stride_bytes)
    {
      memmove (dst, src, copy_len_bytes * n_items);
    }
  else
    {
      for (i = 0; i < n_items; i++)
        {
          memmove (dst, src, copy_len_bytes);
          dst += dst_stride_bytes;
          src += source_stride_bytes;
        }
    }
}

void
gthree_attribute_array_copy_at (GthreeAttributeArray *array,
                                guint index,
                                guint offset,
                                GthreeAttributeArray *source,
                                guint source_index,
                                guint source_offset,
                                guint n_elements,
                                guint n_items)
{
  guint8 *src;
  gsize src_stride;

  g_assert (attribute_type_size[array->type] == attribute_type_size[source->type]);

  src = (guint8*)&source->data[0] +  attribute_type_size[source->type] * (source_index * source->stride + source_offset);
  src_stride = source->stride;
  gthree_attribute_array_copy_raw (array, index, offset,
                                   src, src_stride,
                                   n_elements,
                                   n_items);
}

void
gthree_attribute_array_copy_float (GthreeAttributeArray *array,
                                   guint index,
                                   guint offset,
                                   float *source,
                                   guint source_stride,
                                   guint n_elements,
                                   guint n_items)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_FLOAT);

  gthree_attribute_array_copy_raw (array, index, offset,
                                   source, source_stride,
                                   n_elements,
                                   n_items);
}

void
gthree_attribute_array_copy_uint32 (GthreeAttributeArray *array,
                                    guint index,
                                    guint offset,
                                    guint32 *source,
                                    guint source_stride,
                                    guint n_elements,
                                    guint n_items)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT32);

  gthree_attribute_array_copy_raw (array, index, offset,
                                   source, source_stride,
                                   n_elements,
                                   n_items);
}

void
gthree_attribute_array_copy_uint16 (GthreeAttributeArray *array,
                                    guint index,
                                    guint offset,
                                    guint16 *source,
                                    guint source_stride,
                                    guint n_elements,
                                    guint n_items)
{
  g_assert (array->type == GTHREE_ATTRIBUTE_TYPE_UINT16);

  gthree_attribute_array_copy_raw (array, index, offset,
                                   source, source_stride,
                                   n_elements,
                                   n_items);
}

static void gthree_attribute_real_unrealize (GthreeResource *resource);

struct _GthreeAttribute {
  GthreeResource parent;

  const char *name_intern;
  GthreeAttributeArray *array;
  int item_size;    /* typically same as array->stride, but not of interleaved */
  int item_offset;  /* typically 0, but not if interleaved or stacked */
  int count;        /* May be smaller than the entire array if stacking */
  gboolean normalized;
};

typedef struct {
  GthreeResourceClass parent_class;
} GthreeAttributeClass;

enum {
  PROP_0,

  N_PROPS
};

//static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE (GthreeAttribute, gthree_attribute, GTHREE_TYPE_RESOURCE)

GthreeAttribute *
gthree_attribute_new_with_array_interleaved (const char *name,
                                             GthreeAttributeArray *array,
                                             gboolean normalized,
                                             int item_size,
                                             int item_offset,
                                             int count)
{
  GthreeAttribute *attr;

  /* TODO: Make this properties */
  attr = g_object_new (gthree_attribute_get_type (),
                       NULL);

  attr->name_intern =  g_intern_string (name);
  attr->array = gthree_attribute_array_ref (array);
  attr->normalized = normalized;
  attr->item_size = item_size;
  attr->item_offset = item_offset;
  attr->count = count;

  return attr;
}

GthreeAttribute *
gthree_attribute_new_with_array (const char *name,
                                 GthreeAttributeArray *array,
                                 gboolean normalized)
{
  return gthree_attribute_new_with_array_interleaved (name, array,
                                                      normalized,
                                                      array->stride,
                                                      0,
                                                      array->count);
}

GthreeAttribute *
gthree_attribute_new (const char           *name,
                      GthreeAttributeType   type,
                      int                   count,
                      int                   item_size,
                      gboolean              normalized)
{
  GthreeAttribute *attribute;
  GthreeAttributeArray *array = gthree_attribute_array_new (type, count, item_size);

  attribute = gthree_attribute_new_with_array (name, array, normalized);

  gthree_attribute_array_unref (array);

  return attribute;
}

GthreeAttribute *
gthree_attribute_copy (const char           *name,
                       GthreeAttribute      *source)
{
  GthreeAttribute *attribute;
  GthreeAttributeArray *array = gthree_attribute_array_new (source->array->type,
                                                            source->array->count,
                                                            source->array->stride);

  attribute = gthree_attribute_new_with_array_interleaved (name, array,
                                                           source->normalized,
                                                           source->item_size,
                                                           source->item_offset,
                                                           source->count);

  gthree_attribute_array_unref (array);

  gthree_attribute_copy_at (attribute, 0, source, 0, source->count);

  return attribute;
}


GthreeAttribute *
gthree_attribute_new_from_float (const char           *name,
                                 float                *data,
                                 int                   count,
                                 int                   item_size)
{
  GthreeAttribute *attribute;
  GthreeAttributeArray *array = gthree_attribute_array_new_from_float (data, count, item_size);
  attribute = gthree_attribute_new_with_array (name, array, FALSE);
  gthree_attribute_array_unref (array);
  return attribute;
}

GthreeAttribute *
gthree_attribute_new_from_uint16 (const char           *name,
                                  guint16              *data,
                                  int                   count,
                                  int                   item_size)
{
  GthreeAttribute *attribute;
  GthreeAttributeArray *array = gthree_attribute_array_new_from_uint16 (data, count, item_size);
  attribute = gthree_attribute_new_with_array (name, array, FALSE);
  gthree_attribute_array_unref (array);
  return attribute;
}

GthreeAttribute *
gthree_attribute_new_from_uint32 (const char           *name,
                                  guint32              *data,
                                  int                   count,
                                  int                   item_size)
{
  GthreeAttribute *attribute;
  GthreeAttributeArray *array = gthree_attribute_array_new_from_uint32 (data, count, item_size);
  attribute = gthree_attribute_new_with_array (name, array, FALSE);
  gthree_attribute_array_unref (array);
  return attribute;
}

static void
gthree_attribute_init (GthreeAttribute *attribute)
{
}

static void
gthree_attribute_finalize (GObject *obj)
{
  //GthreeAttribute *attribute = GTHREE_ATTRIBUTE (obj);

  G_OBJECT_CLASS (gthree_attribute_parent_class)->finalize (obj);
}

static void
gthree_attribute_set_property (GObject *obj,
                             guint prop_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  //GthreeAttribute *attribute = GTHREE_ATTRIBUTE (obj);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_attribute_get_property (GObject *obj,
                             guint prop_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  //GthreeAttribute *attribute = GTHREE_ATTRIBUTE (obj);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_attribute_class_init (GthreeAttributeClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeResourceClass *resource_class = GTHREE_RESOURCE_CLASS (klass);

  gobject_class->set_property = gthree_attribute_set_property;
  gobject_class->get_property = gthree_attribute_get_property;
  gobject_class->finalize = gthree_attribute_finalize;

  resource_class->unrealize = gthree_attribute_real_unrealize;

  //g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}


const char *
gthree_attribute_get_name (GthreeAttribute *attribute)
{
  return attribute->name_intern;
}

GthreeAttributeArray *
gthree_attribute_get_array (GthreeAttribute *attribute)
{
  return attribute->array;
}

void
gthree_attribute_set_needs_update (GthreeAttribute *attribute)
{
  attribute->array->dirty = TRUE;
}

void
gthree_attribute_set_array (GthreeAttribute      *attribute,
                            GthreeAttributeArray *array)
{
  if (array)
    gthree_attribute_array_ref (array);
  if (attribute->array)
    gthree_attribute_array_unref (attribute->array);
  attribute->array = array;
}

int
gthree_attribute_get_count (GthreeAttribute *attribute)
{
  return attribute->count;
}

GthreeAttributeType
gthree_attribute_get_attribute_type (GthreeAttribute *attribute)
{
  g_assert (attribute->array);
  return gthree_attribute_array_get_attribute_type (attribute->array);
}

int
gthree_attribute_get_stride (GthreeAttribute *attribute)
{
  return attribute->array->stride;
}

int
gthree_attribute_get_item_size (GthreeAttribute *attribute)
{
  return attribute->item_size;
}

int
gthree_attribute_get_item_offset (GthreeAttribute *attribute)
{
  return attribute->item_offset;
}

gboolean
gthree_attribute_get_normalized (GthreeAttribute *attribute)
{
  return attribute->normalized;
}

gboolean
gthree_attribute_get_dynamic (GthreeAttribute *attribute)
{
  return attribute->array->dynamic;
}

void
gthree_attribute_set_dynamic (GthreeAttribute *attribute,
                              gboolean dynamic)
{
  attribute->array->dynamic = !!dynamic;
}

void
gthree_attribute_copy_at (GthreeAttribute      *attribute,
                          guint                 index,
                          GthreeAttribute      *source,
                          guint                 source_index,
                          guint                 n_items)
{
  gthree_attribute_array_copy_at (attribute->array, index, attribute->item_offset,
                                  source->array, source_index, source->item_offset,
                                  attribute->item_size, n_items);
}

static void
gthree_attribute_real_unrealize (GthreeResource *resource)
{
  GthreeAttribute *attribute = GTHREE_ATTRIBUTE (resource);

  gthree_attribute_array_destroy_buffer (attribute->array);
}

guint8 *
gthree_attribute_peek_uint8 (GthreeAttribute *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_uint8_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

guint8 *
gthree_attribute_peek_uint8_at (GthreeAttribute *attribute,
                                int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_uint8_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

gint8 *
gthree_attribute_peek_int8 (GthreeAttribute  *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_int8_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

gint8 *
gthree_attribute_peek_int8_at (GthreeAttribute *attribute,
                               int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_int8_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

gint16 *
gthree_attribute_peek_int16 (GthreeAttribute *attribute)
  {
  if (attribute->array)
    return gthree_attribute_array_peek_int16_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

gint16 *
gthree_attribute_peek_int16_at (GthreeAttribute *attribute,
                                int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_int16_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

guint16 *
gthree_attribute_peek_uint16 (GthreeAttribute *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_uint16_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

guint16 *
gthree_attribute_peek_uint16_at (GthreeAttribute *attribute,
                                  int             index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_uint16_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

gint32 *
gthree_attribute_peek_int32 (GthreeAttribute *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_int32_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

gint32 *
gthree_attribute_peek_int32_at (GthreeAttribute *attribute,
                                int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_int32_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

guint32 *
gthree_attribute_peek_uint32 (GthreeAttribute *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_uint32_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

guint32 *
gthree_attribute_peek_uint32_at (GthreeAttribute *attribute,
                                 int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_uint32_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

float *
gthree_attribute_peek_float (GthreeAttribute *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_float_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

float *
gthree_attribute_peek_float_at (GthreeAttribute *attribute,
                                int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_float_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

double *
gthree_attribute_peek_double (GthreeAttribute  *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_double_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

double *
gthree_attribute_peek_double_at (GthreeAttribute *attribute,
                                 int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_double_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

graphene_point3d_t *
gthree_attribute_peek_point3d (GthreeAttribute *attribute)
{
  if (attribute->array)
    return gthree_attribute_array_peek_point3d_at (attribute->array, 0, attribute->item_offset);
  return NULL;
}

graphene_point3d_t *
gthree_attribute_peek_point3d_at (GthreeAttribute *attribute,
                                  int              index)
{
  if (attribute->array)
    return gthree_attribute_array_peek_point3d_at (attribute->array, index, attribute->item_offset);
  return NULL;
}

void
gthree_attribute_set_x  (GthreeAttribute      *attribute,
                         guint                 index,
                         float                 x)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_x  (attribute->array, index, attribute->item_offset, x);
}

void
gthree_attribute_set_y (GthreeAttribute      *attribute,
                        guint                 index,
                        float                 y)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_y  (attribute->array, index, attribute->item_offset, y);
}

void
gthree_attribute_set_z (GthreeAttribute      *attribute,
                        guint                 index,
                        float                 z)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_z  (attribute->array, index, attribute->item_offset, z);
}

void
gthree_attribute_set_w (GthreeAttribute      *attribute,
                        guint                 index,
                        float                 w)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_w (attribute->array, index, attribute->item_offset, w);
}

void
gthree_attribute_set_xy (GthreeAttribute      *attribute,
                         guint                 index,
                         float                 x,
                         float                 y)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_xy  (attribute->array, index, attribute->item_offset, x, y);
}

void
gthree_attribute_set_xyz (GthreeAttribute      *attribute,
                          guint                 index,
                          float                 x,
                          float                 y,
                          float                 z)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_xyz  (attribute->array, index, attribute->item_offset, x, y, z);
}

void
gthree_attribute_get_xyz (GthreeAttribute      *attribute,
                          guint                 index,
                          float                 *x,
                          float                 *y,
                          float                 *z)
{
  g_assert (attribute->array);
  gthree_attribute_array_get_xyz (attribute->array, index, attribute->item_offset, x, y, z);
}

void
gthree_attribute_set_xyzw (GthreeAttribute      *attribute,
                           guint                 index,
                           float                 x,
                           float                 y,
                           float                 z,
                           float                 w)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_xyzw  (attribute->array, index, attribute->item_offset, x, y, z, w);
}

void
gthree_attribute_get_xyzw (GthreeAttribute      *attribute,
                           guint                 index,
                           float                 *x,
                           float                 *y,
                           float                 *z,
                           float                 *w)
{
  g_assert (attribute->array);
  gthree_attribute_array_get_xyzw  (attribute->array, index, attribute->item_offset, x, y, z, w);
}

void
gthree_attribute_set_point3d (GthreeAttribute      *attribute,
                              guint                 index,
                              graphene_point3d_t   *point)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_point3d  (attribute->array, index, attribute->item_offset, point);
}

void
gthree_attribute_set_vec2 (GthreeAttribute      *attribute,
                           guint                 index,
                           graphene_vec2_t      *vec2)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_vec2  (attribute->array, index, attribute->item_offset, vec2);
}

void
gthree_attribute_set_vec3 (GthreeAttribute      *attribute,
                           guint                 index,
                           graphene_vec3_t      *vec3)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_vec3 (attribute->array, index, attribute->item_offset, vec3);
}

void
gthree_attribute_set_vec4 (GthreeAttribute      *attribute,
                           guint                 index,
                           graphene_vec4_t      *vec4)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_vec4  (attribute->array, index, attribute->item_offset, vec4);
}

void
gthree_attribute_get_vec4 (GthreeAttribute      *attribute,
                           guint                 index,
                           graphene_vec4_t      *vec4)
{
  g_assert (attribute->array);
  gthree_attribute_array_get_vec4  (attribute->array, index, attribute->item_offset, vec4);
}

void
gthree_attribute_get_matrix (GthreeAttribute      *attribute,
                             guint                 index,
                             graphene_matrix_t    *matrix)
{
  g_assert (attribute->array);
  gthree_attribute_array_get_matrix  (attribute->array, index, attribute->item_offset, matrix);
}

void
gthree_attribute_set_rgb (GthreeAttribute      *attribute,
                          guint                 index,
                          GdkRGBA              *color)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_rgb  (attribute->array, index, attribute->item_offset, color);
}

void
gthree_attribute_set_rgba (GthreeAttribute      *attribute,
                           guint                 index,
                           GdkRGBA              *color)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_rgba  (attribute->array, index, attribute->item_offset, color);
}

void
gthree_attribute_set_uint8  (GthreeAttribute      *attribute,
                             guint                 index,
                             gint8                 value)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_uint8  (attribute->array, index, attribute->item_offset, value);
}

guint8
gthree_attribute_get_uint8 (GthreeAttribute      *attribute,
                            guint                 index)
{
  g_assert (attribute->array);
  return gthree_attribute_array_get_uint8  (attribute->array, index, attribute->item_offset);
}

void
gthree_attribute_set_uint16 (GthreeAttribute      *attribute,
                             guint                 index,
                             gint16                value)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_uint16  (attribute->array, index, attribute->item_offset, value);
}

guint16
gthree_attribute_get_uint16 (GthreeAttribute      *attribute,
                             guint                 index)
{
  g_assert (attribute->array);
  return gthree_attribute_array_get_uint16  (attribute->array, index, attribute->item_offset);
}

void
gthree_attribute_set_uint32 (GthreeAttribute      *attribute,
                             guint                 index,
                             guint32               value)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_uint32  (attribute->array, index, attribute->item_offset, value);
}

guint32
gthree_attribute_get_uint32 (GthreeAttribute      *attribute,
                             guint                 index)
{
  g_assert (attribute->array);
  return gthree_attribute_array_get_uint32  (attribute->array, index, attribute->item_offset);
}

void
gthree_attribute_set_uint (GthreeAttribute      *attribute,
                           guint                 index,
                           guint                 value)
{
  g_assert (attribute->array);
  gthree_attribute_array_set_uint  (attribute->array, index, attribute->item_offset, value);
}

guint
gthree_attribute_get_uint (GthreeAttribute      *attribute,
                           guint                 index)
{
  g_assert (attribute->array);
  return gthree_attribute_array_get_uint  (attribute->array, index, attribute->item_offset);
}

void
gthree_attribute_get_point3d (GthreeAttribute      *attribute,
                              guint                 index,
                              graphene_point3d_t   *point)
{
  gthree_attribute_array_get_point3d (attribute->array, index, attribute->item_offset, point);
}


void
gthree_attribute_update (GthreeAttribute *attribute, gint buffer_type)
{
  if (attribute->array->gl_buffer == 0)
    {
      gthree_resource_set_realized_for (GTHREE_RESOURCE (attribute), gdk_gl_context_get_current ());
      gthree_attribute_array_create_buffer (attribute->array, buffer_type);
    }
  else if (attribute->array->dirty)
    gthree_attribute_array_update_buffer (attribute->array, buffer_type);
}

int
gthree_attribute_get_gl_buffer (GthreeAttribute *attribute)
{
  return attribute->array->gl_buffer;
}

int
gthree_attribute_get_gl_type (GthreeAttribute *attribute)
{
  return attribute_type_gl[attribute->array->type];
}

int
gthree_attribute_get_gl_bytes_per_element (GthreeAttribute *attribute)
{
  return attribute_type_size[attribute->array->type];
}


static struct {
  char *name;
  GthreeAttributeType type;
} attribute_type_names[] = {
  { "Int8Array", GTHREE_ATTRIBUTE_TYPE_INT8},
  { "Uint8Array", GTHREE_ATTRIBUTE_TYPE_UINT8},
  { "Int16Array", GTHREE_ATTRIBUTE_TYPE_INT16},
  { "Uint16Array", GTHREE_ATTRIBUTE_TYPE_UINT16},
  { "Int132rray", GTHREE_ATTRIBUTE_TYPE_INT32},
  { "Uint132rray", GTHREE_ATTRIBUTE_TYPE_UINT32},
  { "Float32Array", GTHREE_ATTRIBUTE_TYPE_FLOAT},
  { "Float64Array", GTHREE_ATTRIBUTE_TYPE_DOUBLE},
};

GthreeAttribute *
gthree_attribute_parse_json (JsonObject *attr,
                             const char *name)
{
  gint64 item_size;
  const gchar *typename;
  int type = -1;
  g_autoptr(GthreeAttribute) attribute = NULL;
  JsonArray *array;
  gboolean normalized = FALSE;
  int i, len;

  item_size = json_object_get_int_member (attr, "itemSize");

  typename = json_object_get_string_member (attr, "type");
  for (i = 0; i < G_N_ELEMENTS(attribute_type_names); i++)
    {
      if (strcmp (typename, attribute_type_names[i].name) == 0)
        {
          type = attribute_type_names[i].type;
          break;
        }
    }
  g_assert (type != -1);

  item_size = json_object_get_int_member (attr, "itemSize");
  if (json_object_has_member (attr, "normalized"))
    normalized = json_object_get_boolean_member (attr, "normalized");

  array = json_object_get_array_member (attr, "array");
  len = json_array_get_length (array);


  attribute = gthree_attribute_new (name, type, len / item_size, item_size, normalized);

  switch (type)
    {
    case GTHREE_ATTRIBUTE_TYPE_INT8:
      {
        gint8 *p = gthree_attribute_peek_int8 (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_int_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT8:
      {
        guint8 *p = gthree_attribute_peek_uint8 (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_int_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT16:
      {
        gint16 *p = gthree_attribute_peek_int16 (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_int_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT16:
      {
        guint16 *p = gthree_attribute_peek_uint16 (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_int_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_INT32:
      {
        gint32 *p = gthree_attribute_peek_int32 (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_int_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_UINT32:
      {
        guint32 *p = gthree_attribute_peek_uint32 (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_int_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_FLOAT:
      {
        float *p = gthree_attribute_peek_float (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_double_element (array, i);
        break;
      }
    case GTHREE_ATTRIBUTE_TYPE_DOUBLE:
      {
        double *p = gthree_attribute_peek_double (attribute);
        for (i = 0; i < len; i++)
          p[i] = json_array_get_double_element (array, i);
        break;
      }
    }
  return g_steal_pointer (&attribute);
}
