#include <math.h>
#include <epoxy/gl.h>

#include "gthreeobjectprivate.h"
#include "gthreemesh.h"

#include <graphene.h>

enum
{
  DESTROY,
  PARENT_SET,

  LAST_SIGNAL
};

static GQuark q_modelMatrix;
static GQuark q_modelViewMatrix;
static GQuark q_normalMatrix;


static guint object_signals[LAST_SIGNAL] = { 0, };

typedef struct {
  char *name;
  char *uuid;

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
  gboolean cast_shadow;
  gboolean receive_shadow;
  guint32 layer_mask;

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
  guint matrix_need_update : 1;

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

static gboolean gthree_object_real_update_matrix_world (GthreeObject *object,
                                                        gboolean force);
static void gthree_object_real_set_direct_uniforms  (GthreeObject *object,
                                                     GthreeProgram *program,
                                                     GthreeRenderer *renderer);

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

  priv->uuid = g_uuid_string_random ();

  priv->matrix_auto_update = TRUE;
  priv->matrix_need_update = TRUE;
  priv->visible = TRUE;
  priv->layer_mask = 1;
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
  GthreeObject *object = GTHREE_OBJECT (obj);
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_free (priv->uuid);
  g_free (priv->name);

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

#define INIT_QUARK(name) q_##name = g_quark_from_static_string (#name)
  INIT_QUARK(modelMatrix);
  INIT_QUARK(modelViewMatrix);
  INIT_QUARK(normalMatrix);

  obj_class->set_property = gthree_object_set_property;
  obj_class->get_property = gthree_object_get_property;
  obj_class->dispose = gthree_object_dispose;
  obj_class->finalize = gthree_object_finalize;

  klass->destroy = gthree_object_real_destroy;
  klass->update_matrix_world = gthree_object_real_update_matrix_world;
  klass->set_direct_uniforms = gthree_object_real_set_direct_uniforms;

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

const char *
gthree_object_get_name (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->name;
}

void
gthree_object_set_name (GthreeObject *object,
                        const char *name)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_free (priv->name);
  priv->name = g_strdup (name);
}

const char *
gthree_object_get_uuid (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->uuid;
}

void
gthree_object_set_uuid (GthreeObject *object,
                        const char *uuid)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  g_free (priv->uuid);
  priv->uuid = g_strdup (uuid);
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
gthree_object_get_cast_shadow (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->cast_shadow;
}

void
gthree_object_set_cast_shadow (GthreeObject *object,
                               gboolean cast_shadow)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (priv->cast_shadow == cast_shadow)
    return;

  priv->cast_shadow = cast_shadow;
}

gboolean
gthree_object_get_receive_shadow (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->receive_shadow;
}

void
gthree_object_set_receive_shadow (GthreeObject *object,
                                  gboolean receive_shadow)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (priv->receive_shadow == receive_shadow)
    return;

  priv->receive_shadow = receive_shadow;
}

void
gthree_object_show (GthreeObject *object)
{
  gthree_object_set_visible (object, TRUE);
}

void
gthree_object_hide (GthreeObject *object)
{
  gthree_object_set_visible (object, FALSE);
}

guint32
gthree_object_get_layer_mask (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return priv->layer_mask;
}

void
gthree_object_set_layer (GthreeObject *object,
                         guint layer)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->layer_mask = 1 << layer;
}

void
gthree_object_enable_layer (GthreeObject *object,
                            guint layer)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->layer_mask |= 1 << layer;
}

void
gthree_object_disable_layer (GthreeObject *object,
                             guint layer)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->layer_mask &= ~ (1 << layer);
}

void
gthree_object_toggle_layer (GthreeObject *object,
                            guint layer)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->layer_mask ^= ~ 1 << layer;
}

gboolean
gthree_object_check_layer (GthreeObject *object,
                           guint32 layer_mask)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return (priv->layer_mask & layer_mask) != 0;
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
gthree_object_raycast (GthreeObject                *object,
                       GthreeRaycaster             *raycaster,
                       GPtrArray                   *intersections)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  if (class->raycast)
    return class->raycast (object, raycaster, intersections);
}

void
gthree_object_set_up (GthreeObject *object,
                      const graphene_vec3_t *up)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->up = *up;
}

const graphene_vec3_t *
gthree_object_get_up (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return &priv->up;
}

void
gthree_object_look_at (GthreeObject *object,
                       const graphene_vec3_t *pos)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  graphene_matrix_t m;

  graphene_matrix_init_look_at (&m, &priv->position, pos, &priv->up);
  graphene_quaternion_init_from_matrix (&priv->quaternion, &m);
  priv->matrix_need_update = TRUE;
  priv->euler_valid = FALSE;
}

