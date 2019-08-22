#ifndef __GTHREE_ATTRIBUTE_H__
#define __GTHREE_ATTRIBUTE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <glib-object.h>
#include <graphene.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeresource.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

GTHREE_API
int gthree_attribute_type_length (GthreeAttributeType type);

GTHREE_API
GthreeAttributeArray *gthree_attribute_array_new                (GthreeAttributeType   type,
                                                                 int                   count,
                                                                 int                   stride);
GTHREE_API
GthreeAttributeArray *gthree_attribute_array_new_from_float     (float                *data,
                                                                 int                   count,
                                                                 int                   item_size);
GTHREE_API
GthreeAttributeArray *gthree_attribute_array_new_from_uint16    (guint16              *data,
                                                                 int                   count,
                                                                 int                   item_size);
GTHREE_API
GthreeAttributeArray *gthree_attribute_array_new_from_uint32    (guint32              *data,
                                                                 int                   count,
                                                                 int                   item_size);
GTHREE_API
GthreeAttributeArray *gthree_attribute_array_reshape (GthreeAttributeArray *array,
                                                      guint                 index,
                                                      guint                 offset,
                                                      guint                 count,
                                                      guint                 item_size,
                                                      gboolean              share_if_possible);
GTHREE_API
GthreeAttributeArray *gthree_attribute_array_ref                (GthreeAttributeArray *array);
GTHREE_API
void                  gthree_attribute_array_unref              (GthreeAttributeArray *array);
GTHREE_API
GthreeAttributeType   gthree_attribute_array_get_attribute_type (GthreeAttributeArray *array);
GTHREE_API
int                   gthree_attribute_array_get_len            (GthreeAttributeArray *array);
GTHREE_API
int                   gthree_attribute_array_get_count          (GthreeAttributeArray *array);
GTHREE_API
int                   gthree_attribute_array_get_stride         (GthreeAttributeArray *array);
GTHREE_API
guint8 *              gthree_attribute_array_peek_uint8         (GthreeAttributeArray *array);
GTHREE_API
guint8 *              gthree_attribute_array_peek_uint8_at      (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
gint8 *               gthree_attribute_array_peek_int8          (GthreeAttributeArray *array);
GTHREE_API
gint8 *               gthree_attribute_array_peek_int8_at       (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
gint16 *              gthree_attribute_array_peek_int16         (GthreeAttributeArray *array);
GTHREE_API
gint16 *              gthree_attribute_array_peek_int16_at      (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
guint16 *             gthree_attribute_array_peek_uint16        (GthreeAttributeArray *array);
GTHREE_API
guint16 *             gthree_attribute_array_peek_uint16_at     (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
gint32 *              gthree_attribute_array_peek_int32         (GthreeAttributeArray *array);
GTHREE_API
gint32 *              gthree_attribute_array_peek_int32_at      (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
guint32 *             gthree_attribute_array_peek_uint32        (GthreeAttributeArray *array);
GTHREE_API
guint32 *             gthree_attribute_array_peek_uint32_at     (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
float *               gthree_attribute_array_peek_float         (GthreeAttributeArray *array);
GTHREE_API
float *               gthree_attribute_array_peek_float_at      (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
float                 gthree_attribute_array_get_float_at       (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
double *              gthree_attribute_array_peek_double        (GthreeAttributeArray *array);
GTHREE_API
double *              gthree_attribute_array_peek_double_at     (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
graphene_point3d_t *  gthree_attribute_array_peek_point3d       (GthreeAttributeArray *array);
GTHREE_API
graphene_point3d_t *  gthree_attribute_array_peek_point3d_at    (GthreeAttributeArray *array,
                                                                 int                   index,
                                                                 int                   offset);
GTHREE_API
void                  gthree_attribute_array_copy_at            (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 GthreeAttributeArray *source,
                                                                 guint                 source_index,
                                                                 guint                 source_offset,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
GTHREE_API
void                  gthree_attribute_array_copy_float         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                *source,
                                                                 guint                 source_stride,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
GTHREE_API
void                  gthree_attribute_array_copy_uint16        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint16              *source,
                                                                 guint                 source_stride,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
GTHREE_API
void                  gthree_attribute_array_copy_uint32        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint32              *source,
                                                                 guint                 source_stride,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
GTHREE_API
void                  gthree_attribute_array_set_x              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x);
GTHREE_API
void                  gthree_attribute_array_set_y              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 y);
GTHREE_API
void                  gthree_attribute_array_set_z              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 z);
GTHREE_API
void                  gthree_attribute_array_set_w              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 w);
GTHREE_API
void                  gthree_attribute_array_set_xy             (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x,
                                                                 float                 y);
GTHREE_API
void                  gthree_attribute_array_set_xyz            (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x,
                                                                 float                 y,
                                                                 float                 z);
GTHREE_API
void                  gthree_attribute_array_get_xyz            (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                *x,
                                                                 float                *y,
                                                                 float                *z);
GTHREE_API
void                  gthree_attribute_array_set_xyzw           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x,
                                                                 float                 y,
                                                                 float                 z,
                                                                 float                 w);
GTHREE_API
void                  gthree_attribute_array_get_xyzw           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                *x,
                                                                 float                *y,
                                                                 float                *z,
                                                                 float                *w);
GTHREE_API
void                  gthree_attribute_array_set_point3d        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_point3d_t   *point);
GTHREE_API
void                  gthree_attribute_array_set_vec2           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 const graphene_vec2_t *vec2);
GTHREE_API
void                  gthree_attribute_array_get_vec2           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_vec2_t      *vec2);
GTHREE_API
void                  gthree_attribute_array_set_vec3           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 const graphene_vec3_t *vec3);
GTHREE_API
void                  gthree_attribute_array_get_vec3           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_vec3_t      *vec3);
GTHREE_API
void                  gthree_attribute_array_set_vec4           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 const graphene_vec4_t *vec4);
GTHREE_API
void                  gthree_attribute_array_get_vec4           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_vec4_t      *vec4);
GTHREE_API
void                  gthree_attribute_array_get_matrix         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_matrix_t    *matrix);
GTHREE_API
void                  gthree_attribute_array_set_uint8          (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 gint8                 value);
GTHREE_API
guint8                gthree_attribute_array_get_uint8          (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
GTHREE_API
void                  gthree_attribute_array_set_uint16         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 gint16                value);
GTHREE_API
guint16               gthree_attribute_array_get_uint16         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
GTHREE_API
void                  gthree_attribute_array_set_uint32         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint32               value);
GTHREE_API
guint32               gthree_attribute_array_get_uint32         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
GTHREE_API
void                  gthree_attribute_array_set_uint           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint                 value);
GTHREE_API
guint                 gthree_attribute_array_get_uint           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
GTHREE_API
void                  gthree_attribute_array_get_point3d        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_point3d_t   *point);
GTHREE_API
void                  gthree_attribute_array_get_elements_as_float (GthreeAttributeArray *array,
                                                                    guint                 index,
                                                                    guint                 offset,
                                                                    float                *dest,
                                                                    guint                 n_elements);
