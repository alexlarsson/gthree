#ifndef __GTHREE_ATTRIBUTE_H__
#define __GTHREE_ATTRIBUTE_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <glib-object.h>
#include <graphene.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeresource.h>

G_BEGIN_DECLS

GthreeAttributeArray *gthree_attribute_array_new                (GthreeAttributeType   type,
                                                                 int                   count,
                                                                 int                   stride);
GthreeAttributeArray *gthree_attribute_array_new_from_float     (float                *data,
                                                                 int                   count,
                                                                 int                   item_size);
GthreeAttributeArray *gthree_attribute_array_new_from_uint16    (guint16              *data,
                                                                 int                   count,
                                                                 int                   item_size);
GthreeAttributeArray *gthree_attribute_array_new_from_uint32    (guint32              *data,
                                                                 int                   count,
                                                                 int                   item_size);
GthreeAttributeArray *gthree_attribute_array_ref                (GthreeAttributeArray *array);
void                  gthree_attribute_array_unref              (GthreeAttributeArray *array);
GthreeAttributeType   gthree_attribute_array_get_attribute_type (GthreeAttributeArray *array);
int                   gthree_attribute_array_get_len            (GthreeAttributeArray *array);
int                   gthree_attribute_array_get_count          (GthreeAttributeArray *array);
int                   gthree_attribute_array_get_stride         (GthreeAttributeArray *array);
guint8 *              gthree_attribute_array_peek_uint8         (GthreeAttributeArray *array);
guint8 *              gthree_attribute_array_peek_uint8_at      (GthreeAttributeArray *array,
                                                                 int                   index);
gint8 *               gthree_attribute_array_peek_int8          (GthreeAttributeArray *array);
gint8 *               gthree_attribute_array_peek_int8_at       (GthreeAttributeArray *array,
                                                                 int                   index);
gint16 *              gthree_attribute_array_peek_int16         (GthreeAttributeArray *array);
gint16 *              gthree_attribute_array_peek_int16_at      (GthreeAttributeArray *array,
                                                                 int                   index);
guint16 *             gthree_attribute_array_peek_uint16        (GthreeAttributeArray *array);
guint16 *             gthree_attribute_array_peek_uint16_at     (GthreeAttributeArray *array,
                                                                 int                   index);
gint32 *              gthree_attribute_array_peek_int32         (GthreeAttributeArray *array);
gint32 *              gthree_attribute_array_peek_int32_at      (GthreeAttributeArray *array,
                                                                 int                   index);
guint32 *             gthree_attribute_array_peek_uint32        (GthreeAttributeArray *array);
guint32 *             gthree_attribute_array_peek_uint32_at     (GthreeAttributeArray *array,
                                                                 int                   index);
float *               gthree_attribute_array_peek_float         (GthreeAttributeArray *array);
float *               gthree_attribute_array_peek_float_at      (GthreeAttributeArray *array,
                                                                 int                   index);
double *              gthree_attribute_array_peek_double        (GthreeAttributeArray *array);
double *              gthree_attribute_array_peek_double_at     (GthreeAttributeArray *array,
                                                                 int                   index);
graphene_point3d_t *  gthree_attribute_array_peek_point3d       (GthreeAttributeArray *array);
graphene_point3d_t *  gthree_attribute_array_peek_point3d_at    (GthreeAttributeArray *array,
                                                                 int                   index);
void                  gthree_attribute_array_copy_at            (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 GthreeAttributeArray *source,
                                                                 guint                 source_index,
                                                                 guint                 source_offset,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
void                  gthree_attribute_array_copy_float         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                *source,
                                                                 guint                 source_stride,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
void                  gthree_attribute_array_copy_uint16        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint16              *source,
                                                                 guint                 source_stride,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
void                  gthree_attribute_array_copy_uint32        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint32              *source,
                                                                 guint                 source_stride,
                                                                 guint                 n_elements,
                                                                 guint                 n_items);
void                  gthree_attribute_array_set_x              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x);
void                  gthree_attribute_array_set_y              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 y);
void                  gthree_attribute_array_set_z              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 z);
void                  gthree_attribute_array_set_w              (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 w);
void                  gthree_attribute_array_set_xy             (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x,
                                                                 float                 y);
void                  gthree_attribute_array_set_xyz            (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x,
                                                                 float                 y,
                                                                 float                 z);
void                  gthree_attribute_array_set_xyzw           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 float                 x,
                                                                 float                 y,
                                                                 float                 z,
                                                                 float                 w);
