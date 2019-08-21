#ifndef __GTHREE_OBJECT_H__
#define __GTHREE_OBJECT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <glib-object.h>
#include <graphene.h>

#include <gthree/gthreetypes.h>
#include <gthree/gthreeenums.h>

G_BEGIN_DECLS

#define GTHREE_PRIVATE_FIELD(x)        x

#define GTHREE_TYPE_OBJECT            (gthree_object_get_type ())
#define GTHREE_OBJECT(inst)           (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_OBJECT,  GthreeObject))
#define GTHREE_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_OBJECT, GthreeObjectClass))
#define GTHREE_IS_OBJECT(inst)        (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_OBJECT))
#define GTHREE_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTHREE_TYPE_OBJECT))
#define GTHREE_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTHREE_TYPE_OBJECT, GthreeObjectClass))

typedef struct {
  GObject parent;
} GthreeObject;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeObject, g_object_unref)

typedef struct {
  GObjectClass parent_class;

  gboolean (* in_frustum)       (GthreeObject             *object,
                                 const graphene_frustum_t *frustum);

  void (* parent_set)           (GthreeObject          *object,
                                 GthreeObject          *old_parent);

  void (* update)               (GthreeObject          *object);
  void (* destroy)              (GthreeObject          *object);
  gboolean (* update_matrix_world) (GthreeObject          *object,
                                    gboolean               force);


  void (* fill_render_list)      (GthreeObject          *object,
                                  GthreeRenderList      *list);
  void (* set_direct_uniforms)   (GthreeObject          *object,
                                  GthreeProgram         *program,
                                  GthreeRenderer *renderer);
  void (* raycast)               (GthreeObject          *object,
                                  GthreeRaycaster       *raycaster,
                                  GPtrArray             *intersections);
} GthreeObjectClass;

GTHREE_API
GType gthree_object_get_type (void) G_GNUC_CONST;

GTHREE_API
GthreeObject *gthree_object_new ();

typedef void (*GthreeBeforeRenderCallback) (GthreeObject                *object,
                                            GthreeScene                 *scene,
                                            GthreeCamera                *camera);

typedef gboolean (*GthreeTraverseCallback) (GthreeObject                *object,
                                            gpointer                     user_data);