GTHREE_API
void                  gthree_attribute_array_set_elements_from_float (GthreeAttributeArray *array,
                                                                      guint                 index,
                                                                      guint                 offset,
                                                                      float                *src,
                                                                      guint                 n_elements);


#define GTHREE_TYPE_ATTRIBUTE      (gthree_attribute_get_type ())
#define GTHREE_ATTRIBUTE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_ATTRIBUTE, GthreeAttribute))
#define GTHREE_ATTRIBUTE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_ATTRIBUTE, GthreeAttributeClass))
#define GTHREE_IS_ATTRIBUTE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_ATTRIBUTE))
#define GTHREE_IS_ATTRIBUTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_ATTRIBUTE))
#define GTHREE_ATTRIBUTE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_ATTRIBUTE, GthreeAttributeClass))

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAttributeArray, gthree_attribute_array_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAttribute, g_object_unref)

GTHREE_API
GType gthree_attribute_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeAttribute *gthree_attribute_new                        (const char           *name,
                                                              GthreeAttributeType   type,
                                                              int                   count,
                                                              int                   item_size,
                                                              gboolean              normalized);
GTHREE_API
GthreeAttribute *gthree_attribute_copy                       (const char           *name,
                                                              GthreeAttribute      *source);
GTHREE_API
GthreeAttribute *gthree_attribute_new_from_float             (const char           *name,
                                                              float                *data,
                                                              int                   count,
                                                              int                   item_size);