void
gthree_object_look_at_point3d (GthreeObject *object,
                               const graphene_point3d_t *pos)
{
  graphene_vec3_t vec;

  graphene_point3d_to_vec3 (pos, &vec);
  gthree_object_look_at (object, &vec);
}

void
gthree_object_look_at_xyz (GthreeObject *object,
                           float         x,
                           float         y,
                           float         z)
{
  graphene_vec3_t pos;

  graphene_vec3_init (&pos, x, y, z);
  gthree_object_look_at (object, &pos);
}

void
gthree_object_set_position (GthreeObject *object,
                            const graphene_vec3_t *vec)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->position = *vec;
  priv->matrix_need_update = TRUE;
}

void
gthree_object_set_position_xyz (GthreeObject *object,
                                float x,
                                float y,
                                float z)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_vec3_init (&priv->position, x, y, z);
  priv->matrix_need_update = TRUE;
}

void
gthree_object_set_position_point3d (GthreeObject *object,
                                    const graphene_point3d_t *pos)
{
  graphene_vec3_t vec;

  graphene_point3d_to_vec3 (pos, &vec);
  gthree_object_set_position (object, &vec);
}

void
gthree_object_translate_on_axis (GthreeObject                *object,
                                 const graphene_vec3_t       *axis,
                                 float                        distance)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  graphene_matrix_t m;
  graphene_vec3_t rotated_axis;

  // translate object by distance along axis in object space
  // axis is assumed to be normalized

  graphene_quaternion_to_matrix (&priv->quaternion, &m);
  graphene_matrix_transform_vec3 (&m, axis, &rotated_axis);
  graphene_vec3_scale (&rotated_axis, distance, &rotated_axis);
  graphene_vec3_add (&priv->position, &rotated_axis, &priv->position);
  priv->matrix_need_update = TRUE;
}

void
gthree_object_translate_x (GthreeObject                *object,
                           float                        distance)
{
  gthree_object_translate_on_axis (object, graphene_vec3_x_axis (), distance);
}

void
gthree_object_translate_y (GthreeObject                *object,
                           float                        distance)
{
  gthree_object_translate_on_axis (object, graphene_vec3_y_axis (), distance);
}

void
gthree_object_translate_z (GthreeObject                *object,
                           float                        distance)
{
  gthree_object_translate_on_axis (object, graphene_vec3_z_axis (), distance);
}


const graphene_vec3_t *
gthree_object_get_position (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return &priv->position;
}

void
gthree_object_set_scale (GthreeObject             *object,
                         const graphene_vec3_t    *scale)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  priv->scale = *scale;
  priv->matrix_need_update = TRUE;
}

void
gthree_object_set_scale_xyz (GthreeObject                *object,
                             float                        x,
                             float                        y,
                             float                        z)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_vec3_init (&priv->scale, x, y, z);
  priv->matrix_need_update = TRUE;
}

void
gthree_object_set_scale_uniform (GthreeObject            *object,
                                 float                    scale)
{
  gthree_object_set_scale_xyz (object, scale, scale, scale);
}

void
gthree_object_set_scale_point3d (GthreeObject                *object,
                                 const graphene_point3d_t    *scale)
{
  graphene_vec3_t v;
  graphene_point3d_to_vec3 (scale, &v);
  gthree_object_set_scale (object, &v);
}

void
gthree_object_set_quaternion (GthreeObject *object,
                              const graphene_quaternion_t *q)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_quaternion_init_from_quaternion (&priv->quaternion, q);
  priv->euler_valid = FALSE;
  priv->matrix_need_update = TRUE;
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
  priv->matrix_need_update = TRUE;
}

void
gthree_object_set_rotation_xyz (GthreeObject *object,
                                float         x,
                                float         y,
                                float         z)
{
  graphene_euler_t euler;

  gthree_object_set_rotation (object, graphene_euler_init (&euler, x, y, z));
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

const graphene_vec3_t *
gthree_object_get_scale (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return &priv->scale;
}

/* Only valid if update_matrix () was run */
const graphene_matrix_t *
gthree_object_get_matrix (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  return &priv->matrix;
}

static void
gthree_object_decompose_matrix (GthreeObject                *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  graphene_vec3_t shear;
  graphene_vec4_t perspective;

  if (!graphene_matrix_decompose (&priv->matrix,
                                  &priv->position,
                                  &priv->scale,
                                  &priv->quaternion,
                                  &shear, &perspective))
    {
      // If this fails, at least get the position
      graphene_vec4_t transl;
      graphene_matrix_get_row (&priv->matrix, 3, &transl);
      graphene_vec4_get_xyz (&transl, &priv->position);
    }
}

/* Only useful if auto-update are off, otherwise its overwritten the next frame */
void
gthree_object_set_matrix (GthreeObject                *object,
                          const graphene_matrix_t     *matrix)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  graphene_matrix_init_from_matrix  (&priv->matrix, matrix);

  gthree_object_decompose_matrix (object);

  priv->world_matrix_need_update = TRUE;
  priv->matrix_need_update = FALSE;
}

