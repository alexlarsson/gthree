#include <math.h>
#include <epoxy/gl.h>

#include "gthreescene.h"

#include "gthreeobjectprivate.h"

typedef struct {
  int dummy;
} GthreeScenePrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeScene, gthree_scene, GTHREE_TYPE_OBJECT);

GthreeScene *
gthree_scene_new ()
{
  GthreeScene *scene;

  scene = g_object_new (gthree_scene_get_type (),
                         NULL);

  return scene;
}

static void
gthree_scene_init (GthreeScene *scene)
{
  gthree_object_set_matrix_auto_update (GTHREE_OBJECT (scene), FALSE);
}

static void
gthree_scene_finalize (GObject *obj)
{
  //GthreeScene *scene = GTHREE_SCENE (obj);
  //GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  G_OBJECT_CLASS (gthree_scene_parent_class)->finalize (obj);
}

static void
gthree_scene_class_init (GthreeSceneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_scene_finalize;
}