GTHREE_API
GthreeAttribute *gthree_attribute_new_from_uint32            (const char           *name,
                                                              guint32              *data,
                                                              int                   count,
                                                              int                   item_size);
GTHREE_API
GthreeAttribute *gthree_attribute_new_from_uint16            (const char           *name,
                                                              guint16              *data,
                                                              int                   count,
                                                              int                   item_size);
GTHREE_API
GthreeAttribute *gthree_attribute_new_with_array             (const char           *name,
                                                              GthreeAttributeArray *array,
                                                              gboolean              normalized);
GTHREE_API
GthreeAttribute *gthree_attribute_new_with_array_interleaved (const char           *name,
                                                              GthreeAttributeArray *array,
                                                              gboolean              normalized,
                                                              int                   item_size,
                                                              int                   item_offset,
                                                              int                   count);
GTHREE_API
GthreeAttribute *gthree_attribute_parse_json                 (JsonObject           *root,
                                                              const char           *name);

GTHREE_API
const char *          gthree_attribute_get_name           (GthreeAttribute      *attribute);
GTHREE_API
GthreeAttributeArray *gthree_attribute_get_array          (GthreeAttribute      *attribute);
GTHREE_API
void                  gthree_attribute_set_array          (GthreeAttribute      *attribute,
                                                           GthreeAttributeArray *array);
GTHREE_API
void                  gthree_attribute_set_needs_update   (GthreeAttribute      *attribute);
GTHREE_API
int                   gthree_attribute_get_count          (GthreeAttribute      *attribute);
GTHREE_API
GthreeAttributeType   gthree_attribute_get_attribute_type (GthreeAttribute      *attribute);
GTHREE_API
int                   gthree_attribute_get_stride         (GthreeAttribute      *attribute);
GTHREE_API
int                   gthree_attribute_get_item_size      (GthreeAttribute      *attribute);
GTHREE_API
int                   gthree_attribute_get_item_offset    (GthreeAttribute      *attribute);
GTHREE_API
gboolean              gthree_attribute_get_normalized     (GthreeAttribute      *attribute);
GTHREE_API
gboolean              gthree_attribute_get_dynamic        (GthreeAttribute      *attribute);
GTHREE_API
void                  gthree_attribute_set_dynamic        (GthreeAttribute      *attribute,
                                                           gboolean              dynamic);
GTHREE_API
void                  gthree_attribute_copy_at            (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           GthreeAttribute      *source,
                                                           guint                 source_index,
                                                           guint                 n_items);
GTHREE_API
void                  gthree_attribute_update             (GthreeAttribute      *attribute,
                                                           int                   buffer_type);