GTHREE_API
const char *                 gthree_object_get_name                     (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_name                     (GthreeObject                *object,
                                                                         const char                  *name);
GTHREE_API
const char *                 gthree_object_get_uuid                     (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_uuid                     (GthreeObject                *object,
                                                                         const char                  *uuid);
GTHREE_API
const graphene_matrix_t *    gthree_object_get_world_matrix             (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_world_matrix             (GthreeObject                *object,
                                                                         const graphene_matrix_t     *matrix);
GTHREE_API
void                         gthree_object_set_matrix_auto_update       (GthreeObject                *object,
                                                                         gboolean                     auto_update);
GTHREE_API
void                         gthree_object_update_matrix_world          (GthreeObject                *object,
                                                                         gboolean                     force);
GTHREE_API
void                         gthree_object_update_matrix                (GthreeObject                *object);
GTHREE_API
void                         gthree_object_update_matrix_view           (GthreeObject                *object,
                                                                         const graphene_matrix_t     *camera_matrix);
GTHREE_API
void                        gthree_object_set_up                        (GthreeObject                *object,
                                                                         const graphene_vec3_t       *up);
GTHREE_API
const graphene_vec3_t *     gthree_object_get_up                        (GthreeObject                *object);
GTHREE_API
void                         gthree_object_look_at                      (GthreeObject                *object,
                                                                         graphene_point3d_t          *pos);
GTHREE_API
const graphene_matrix_t *    gthree_object_get_matrix                   (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_matrix                   (GthreeObject                *object,
                                                                         const graphene_matrix_t     *matrix);
GTHREE_API
void                         gthree_object_apply_matrix                 (GthreeObject                *object,
                                                                         const graphene_matrix_t     *matrix);
GTHREE_API
void                         gthree_object_set_position                 (GthreeObject                *object,
                                                                         const graphene_vec3_t       *vec);
GTHREE_API
void                         gthree_object_set_position_point3d         (GthreeObject                *object,
                                                                         const graphene_point3d_t    *pos);
GTHREE_API
const graphene_vec3_t *      gthree_object_get_position                 (GthreeObject                *object);
GTHREE_API
void                         gthree_object_translate_on_axis            (GthreeObject                *object,
                                                                         const graphene_vec3_t       *axis,
                                                                         float                        distance);
GTHREE_API
void                         gthree_object_translate_x                  (GthreeObject                *object,
                                                                         float                        distance);
GTHREE_API
void                         gthree_object_translate_y                  (GthreeObject                *object,
                                                                         float                        distance);
GTHREE_API
void                         gthree_object_translate_z                  (GthreeObject                *object,
                                                                         float                        distance);
GTHREE_API
void                         gthree_object_set_scale                    (GthreeObject                *object,
                                                                         const graphene_vec3_t       *scale);
GTHREE_API
void                         gthree_object_set_scale_point3d            (GthreeObject                *object,
                                                                         const graphene_point3d_t    *scale);
GTHREE_API
const graphene_vec3_t *      gthree_object_get_scale                    (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_quaternion               (GthreeObject                *object,
                                                                         const graphene_quaternion_t *q);
GTHREE_API
const graphene_quaternion_t *gthree_object_get_quaternion               (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_rotation                 (GthreeObject                *object,
                                                                         const graphene_euler_t      *rot);
GTHREE_API
const graphene_euler_t *     gthree_object_get_rotation                 (GthreeObject                *object);
GTHREE_API
gboolean                     gthree_object_has_attribute_data           (GthreeObject                *object,
                                                                         GQuark                       attribute);
GTHREE_API
void                         gthree_object_get_world_matrix_floats      (GthreeObject                *object,
                                                                         float                       *dest);
GTHREE_API
void                         gthree_object_get_model_view_matrix_floats (GthreeObject                *object,
                                                                         float                       *dest);
GTHREE_API
void                         gthree_object_get_normal_matrix3_floats    (GthreeObject                *object,
                                                                         float                       *dest);
GTHREE_API
gboolean                     gthree_object_get_cast_shadow              (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_cast_shadow              (GthreeObject                *object,
                                                                         gboolean                     cast_shadow);
GTHREE_API
gboolean                     gthree_object_get_receive_shadow           (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_receive_shadow           (GthreeObject                *object,
                                                                         gboolean                     receive_shadow);
GTHREE_API
gboolean                     gthree_object_get_visible                  (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_visible                  (GthreeObject                *object,
                                                                         gboolean                     visible);
GTHREE_API
void                         gthree_object_show                         (GthreeObject                *object);
GTHREE_API
void                         gthree_object_hide                         (GthreeObject                *object);
GTHREE_API
guint32                      gthree_object_get_layer_mask               (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_layer                    (GthreeObject                *object,
                                                                         guint                        layer);
GTHREE_API
void                         gthree_object_enable_layer                 (GthreeObject                *object,
                                                                         guint                        layer);
GTHREE_API
void                         gthree_object_disable_layer                (GthreeObject                *object,
                                                                         guint                        layer);
GTHREE_API
void                         gthree_object_toggle_layer                 (GthreeObject                *object,
                                                                         guint                        layer);
GTHREE_API
gboolean                     gthree_object_check_layer                  (GthreeObject                *object,
                                                                         guint32                      layer_mask);
GTHREE_API
gboolean                     gthree_object_get_is_frustum_culled        (GthreeObject                *object);
GTHREE_API
void                         gthree_object_raycast                      (GthreeObject                *object,
                                                                         GthreeRaycaster             *raycaster,
                                                                         GPtrArray                   *intersections);
GTHREE_API
gboolean                     gthree_object_is_in_frustum                (GthreeObject                *object,
                                                                         const graphene_frustum_t    *frustum);
GTHREE_API
void                         gthree_object_add_child                    (GthreeObject                *object,
                                                                         GthreeObject                *child);
GTHREE_API
void                         gthree_object_remove_child                 (GthreeObject                *object,
                                                                         GthreeObject                *child);
GTHREE_API
void                         gthree_object_update                       (GthreeObject                *object);
GTHREE_API
void                         gthree_object_destroy                      (GthreeObject                *object);
GTHREE_API
GthreeObject *               gthree_object_get_parent                   (GthreeObject                *object);
GTHREE_API
GthreeObject *               gthree_object_get_first_child              (GthreeObject                *object);
GTHREE_API
GthreeObject *               gthree_object_get_last_child               (GthreeObject                *object);
GTHREE_API
int                          gthree_object_get_n_children               (GthreeObject                *object);
GTHREE_API
GthreeObject *               gthree_object_get_next_sibling             (GthreeObject                *object);
GTHREE_API
GthreeObject *               gthree_object_get_previous_sibling         (GthreeObject                *object);
GTHREE_API
void                         gthree_object_destroy_all_children         (GthreeObject                *object);
GTHREE_API
void                         gthree_object_set_before_render_callback   (GthreeObject                *object,
                                                                         GthreeBeforeRenderCallback   callback);
GTHREE_API
void                         gthree_object_get_mesh_extents             (GthreeObject                *object,
                                                                         graphene_box_t              *box);
GTHREE_API
void                         gthree_object_traverse                     (GthreeObject                *object,
                                                                         GthreeTraverseCallback       callback,
                                                                         gpointer                     user_data);
GTHREE_API
void                         gthree_object_traverse_visible             (GthreeObject                *object,
                                                                         GthreeTraverseCallback       callback,
                                                                         gpointer                     user_data);
GTHREE_API
void                         gthree_object_traverse_ancestors           (GthreeObject                *object,
                                                                         GthreeTraverseCallback       callback,
                                                                         gpointer                     user_data);
GTHREE_API
GList *                      gthree_object_find_by_type                 (GthreeObject                *object,
                                                                         GType                        g_type);
GTHREE_API
GList *                      gthree_object_find_by_name                 (GthreeObject                *object,
                                                                         const char                  *name);
GTHREE_API
GthreeObject                *gthree_object_find_first_by_name           (GthreeObject                *object,
                                                                         const char                  *name);



typedef struct _GthreeObjectIter GthreeObjectIter;

struct _GthreeObjectIter
{
  /*< private >*/
  gpointer GTHREE_PRIVATE_FIELD (dummy1);
  gpointer GTHREE_PRIVATE_FIELD (dummy2);
  gpointer GTHREE_PRIVATE_FIELD (dummy3);
  gint     GTHREE_PRIVATE_FIELD (dummy4);
  gpointer GTHREE_PRIVATE_FIELD (dummy5);
};

GTHREE_API
void     gthree_object_iter_init     (GthreeObjectIter        *iter,
                                      GthreeObject            *root);
GTHREE_API
gboolean gthree_object_iter_is_valid (const GthreeObjectIter  *iter);
GTHREE_API
gboolean gthree_object_iter_next     (GthreeObjectIter        *iter,
                                      GthreeObject           **child);
GTHREE_API
gboolean gthree_object_iter_prev     (GthreeObjectIter        *iter,
                                      GthreeObject           **child);
GTHREE_API
void     gthree_object_iter_remove   (GthreeObjectIter        *iter);
GTHREE_API
void     gthree_object_iter_destroy  (GthreeObjectIter        *iter);

G_END_DECLS

#endif /* __GTHREE_OBJECT_H__ */