void
gthree_object_apply_matrix (GthreeObject                *object,
                            const graphene_matrix_t     *matrix)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (priv->matrix_auto_update)
    gthree_object_update_matrix (object);

  graphene_matrix_multiply (matrix,
                            &priv->matrix,
                            &priv->matrix);

  gthree_object_decompose_matrix (object);

  priv->world_matrix_need_update = TRUE;
  priv->matrix_need_update = FALSE;
}

void
gthree_object_update_matrix (GthreeObject *object)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  graphene_point3d_t pos;

  if (priv->matrix_need_update)
    {
      priv->matrix_need_update = FALSE;
      graphene_matrix_init_scale (&priv->matrix,
                                  graphene_vec3_get_x (&priv->scale),
                                  graphene_vec3_get_y (&priv->scale),
                                  graphene_vec3_get_z (&priv->scale));
      graphene_matrix_rotate_quaternion (&priv->matrix, &priv->quaternion);
      graphene_point3d_init_from_vec3 (&pos, &priv->position);
      graphene_matrix_translate  (&priv->matrix, &pos);

      priv->world_matrix_need_update = TRUE;
    }
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

  // TODO: decompose matrix into position, quat, scale
}

static gboolean
gthree_object_real_update_matrix_world (GthreeObject *object,
                                        gboolean force)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

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

  return force;
}

void
gthree_object_update_matrix_world (GthreeObject *object,
                                   gboolean force)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);
  GthreeObject *child;

  force = class->update_matrix_world (object, force);

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

  if (child_priv->parent != NULL)
    {
      g_warning ("The object '%p' already has a parent, '%p'. You must "
                 "use gthree_object_remove_child() first.", child, child_priv->parent);
      return;
    }

  g_return_if_fail (GTHREE_IS_OBJECT (object));
  g_return_if_fail (GTHREE_IS_OBJECT (child));
  g_return_if_fail (object != child);
  g_return_if_fail (child_priv->parent == NULL);

  obj = G_OBJECT (object);
  g_object_freeze_notify (obj);

  g_object_ref (child);

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

int
gthree_object_get_n_children (GthreeObject *object)
{
  GthreeObjectIter iter;
  GthreeObject *child;
  int len = 0;

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    len++;

  return len;
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
gthree_object_update (GthreeObject *object,
                      GthreeRenderer *renderer)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  if (class->update)
    class->update (object, renderer);
}

void
gthree_object_fill_render_list (GthreeObject   *object,
                                GthreeRenderList *list)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  if (class->fill_render_list)
    return class->fill_render_list (object, list);
}

static void
gthree_object_real_set_direct_uniforms  (GthreeObject *object,
                                         GthreeProgram *program,
                                         GthreeRenderer *renderer)
{
  float matrix[16];
  int mvm_location = gthree_program_lookup_uniform_location (program, q_modelViewMatrix);
  int nm_location = gthree_program_lookup_uniform_location (program, q_normalMatrix);
  int mm_location;

  gthree_object_get_model_view_matrix_floats (object, matrix);
  glUniformMatrix4fv (mvm_location, 1, FALSE, matrix);

  if (nm_location >= 0)
    {
      gthree_object_get_normal_matrix3_floats (object, matrix);
      glUniformMatrix3fv (nm_location, 1, FALSE, matrix);
    }

  mm_location = gthree_program_lookup_uniform_location (program, q_modelMatrix);
  if (mm_location >= 0)
    {
      float matrix[16];
      gthree_object_get_world_matrix_floats (object, matrix);
      glUniformMatrix4fv (mm_location, 1, FALSE, matrix);
    }
}

void
gthree_object_set_direct_uniforms  (GthreeObject *object,
                                    GthreeProgram *program,
                                    GthreeRenderer *renderer)
{
  GthreeObjectClass *class = GTHREE_OBJECT_GET_CLASS(object);

  class->set_direct_uniforms (object, program, renderer);
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

static gboolean
_gthree_object_traverse (GthreeObject                *object,
                        GthreeTraverseCallback       callback,
                        gpointer                     user_data)
{
  GthreeObjectIter iter;
  GthreeObject *child;

  if (!callback (object, user_data))
    return FALSE;

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    {
      if (!_gthree_object_traverse (child, callback, user_data))
        return FALSE;
    }

  return TRUE;
}

void
gthree_object_traverse (GthreeObject                *object,
                        GthreeTraverseCallback       callback,
                        gpointer                     user_data)
{
  _gthree_object_traverse (object, callback, user_data);
}

static gboolean
_gthree_object_traverse_visible (GthreeObject                *object,
                                 GthreeTraverseCallback       callback,
                                 gpointer                     user_data)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObjectIter iter;
  GthreeObject *child;

  if (!priv->visible)
    return TRUE;

  if (!callback (object, user_data))
    return FALSE;

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    {
      if (!_gthree_object_traverse (child, callback, user_data))
        return FALSE;
    }

  return TRUE;
}