GTHREE_API
guint8 *              gthree_attribute_peek_uint8         (GthreeAttribute      *attribute);
GTHREE_API
guint8 *              gthree_attribute_peek_uint8_at      (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
gint8 *               gthree_attribute_peek_int8          (GthreeAttribute      *attribute);
GTHREE_API
gint8 *               gthree_attribute_peek_int8_at       (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
gint16 *              gthree_attribute_peek_int16         (GthreeAttribute      *attribute);
GTHREE_API
gint16 *              gthree_attribute_peek_int16_at      (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
guint16 *             gthree_attribute_peek_uint16        (GthreeAttribute      *attribute);
GTHREE_API
guint16 *             gthree_attribute_peek_uint16_at     (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
gint32 *              gthree_attribute_peek_int32         (GthreeAttribute      *attribute);
GTHREE_API
gint32 *              gthree_attribute_peek_int32_at      (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
guint32 *             gthree_attribute_peek_uint32        (GthreeAttribute      *attribute);
GTHREE_API
guint32 *             gthree_attribute_peek_uint32_at     (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
float *               gthree_attribute_peek_float         (GthreeAttribute      *attribute);
GTHREE_API
float *               gthree_attribute_peek_float_at      (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
double *              gthree_attribute_peek_double        (GthreeAttribute      *attribute);
GTHREE_API
double *              gthree_attribute_peek_double_at     (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
graphene_point3d_t *  gthree_attribute_peek_point3d       (GthreeAttribute      *attribute);
GTHREE_API
graphene_point3d_t *  gthree_attribute_peek_point3d_at    (GthreeAttribute      *attribute,
                                                           int                   index);
GTHREE_API
void                  gthree_attribute_set_x              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x);
GTHREE_API
void                  gthree_attribute_set_y              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 y);
GTHREE_API
void                  gthree_attribute_set_z              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 z);
GTHREE_API
void                  gthree_attribute_set_w              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 w);
GTHREE_API
void                  gthree_attribute_set_xy             (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x,
                                                           float                 y);
GTHREE_API
void                  gthree_attribute_set_xyz            (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x,
                                                           float                 y,
                                                           float                 z);
GTHREE_API
void                  gthree_attribute_get_xyz            (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                *x,
                                                           float                *y,
                                                           float                *z);
GTHREE_API
void                  gthree_attribute_set_xyzw           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x,
                                                           float                 y,
                                                           float                 z,
                                                           float                 w);
GTHREE_API
void                  gthree_attribute_get_xyzw           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                *x,
                                                           float                *y,
                                                           float                *z,
                                                           float                *w);
GTHREE_API
void                  gthree_attribute_set_point3d        (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_point3d_t   *point);
GTHREE_API
void                  gthree_attribute_set_vec2           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           const graphene_vec2_t *vec2);
GTHREE_API
void                  gthree_attribute_get_vec2           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_vec2_t      *vec2);
GTHREE_API
void                  gthree_attribute_set_vec3           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           const graphene_vec3_t *vec3);
GTHREE_API
void                  gthree_attribute_get_vec3           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_vec3_t      *vec3);
GTHREE_API
void                  gthree_attribute_set_vec4           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           const graphene_vec4_t *vec4);
GTHREE_API
void                  gthree_attribute_get_vec4           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_vec4_t      *vec4);
GTHREE_API
void                  gthree_attribute_get_matrix         (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_matrix_t    *matrix);
GTHREE_API
void                  gthree_attribute_set_uint8          (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           gint8                 value);
GTHREE_API
guint8                gthree_attribute_get_uint8          (GthreeAttribute      *attribute,
                                                           guint                 index);
GTHREE_API
void                  gthree_attribute_set_uint16         (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           gint16                value);
GTHREE_API
guint16               gthree_attribute_get_uint16         (GthreeAttribute      *attribute,
                                                           guint                 index);
GTHREE_API
void                  gthree_attribute_set_uint32         (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           guint32               value);
GTHREE_API
guint32               gthree_attribute_get_uint32         (GthreeAttribute      *attribute,
                                                           guint                 index);
GTHREE_API
void                  gthree_attribute_set_uint           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           guint                 value);
GTHREE_API
guint                 gthree_attribute_get_uint           (GthreeAttribute      *attribute,
                                                           guint                 index);
GTHREE_API
void                  gthree_attribute_get_point3d        (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_point3d_t   *point);



/* These are valid when realized */
GTHREE_API
int gthree_attribute_get_gl_buffer            (GthreeAttribute *attribute);
GTHREE_API
int gthree_attribute_get_gl_type              (GthreeAttribute *attribute);
GTHREE_API
int gthree_attribute_get_gl_bytes_per_element (GthreeAttribute *attribute);


G_END_DECLS

#endif /* __GTHREE_ATTRIBUTE_H__ */
