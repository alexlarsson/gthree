#include <math.h>
#include <epoxy/gl.h>

#include "gthreeobjectprivate.h"

#include <graphene.h>

enum
{
  DESTROY,
  PARENT_SET,

  LAST_SIGNAL
};

static guint object_signals[LAST_SIGNAL] = { 0, };

typedef struct {
  graphene_vec3_t position;
  graphene_quaternion_t quaternion;
  graphene_euler_t euler; /* Only valid if euler_valid == TRUE, canonical value in quaternion */
  graphene_vec3_t scale;
  graphene_vec3_t up;

  graphene_matrix_t matrix;
  graphene_matrix_t world_matrix;

  graphene_matrix_t model_view_matrix;
  graphene_matrix_t normal_matrix;

  gboolean visible;

  GthreeBeforeRenderCallback before_render_cb;

  /* object graph */
  GthreeObject *parent;
  GthreeObject *prev_sibling;
  GthreeObject *next_sibling;
  GthreeObject *first_child;
  GthreeObject *last_child;

  gint n_children;
  gint age;

  guint realized : 1;
  guint in_destruction : 1;
  guint euler_valid : 1;
  guint world_matrix_need_update : 1;
  guint matrix_auto_update : 1;

  guint frustum_culled : 1;
} GthreeObjectPrivate;

enum
{
  PROP_0,

  PROP_VISIBLE,
  PROP_PARENT,
  PROP_FIRST_CHILD,
  PROP_LAST_CHILD,

  PROP_NEXT_SIBLING,
  PROP_PREVIOUS_SIBLING,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeObject, gthree_object, G_TYPE_OBJECT);

#define PRIV(_o) ((GthreeObjectPrivate*)gthree_object_get_instance_private (_o))

GthreeObject *
gthree_object_new ()
{
  return g_object_new (gthree_object_get_type (),
                       NULL);
}

void
gthree_object_destroy (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_return_if_fail (GTHREE_IS_OBJECT (object));

  g_object_ref (object);

  /* avoid recursion while destroying */
  if (!priv->in_destruction)
    {
      priv->in_destruction = TRUE;

      g_object_run_dispose (G_OBJECT (object));

      priv->in_destruction = FALSE;
    }

  g_object_unref (object);
}


static void
gthree_object_init (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->matrix_auto_update = TRUE;
  priv->visible = TRUE;
  priv->frustum_culled = TRUE;

  graphene_matrix_init_identity (&priv->matrix);
  graphene_matrix_init_identity (&priv->world_matrix);
  graphene_quaternion_init_identity (&priv->quaternion);
  graphene_vec3_init (&priv->scale, 1, 1, 1);
  graphene_vec3_init (&priv->up, 0, 1, 0);
}

static void
gthree_object_real_destroy (GthreeObject *object)
{
  GthreeObjectIter iter;

  g_object_freeze_notify (G_OBJECT (object));

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, NULL))
    gthree_object_iter_destroy (&iter);

  g_object_thaw_notify (G_OBJECT (object));
}

static void
gthree_object_dispose (GObject *obj)
{
  GthreeObject *object = GTHREE_OBJECT (obj);
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_signal_emit (object, object_signals[DESTROY], 0);

  if (priv->parent != NULL)
    gthree_object_remove_child (priv->parent, object);

  /* parent must be gone at this point */
  g_assert (priv->parent == NULL);

  G_OBJECT_CLASS (gthree_object_parent_class)->dispose (obj);
}


static void
gthree_object_finalize (GObject *obj)
{
  //GthreeObject *object = GTHREE_OBJECT (obj);
  //GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  G_OBJECT_CLASS (gthree_object_parent_class)->finalize (obj);
}