void                  gthree_attribute_array_set_point3d        (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_point3d_t   *point);
void                  gthree_attribute_array_set_vec2           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_vec2_t      *vec2);
void                  gthree_attribute_array_set_vec3           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_vec3_t      *vec3);
void                  gthree_attribute_array_set_vec4           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 graphene_vec4_t      *vec4);
void                  gthree_attribute_array_set_rgb            (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 GdkRGBA              *color);
void                  gthree_attribute_array_set_rgba           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 GdkRGBA              *color);
void                  gthree_attribute_array_set_uint8          (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 gint8                 value);
guint8                gthree_attribute_array_get_uint8          (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
void                  gthree_attribute_array_set_uint16         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 gint16                value);
guint16               gthree_attribute_array_get_uint16         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
void                  gthree_attribute_array_set_uint32         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint32               value);
guint32               gthree_attribute_array_get_uint32         (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);
void                  gthree_attribute_array_set_uint           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset,
                                                                 guint                 value);
guint                 gthree_attribute_array_get_uint           (GthreeAttributeArray *array,
                                                                 guint                 index,
                                                                 guint                 offset);


/* Some pre-defined values for attribute names */
#define GTHREE_ATTRIBUTE_NAME_POSITION      0
#define GTHREE_ATTRIBUTE_NAME_COLOR         1
#define GTHREE_ATTRIBUTE_NAME_NORMAL        2
#define GTHREE_ATTRIBUTE_NAME_UV            3
#define GTHREE_ATTRIBUTE_NAME_UV2           4
#define GTHREE_ATTRIBUTE_NAME_SKIN_INDEX    5
#define GTHREE_ATTRIBUTE_NAME_SKIN_WEIGHT   6
#define GTHREE_ATTRIBUTE_NAME_LINE_DISTANCE 7
#define GTHREE_ATTRIBUTE_NAME_INDEX         8

GthreeAttributeName gthree_attribute_name_get_for_static (const char          *string);
GthreeAttributeName gthree_attribute_name_get            (const char          *string);
const char *        gthree_attribute_name_to_string      (GthreeAttributeName  name);

#define GTHREE_TYPE_ATTRIBUTE      (gthree_attribute_get_type ())
#define GTHREE_ATTRIBUTE(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_ATTRIBUTE, GthreeAttribute))
#define GTHREE_ATTRIBUTE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_ATTRIBUTE, GthreeAttributeClass))
#define GTHREE_IS_ATTRIBUTE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_ATTRIBUTE))
#define GTHREE_IS_ATTRIBUTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_ATTRIBUTE))
#define GTHREE_ATTRIBUTE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_ATTRIBUTE, GthreeAttributeClass))

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAttribute, g_object_unref)

GType gthree_attribute_get_type (void) G_GNUC_CONST;

GthreeAttribute *gthree_attribute_new                        (const char           *name,
                                                              GthreeAttributeType   type,
                                                              int                   count,
                                                              int                   item_size,
                                                              gboolean              normalized);
GthreeAttribute *gthree_attribute_new_from_float             (const char           *name,
                                                              float                *data,
                                                              int                   count,
                                                              int                   item_size);
GthreeAttribute *gthree_attribute_new_from_uint32            (const char           *name,
                                                              guint32              *data,
                                                              int                   count,
                                                              int                   item_size);
GthreeAttribute *gthree_attribute_new_from_uint16            (const char           *name,
                                                              guint16              *data,
                                                              int                   count,
                                                              int                   item_size);
GthreeAttribute *gthree_attribute_new_with_array             (const char           *name,
                                                              GthreeAttributeArray *array,
                                                              gboolean              normalized);
GthreeAttribute *gthree_attribute_new_with_array_interleaved (const char           *name,
                                                              GthreeAttributeArray *array,
                                                              gboolean              normalized,
                                                              int                   item_size,
                                                              int                   item_offset);

GthreeAttributeName   gthree_attribute_get_name           (GthreeAttribute      *attribute);
GthreeAttributeArray *gthree_attribute_get_array          (GthreeAttribute      *attribute);
void                  gthree_attribute_set_array          (GthreeAttribute      *attribute,
                                                           GthreeAttributeArray *array);
void                  gthree_attribute_set_needs_update   (GthreeAttribute      *attribute);
int                   gthree_attribute_get_count          (GthreeAttribute      *attribute);
GthreeAttributeType   gthree_attribute_get_attribute_type (GthreeAttribute      *attribute);
int                   gthree_attribute_get_item_size      (GthreeAttribute      *attribute);
int                   gthree_attribute_get_item_offset    (GthreeAttribute      *attribute);
gboolean              gthree_attribute_get_normalized     (GthreeAttribute      *attribute);
gboolean              gthree_attribute_get_dynamic        (GthreeAttribute      *attribute);
void                  gthree_attribute_set_dynamic        (GthreeAttribute      *attribute,
                                                           gboolean              dynamic);
