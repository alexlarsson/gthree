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
  GInitiallyUnowned parent;
} GthreeObject;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeObject, g_object_unref)

typedef struct {
  GInitiallyUnownedClass parent_class;

  gboolean (* in_frustum)       (GthreeObject             *object,
                                 const graphene_frustum_t *frustum);

  void (* parent_set)           (GthreeObject          *object,
                                 GthreeObject          *old_parent);

  void (* update)               (GthreeObject          *object);
  void (* destroy)              (GthreeObject          *object);


  void (* fill_render_list)      (GthreeObject          *object,
                                  GthreeRenderList      *list);
} GthreeObjectClass;

GType gthree_object_get_type (void) G_GNUC_CONST;

GthreeObject *gthree_object_new ();

typedef void (*GthreeBeforeRenderCallback) (GthreeObject                *object,
                                            GthreeScene                 *scene,
                                            GthreeCamera                *camera);

const graphene_matrix_t *    gthree_object_get_world_matrix             (GthreeObject                *object);
void                         gthree_object_set_world_matrix             (GthreeObject                *object,
                                                                         const graphene_matrix_t     *matrix);
void                         gthree_object_set_matrix_auto_update       (GthreeObject                *object,
                                                                         gboolean                     auto_update);
void                         gthree_object_update_matrix_world          (GthreeObject                *object,
                                                                         gboolean                     force);
void                         gthree_object_update_matrix                (GthreeObject                *object);
void                         gthree_object_update_matrix_view           (GthreeObject                *object,
                                                                         const graphene_matrix_t     *camera_matrix);
void                         gthree_object_look_at                      (GthreeObject                *object,
                                                                         graphene_point3d_t          *pos);
const graphene_matrix_t *    gthree_object_get_matrix                   (GthreeObject                *object);
void                         gthree_object_set_matrix                   (GthreeObject                *object,
                                                                         const graphene_matrix_t     *matrix);
void                         gthree_object_set_position                 (GthreeObject                *object,
                                                                         const graphene_point3d_t    *pos);
graphene_point3d_t *         gthree_object_get_position                 (GthreeObject                *object,
                                                                         graphene_point3d_t          *res);
void                         gthree_object_set_scale                    (GthreeObject                *object,
                                                                         const graphene_point3d_t    *scale);
void                         gthree_object_set_quaternion               (GthreeObject                *object,
                                                                         const graphene_quaternion_t *q);
const graphene_quaternion_t *gthree_object_get_quaternion               (GthreeObject                *object);
void                         gthree_object_set_rotation                 (GthreeObject                *object,
                                                                         const graphene_euler_t      *rot);
const graphene_euler_t *     gthree_object_get_rotation                 (GthreeObject                *object);
gboolean                     gthree_object_has_attribute_data           (GthreeObject                *object,
                                                                         GQuark                       attribute);
void                         gthree_object_get_world_matrix_floats      (GthreeObject                *object,
                                                                         float                       *dest);
void                         gthree_object_get_model_view_matrix_floats (GthreeObject                *object,
                                                                         float                       *dest);
void                         gthree_object_get_normal_matrix3_floats    (GthreeObject                *object,
                                                                         float                       *dest);
gboolean                     gthree_object_get_visible                  (GthreeObject                *object);
void                         gthree_object_set_visible                  (GthreeObject                *object,
                                                                         gboolean                     visible);
gboolean                     gthree_object_get_is_frustum_culled        (GthreeObject                *object);
gboolean                     gthree_object_is_in_frustum                (GthreeObject                *object,
                                                                         const graphene_frustum_t    *frustum);
void                         gthree_object_add_child                    (GthreeObject                *object,
                                                                         GthreeObject                *child);
void                         gthree_object_remove_child                 (GthreeObject                *object,
                                                                         GthreeObject                *child);
void                         gthree_object_update                       (GthreeObject                *object);
void                         gthree_object_destroy                      (GthreeObject                *object);
GthreeObject *               gthree_object_get_parent                   (GthreeObject                *object);
GthreeObject *               gthree_object_get_first_child              (GthreeObject                *object);
GthreeObject *               gthree_object_get_last_child               (GthreeObject                *object);
GthreeObject *               gthree_object_get_next_sibling             (GthreeObject                *object);
GthreeObject *               gthree_object_get_previous_sibling         (GthreeObject                *object);
void                         gthree_object_destroy_all_children         (GthreeObject                *object);
void                         gthree_object_set_before_render_callback   (GthreeObject                *object,
                                                                         GthreeBeforeRenderCallback  callback);
GList *                      gthree_object_find_by_type                 (GthreeObject                *object,
                                                                         GType                        g_type);
void                         gthree_object_get_mesh_extents             (GthreeObject                *object,
                                                                         graphene_box_t              *box);


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

void     gthree_object_iter_init     (GthreeObjectIter        *iter,
                                      GthreeObject            *root);
gboolean gthree_object_iter_is_valid (const GthreeObjectIter  *iter);
gboolean gthree_object_iter_next     (GthreeObjectIter        *iter,
                                      GthreeObject           **child);
gboolean gthree_object_iter_prev     (GthreeObjectIter        *iter,
                                      GthreeObject           **child);
void     gthree_object_iter_remove   (GthreeObjectIter        *iter);
void     gthree_object_iter_destroy  (GthreeObjectIter        *iter);

G_END_DECLS

#endif /* __GTHREE_OBJECT_H__ */