static void
gthree_object_set_property (GObject *obj,
                            guint prop_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
  GthreeObject *object = GTHREE_OBJECT (obj);

  switch (prop_id)
    {
    case PROP_VISIBLE:
      gthree_object_set_visible (object,  g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_object_get_property (GObject *obj,
                            guint prop_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  GthreeObject *object = GTHREE_OBJECT (obj);
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  switch (prop_id)
    {
    case PROP_VISIBLE:
      g_value_set_boolean (value, priv->visible);
      break;

    case PROP_PARENT:
      g_value_set_object (value, priv->parent);
      break;

    case PROP_FIRST_CHILD:
      g_value_set_object (value, priv->first_child);
      break;

    case PROP_LAST_CHILD:
      g_value_set_object (value, priv->last_child);
      break;

    case PROP_NEXT_SIBLING:
      g_value_set_object (value, priv->next_sibling);
      break;

    case PROP_PREVIOUS_SIBLING:
      g_value_set_object (value, priv->prev_sibling);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}


static void
gthree_object_class_init (GthreeObjectClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->set_property = gthree_object_set_property;
  obj_class->get_property = gthree_object_get_property;
  obj_class->dispose = gthree_object_dispose;
  obj_class->finalize = gthree_object_finalize;

  klass->destroy = gthree_object_real_destroy;

  object_signals[DESTROY] =
    g_signal_new ("destroy",
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (GthreeObjectClass, destroy),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_signals[PARENT_SET] =
    g_signal_new ("parent-set",
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GthreeObjectClass, parent_set),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  GTHREE_TYPE_OBJECT);


  obj_props[PROP_VISIBLE] =
    g_param_spec_boolean ("visible", "Visible", "Visible",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_PARENT] =
    g_param_spec_object ("parent", "Parent", "Parent",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_FIRST_CHILD] =
    g_param_spec_object ("first-child", "First Child", "First Child",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_LAST_CHILD] =
    g_param_spec_object ("last-child", "Last Child", "Last Child",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_NEXT_SIBLING] =
    g_param_spec_object ("next-sibling", "Next Sibling", "Next Sibling",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_PREVIOUS_SIBLING] =
    g_param_spec_object ("previous-sibling", "Previous Sibling", "Previous Sibling",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (obj_class, N_PROPS, obj_props);
}

void
gthree_object_set_matrix_auto_update (GthreeObject *object,
                                      gboolean auto_update)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->matrix_auto_update = !! auto_update;
}

gboolean
gthree_object_get_visible (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->visible;
}

void
gthree_object_set_visible (GthreeObject *object,
                           gboolean visible)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (priv->visible == visible)
    return;

  priv->visible = visible;

  g_object_notify_by_pspec (G_OBJECT (object), obj_props[PROP_VISIBLE]);
}

gboolean
gthree_object_get_is_frustum_culled (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->frustum_culled;
}

gboolean
gthree_object_is_in_frustum (GthreeObject *object,
                             const graphene_frustum_t *frustum)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  if (class->in_frustum)
    return class->in_frustum (object, frustum);

  return TRUE;
}

void
gthree_object_look_at (GthreeObject *object,
                       graphene_point3d_t *pos)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  graphene_matrix_t m;
  graphene_vec3_t vec;

  graphene_point3d_to_vec3 (pos, &vec);
  graphene_matrix_init_look_at (&m, &priv->position, &vec, &priv->up);
  graphene_quaternion_init_from_matrix (&priv->quaternion, &m);
  priv->euler_valid = FALSE;
}

void
gthree_object_set_position (GthreeObject *object,
                            const graphene_point3d_t *pos)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_point3d_to_vec3 (pos, &priv->position);
}

graphene_point3d_t *
gthree_object_get_position (GthreeObject *object,
                            graphene_point3d_t *pos)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return graphene_point3d_init_from_vec3 (pos, &priv->position);
}

void
gthree_object_set_scale (GthreeObject                *object,
                         const graphene_point3d_t    *scale)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_point3d_to_vec3 (scale, &priv->scale);
}

void
gthree_object_set_quaternion (GthreeObject *object,
                              const graphene_quaternion_t *q)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_quaternion_init_from_quaternion (&priv->quaternion, q);
  priv->euler_valid = FALSE;
}

const graphene_quaternion_t *
gthree_object_get_quaternion (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return &priv->quaternion;
}