void                  gthree_attribute_copy_at            (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           GthreeAttribute      *source,
                                                           guint                 source_index,
                                                           guint                 n_items);
void                  gthree_attribute_update             (GthreeAttribute      *attribute,
                                                           int                   buffer_type);
guint8 *              gthree_attribute_peek_uint8         (GthreeAttribute      *attribute);
guint8 *              gthree_attribute_peek_uint8_at      (GthreeAttribute      *attribute,
                                                           int                   index);
gint8 *               gthree_attribute_peek_int8          (GthreeAttribute      *attribute);
gint8 *               gthree_attribute_peek_int8_at       (GthreeAttribute      *attribute,
                                                           int                   index);
gint16 *              gthree_attribute_peek_int16         (GthreeAttribute      *attribute);
gint16 *              gthree_attribute_peek_int16_at      (GthreeAttribute      *attribute,
                                                           int                   index);
guint16 *             gthree_attribute_peek_uint16        (GthreeAttribute      *attribute);
guint16 *             gthree_attribute_peek_uint16_at     (GthreeAttribute      *attribute,
                                                           int                   index);
gint32 *              gthree_attribute_peek_int32         (GthreeAttribute      *attribute);
gint32 *              gthree_attribute_peek_int32_at      (GthreeAttribute      *attribute,
                                                           int                   index);
guint32 *             gthree_attribute_peek_uint32        (GthreeAttribute      *attribute);
guint32 *             gthree_attribute_peek_uint32_at     (GthreeAttribute      *attribute,
                                                           int                   index);
float *               gthree_attribute_peek_float         (GthreeAttribute      *attribute);
float *               gthree_attribute_peek_float_at      (GthreeAttribute      *attribute,
                                                           int                   index);
double *              gthree_attribute_peek_double        (GthreeAttribute      *attribute);
double *              gthree_attribute_peek_double_at     (GthreeAttribute      *attribute,
                                                           int                   index);
graphene_point3d_t *  gthree_attribute_peek_point3d       (GthreeAttribute      *attribute);
graphene_point3d_t *  gthree_attribute_peek_point3d_at    (GthreeAttribute      *attribute,
                                                           int                   index);
void                  gthree_attribute_set_x              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x);
void                  gthree_attribute_set_y              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 y);
void                  gthree_attribute_set_z              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 z);
void                  gthree_attribute_set_w              (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 w);
void                  gthree_attribute_set_xy             (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x,
                                                           float                 y);
void                  gthree_attribute_set_xyz            (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x,
                                                           float                 y,
                                                           float                 z);
void                  gthree_attribute_set_xyzw           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           float                 x,
                                                           float                 y,
                                                           float                 z,
                                                           float                 w);
void                  gthree_attribute_set_point3d        (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_point3d_t   *point);
void                  gthree_attribute_set_vec2           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_vec2_t      *vec2);
void                  gthree_attribute_set_vec3           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_vec3_t      *vec3);
void                  gthree_attribute_set_vec4           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           graphene_vec4_t      *vec4);
void                  gthree_attribute_set_rgb            (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           GdkRGBA              *color);
void                  gthree_attribute_set_rgba           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           GdkRGBA              *color);
void                  gthree_attribute_set_uint8          (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           gint8                 value);
guint8                gthree_attribute_get_uint8          (GthreeAttribute      *attribute,
                                                           guint                 index);
void                  gthree_attribute_set_uint16         (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           gint16                value);
guint16               gthree_attribute_get_uint16         (GthreeAttribute      *attribute,
                                                           guint                 index);
void                  gthree_attribute_set_uint32         (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           guint32               value);
guint32               gthree_attribute_get_uint32         (GthreeAttribute      *attribute,
                                                           guint                 index);
void                  gthree_attribute_set_uint           (GthreeAttribute      *attribute,
                                                           guint                 index,
                                                           guint                 value);
guint                 gthree_attribute_get_uint           (GthreeAttribute      *attribute,
                                                           guint                 index);



/* These are valid when realized */
int gthree_attribute_get_gl_buffer            (GthreeAttribute *attribute);
int gthree_attribute_get_gl_type              (GthreeAttribute *attribute);
int gthree_attribute_get_gl_bytes_per_element (GthreeAttribute *attribute);


G_END_DECLS

#endif /* __GTHREE_ATTRIBUTE_H__ */
