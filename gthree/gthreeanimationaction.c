#include <math.h>

#include "gthreeanimationaction.h"


typedef struct {
} GthreeAnimationActionPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeAnimationAction, gthree_animation_action, G_TYPE_OBJECT)

static void
gthree_animation_action_init (GthreeAnimationAction *action)
{
  //GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);
}

static void
gthree_animation_action_finalize (GObject *obj)
{
  //GthreeAnimationAction *action = GTHREE_ANIMATION_ACTION (obj);
  //GthreeAnimationActionPrivate *priv = gthree_animation_action_get_instance_private (action);

  G_OBJECT_CLASS (gthree_animation_action_parent_class)->finalize (obj);
}

static void
gthree_animation_action_class_init (GthreeAnimationActionClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_animation_action_finalize;
}