void
gthree_object_set_rotation (GthreeObject *object,
                            const graphene_euler_t *rot)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->euler = *rot;
  priv->euler_valid = TRUE;
  graphene_quaternion_init_from_euler (&priv->quaternion, rot);
}

const graphene_euler_t *
gthree_object_get_rotation (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (!priv->euler_valid)
    {
      graphene_euler_init_from_quaternion (&priv->euler, &priv->quaternion, GRAPHENE_EULER_ORDER_DEFAULT);
      priv->euler_valid = TRUE;
    }

  return &priv->euler;
}

void
gthree_object_update_matrix (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  graphene_point3d_t pos;

  graphene_quaternion_to_matrix (&priv->quaternion, &priv->matrix);
  graphene_matrix_scale (&priv->matrix,
                         graphene_vec3_get_x (&priv->scale),
                         graphene_vec3_get_y (&priv->scale),
                         graphene_vec3_get_z (&priv->scale));
  graphene_point3d_init_from_vec3 (&pos, &priv->position);
  graphene_matrix_translate  (&priv->matrix, &pos);

  priv->world_matrix_need_update = TRUE;
}

const graphene_matrix_t *
gthree_object_get_world_matrix (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return &priv->world_matrix;
}

/* This is a bit special, it overrides the *world* matrix, which is
   normally calculated from the object position/rotation/scale + that
   of its parent. However, it can be use quite useful in e.g. the pre_render
   callback. */
void
gthree_object_set_world_matrix (GthreeObject *object, const graphene_matrix_t *matrix)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->world_matrix = *matrix;
  priv->world_matrix_need_update = FALSE;
}

void
gthree_object_update_matrix_world (GthreeObject *object,
                                   gboolean force)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObject *child;

  if (priv->matrix_auto_update)
    gthree_object_update_matrix (object);

  if (priv->world_matrix_need_update || force)
    {
      if (priv->parent == NULL)
        priv->world_matrix = priv->matrix;
      else
        graphene_matrix_multiply (&priv->matrix,
                                  &PRIV (priv->parent)->world_matrix,
                                  &priv->world_matrix);

      priv->world_matrix_need_update = FALSE;
      force = TRUE;
    }

  for (child = priv->first_child;
       child != NULL;
       child = PRIV (child)->next_sibling)
    gthree_object_update_matrix_world (child, force);
}

void
gthree_object_update_matrix_view (GthreeObject *object,
                                  const graphene_matrix_t *camera_matrix)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_matrix_multiply (&priv->world_matrix, camera_matrix, &priv->model_view_matrix);

  graphene_matrix_inverse (&priv->model_view_matrix, &priv->normal_matrix);
  graphene_matrix_transpose (&priv->normal_matrix, &priv->normal_matrix);
}

void
gthree_object_get_model_view_matrix_floats (GthreeObject *object,
                                            float *dest)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_matrix_to_float (&priv->model_view_matrix, dest);
}

void
gthree_object_get_normal_matrix3_floats (GthreeObject *object,
                                         float *dest)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  float dest4[16];

  graphene_matrix_to_float (&priv->normal_matrix, dest4);

  dest[0] = dest4[0];  dest[1] = dest4[1]; dest[2] = dest4[2];
  dest[3] = dest4[4];  dest[4] = dest4[5]; dest[5] = dest4[6];
  dest[6] = dest4[8];  dest[7] = dest4[9]; dest[8] = dest4[10];
}

void
gthree_object_get_world_matrix_floats (GthreeObject *object,
                                       float *dest)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_matrix_to_float (&priv->world_matrix, dest);
}