void
gthree_object_traverse_visible (GthreeObject                *object,
                                GthreeTraverseCallback       callback,
                                gpointer                     user_data)
{
  _gthree_object_traverse_visible (object, callback, user_data);
}

void
gthree_object_traverse_ancestors (GthreeObject                *object,
                                  GthreeTraverseCallback       callback,
                                  gpointer                     user_data)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (priv->parent != NULL)
    {
      callback (priv->parent, user_data);
      gthree_object_traverse_ancestors (priv->parent, callback, user_data);
    }
}

struct FindByType {
  GType g_type;
  GList *list;
};

static gboolean
find_by_type_cb (GthreeObject *object,
                 gpointer user_data)
{
  struct FindByType *data = user_data;

  if (G_TYPE_CHECK_INSTANCE_TYPE (object, data->g_type))
    data->list = g_list_prepend (data->list, object);

  return TRUE;
}

GList *
gthree_object_find_by_type (GthreeObject *object,
                            GType  g_type)
{
  struct FindByType data = { g_type, NULL};

  gthree_object_traverse (object,  find_by_type_cb, &data);
  return g_list_reverse (data.list);
}

struct FindByName {
  const char *name;
  GList *list;
};

static gboolean
find_by_name_cb (GthreeObject *object,
                 gpointer user_data)
{
  struct FindByName *data = user_data;
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (g_strcmp0 (data->name, priv->name) == 0 ||
      g_strcmp0 (data->name, priv->uuid) == 0)
    data->list = g_list_prepend (data->list, object);

  return TRUE;
}

GList *
gthree_object_find_by_name (GthreeObject *object,
                            const char *name)
{
  struct FindByName data = { name, NULL};

  gthree_object_traverse (object, find_by_name_cb, &data);
  return g_list_reverse (data.list);
}

struct FindFirstByName {
  const char *name;
  GthreeObject *object;
};

static gboolean
find_first_by_name_cb (GthreeObject *object,
                       gpointer user_data)
{
  struct FindFirstByName *data = user_data;
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);

  if (g_strcmp0 (data->name, priv->name) == 0 ||
      g_strcmp0 (data->name, priv->uuid) == 0)
    {
      data->object = object;
      return FALSE;
    }

  return TRUE;
}

GthreeObject *
gthree_object_find_first_by_name (GthreeObject *object,
                                  const char *name)
{
  struct FindFirstByName data = { name, NULL};

  gthree_object_traverse (object, find_first_by_name_cb, &data);
  return data.object;
}

void
gthree_object_print_tree (GthreeObject *object, int depth)
{
  GthreeObjectPrivate *priv = gthree_object_get_instance_private (object);
  GthreeObjectIter iter;
  GthreeObject *child;
  int i;

  for (i = 0; i < depth; i++)
    g_print ("  ");

  g_print ("%s(%p) %s", g_type_name_from_instance ((gpointer)object), object, priv->uuid);
  if (priv->name)
    g_print ("name: '%s'", priv->name);
  g_print ("\n");

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    gthree_object_print_tree (child, depth + 1);
}

static void
_gthree_object_get_mesh_extents (GthreeObject *object,
                                 graphene_box_t *box)
{
  GthreeObjectIter iter;
  GthreeObject *child;

  if (GTHREE_IS_MESH (object))
    {
      GthreeGeometry *geometry = gthree_mesh_get_geometry (GTHREE_MESH (object));
      graphene_box_t bounding_box;

      graphene_matrix_transform_box (gthree_object_get_world_matrix (object),
                                     gthree_geometry_get_bounding_box (geometry),
                                     &bounding_box);
      graphene_box_union (box, &bounding_box, box);
    }

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    _gthree_object_get_mesh_extents (child, box);
}

/* Note: This relies on the world matrix being up-to-date */
void
gthree_object_get_mesh_extents (GthreeObject *object,
                                graphene_box_t *box)
{
  graphene_box_init_from_box (box, graphene_box_empty ());
  _gthree_object_get_mesh_extents (object, box);
}
