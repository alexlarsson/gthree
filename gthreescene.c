#include <math.h>
#include <epoxy/gl.h>

#include "gthreescene.h"

#include "gthreeobjectprivate.h"

typedef struct {
  GList *added_objects;
  GList *removed_objects;
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
gthree_scene_added_child (GthreeObject *scene_obj,
                          GthreeObject *child)
{
  GthreeScene *scene = GTHREE_SCENE (scene_obj);
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  GList *found;
  GthreeObjectIter iter;
  GthreeObject *grand_child;

  priv->added_objects = g_list_prepend (priv->added_objects, child);

  found = g_list_find (priv->removed_objects, child);
  if (found)
    priv->removed_objects = g_list_remove_link (priv->removed_objects, found);

  gthree_object_iter_init (&iter, child);
  while (gthree_object_iter_next (&iter, &grand_child))
    gthree_scene_added_child (scene_obj, grand_child);
}

static void
gthree_scene_removed_child (GthreeObject *scene_obj,
                           GthreeObject *child)
{
  GthreeScene *scene = GTHREE_SCENE (scene_obj);
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  GList *found;
  GthreeObjectIter iter;
  GthreeObject *grand_child;

  priv->removed_objects = g_list_prepend (priv->removed_objects, child);

  found = g_list_find (priv->added_objects, child);
  if (found)
    priv->added_objects = g_list_remove_link (priv->added_objects, found);

  gthree_object_iter_init (&iter, child);
  while (gthree_object_iter_next (&iter, &grand_child))
    gthree_scene_removed_child (scene_obj, grand_child);
}

void
gthree_scene_realize_objects (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  GList *l;

  for (l = priv->added_objects; l != NULL; l = l->next)
    gthree_object_realize (l->data);

  g_list_free (priv->added_objects);
  priv->added_objects = NULL;

  for (l = priv->removed_objects; l != NULL; l = l->next)
    gthree_object_unrealize (l->data);

  g_list_free (priv->removed_objects);
  priv->removed_objects = NULL;
}

static void
gthree_scene_class_init (GthreeSceneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_scene_finalize;

  GTHREE_OBJECT_CLASS (klass)->added_child = gthree_scene_added_child;
  GTHREE_OBJECT_CLASS (klass)->removed_child = gthree_scene_removed_child;
}