void
gthree_object_add_child (GthreeObject              *object,
                         GthreeObject              *child)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObjectPrivate *child_priv = gthree_object_get_instance_private (child);
  GthreeObject *last_child;
  GObject *obj;

  g_return_if_fail (GTHREE_IS_OBJECT (object));
  g_return_if_fail (GTHREE_IS_OBJECT (child));
  g_return_if_fail (object != child);
  g_return_if_fail (child_priv->parent == NULL);

  if (child_priv->parent != NULL)
    {
      g_warning ("The object '%p' already has a parent, '%p'. You must "
                 "use gthree_object_remove_child() first.", child, child_priv->parent);
      return;
    }

  obj = G_OBJECT (object);
  g_object_freeze_notify (obj);

  g_object_ref_sink (child);

  child_priv->parent = object;

  last_child = priv->last_child;
  child_priv->prev_sibling = last_child;
  if (last_child != NULL)
    {
      GthreeObjectPrivate *last_child_priv = gthree_object_get_instance_private (last_child);
      GthreeObject *tmp = last_child_priv->next_sibling;

      child_priv->next_sibling = tmp;

      if (tmp != NULL)
        PRIV (tmp)->prev_sibling = child;

      last_child_priv->next_sibling = child;
    }
  else
    child_priv->next_sibling = NULL;

  if (child_priv->prev_sibling == NULL)
    priv->first_child = child;

  if (child_priv->next_sibling == NULL)
    priv->last_child = child;

  g_assert (child_priv->parent == object);

  priv->n_children += 1;

  priv->age += 1;

  g_signal_emit (child, object_signals[PARENT_SET], 0, NULL);

  g_object_thaw_notify (obj);
}

void
gthree_object_remove_child (GthreeObject                 *object,
                            GthreeObject                 *child)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObjectPrivate *child_priv = gthree_object_get_instance_private (child);
  GthreeObject *prev_sibling, *next_sibling;
  GObject *obj;

  g_return_if_fail (GTHREE_IS_OBJECT (object));
  g_return_if_fail (GTHREE_IS_OBJECT (child));
  g_return_if_fail (object != child);
  g_return_if_fail (child_priv->parent != NULL);
  g_return_if_fail (child_priv->parent == object);

  obj = G_OBJECT (object);
  g_object_freeze_notify (obj);

  prev_sibling = child_priv->prev_sibling;
  next_sibling = child_priv->next_sibling;

  if (prev_sibling != NULL)
    PRIV (prev_sibling)->next_sibling = next_sibling;

  if (next_sibling != NULL)
    PRIV (next_sibling)->prev_sibling = prev_sibling;

  if (priv->first_child == child)
    priv->first_child = next_sibling;

  if (priv->last_child == child)
    priv->last_child = prev_sibling;

  child_priv->parent = NULL;
  child_priv->prev_sibling = NULL;
  child_priv->next_sibling = NULL;

  priv->n_children -= 1;

  priv->age += 1;

  g_signal_emit (child, object_signals[PARENT_SET], 0, object);

  g_object_thaw_notify (obj);

  /* remove the reference we acquired in gthree_object_add_child() */
  g_object_unref (child);
}


GthreeObject *
gthree_object_get_parent (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_return_val_if_fail (GTHREE_IS_OBJECT (object), NULL);

  return priv->parent;
}

GthreeObject *
gthree_object_get_first_child (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_return_val_if_fail (GTHREE_IS_OBJECT (object), NULL);

  return priv->first_child;
}

GthreeObject *
gthree_object_get_last_child (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_return_val_if_fail (GTHREE_IS_OBJECT (object), NULL);

  return priv->last_child;
}

GthreeObject *
gthree_object_get_previous_sibling (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_return_val_if_fail (GTHREE_IS_OBJECT (object), NULL);

  return priv->prev_sibling;
}

GthreeObject *
gthree_object_get_next_sibling (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_return_val_if_fail (GTHREE_IS_OBJECT (object), NULL);

  return priv->next_sibling;
}

void
gthree_object_destroy_all_children (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObjectIter iter;

  g_return_if_fail (GTHREE_IS_OBJECT (object));

  if (priv->n_children == 0)
    return;

  g_object_freeze_notify (G_OBJECT (object));

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, NULL))
    gthree_object_iter_destroy (&iter);

  g_object_thaw_notify (G_OBJECT (object));

  /* sanity check */
  g_assert (priv->first_child == NULL);
  g_assert (priv->last_child == NULL);
  g_assert (priv->n_children == 0);
}

void
gthree_object_update (GthreeObject *object)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  if (class->update)
    class->update (object);
}

void
gthree_object_fill_render_list (GthreeObject   *object,
                                GthreeRenderList *list)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  if (class->fill_render_list)
    return class->fill_render_list (object, list);
}

void
gthree_object_set_before_render_callback (GthreeObject                *object,
                                          GthreeBeforeRenderCallback  callback)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  priv->before_render_cb = callback;
}

void
gthree_object_call_before_render_callback  (GthreeObject                *object,
                                            GthreeScene                 *scene,
                                            GthreeCamera                *camera)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (priv->before_render_cb)
    priv->before_render_cb (object, scene, camera);
}


typedef struct _RealObjectIter
{
  GthreeObject *root;           /* dummy1 */
  GthreeObject *current;        /* dummy2 */
  gpointer padding_1;           /* dummy3 */
  gint age;                     /* dummy4 */
  gpointer padding_2;           /* dummy5 */
} RealObjectIter;

void
gthree_object_iter_init (GthreeObjectIter *iter,
                         GthreeObject     *root)
{
  RealObjectIter *ri = (RealObjectIter *) iter;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (GTHREE_IS_OBJECT (root));

  ri->root = root;
  ri->current = NULL;
  ri->age = PRIV (root)->age;
}

gboolean
gthree_object_iter_is_valid (const GthreeObjectIter *iter)
{
  RealObjectIter *ri = (RealObjectIter *) iter;

  g_return_val_if_fail (iter != NULL, FALSE);

  if (ri->root == NULL)
    return FALSE;

  return PRIV (ri->root)->age == ri->age;
}

gboolean
gthree_object_iter_next (GthreeObjectIter  *iter,
                         GthreeObject     **child)
{
  RealObjectIter *ri = (RealObjectIter *) iter;

  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (ri->root != NULL, FALSE);
#ifndef G_DISABLE_ASSERT
  g_return_val_if_fail (ri->age == PRIV (ri->root)->age, FALSE);
#endif

  if (ri->current == NULL)
    ri->current = PRIV (ri->root)->first_child;
  else
    ri->current = PRIV (ri->current)->next_sibling;

  if (child != NULL)
    *child = ri->current;

  return ri->current != NULL;
}

gboolean
gthree_object_iter_prev (GthreeObjectIter  *iter,
                         GthreeObject     **child)
{
  RealObjectIter *ri = (RealObjectIter *) iter;

  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (ri->root != NULL, FALSE);
#ifndef G_DISABLE_ASSERT
  g_return_val_if_fail (ri->age == PRIV (ri->root)->age, FALSE);
#endif

  if (ri->current == NULL)
    ri->current = PRIV (ri->root)->last_child;
  else
    ri->current = PRIV (ri->current)->prev_sibling;

  if (child != NULL)
    *child = ri->current;

  return ri->current != NULL;
}

void
gthree_object_iter_remove (GthreeObjectIter *iter)
{
  RealObjectIter *ri = (RealObjectIter *) iter;
  GthreeObject *cur;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (ri->root != NULL);
#ifndef G_DISABLE_ASSERT
  g_return_if_fail (ri->age == PRIV (ri->root)->age);
#endif
  g_return_if_fail (ri->current != NULL);

  cur = ri->current;

  if (cur != NULL)
    {
      ri->current = PRIV (cur)->prev_sibling;

      gthree_object_remove_child (ri->root, cur);

      ri->age += 1;
    }
}

void
gthree_object_iter_destroy (GthreeObjectIter *iter)
{
  RealObjectIter *ri = (RealObjectIter *) iter;
  GthreeObject *cur;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (ri->root != NULL);
#ifndef G_DISABLE_ASSERT
  g_return_if_fail (ri->age == PRIV (ri->root)->age);
#endif
  g_return_if_fail (ri->current != NULL);

  cur = ri->current;

  if (cur != NULL)
    {
      ri->current = PRIV (cur)->prev_sibling;

      gthree_object_destroy (cur);

      ri->age += 1;
    }
}
