#include <math.h>
#include <epoxy/gl.h>

#include "gthreescene.h"
#include "gthreelight.h"

#include "gthreeobjectprivate.h"

typedef struct {
  GdkGLContext *context;
  gint context_count;
  GList *lights;
  GdkRGBA bg_color;
  gboolean bg_color_is_set;
  GthreeTexture *bg_texture;
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
  GthreeScene *scene = GTHREE_SCENE (obj);
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  // These should all have been freed during dispose
  g_assert (priv->lights == NULL);

  if (priv->bg_texture)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->bg_texture));
      g_clear_object (&priv->bg_texture);
    }

  G_OBJECT_CLASS (gthree_scene_parent_class)->finalize (obj);
}

const GdkRGBA *
gthree_scene_get_background_color (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  if (priv->bg_color_is_set)
    return &priv->bg_color;
  return NULL;
}

void
gthree_scene_set_background_color   (GthreeScene   *scene,
                                     GdkRGBA       *color)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  if (color != NULL)
    {
      priv->bg_color_is_set = TRUE;
      priv->bg_color = *color;
    }
  else
    priv->bg_color_is_set = FALSE;
}

GthreeTexture *
gthree_scene_get_background_texture (GthreeScene   *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  return priv->bg_texture;
}

void
gthree_scene_set_background_texture (GthreeScene   *scene,
                                     GthreeTexture *texture)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->bg_texture)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->bg_texture));

  g_object_ref (texture);
  g_clear_object (&priv->bg_texture);
  priv->bg_texture = texture;
}

static void
gthree_scene_added_child (GthreeObject *scene_obj,
                          GthreeObject *child)
{
  GthreeScene *scene = GTHREE_SCENE (scene_obj);
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  GthreeObjectIter iter;
  GthreeObject *grand_child;

  if (GTHREE_IS_LIGHT (child))
    priv->lights = g_list_prepend (priv->lights, child);

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
  GthreeObjectIter iter;
  GthreeObject *grand_child;

  if (GTHREE_IS_LIGHT (child))
    priv->lights = g_list_remove (priv->lights, child);

  gthree_object_iter_init (&iter, child);
  while (gthree_object_iter_next (&iter, &grand_child))
    gthree_scene_removed_child (scene_obj, grand_child);
}

GthreeMaterial *
gthree_scene_get_override_material (GthreeScene *scene)
{
  // TODO
  return NULL;
}

GdkGLContext *
gthree_scene_get_context (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  return priv->context;
}

void
gthree_scene_set_context (GthreeScene *scene,
                          GdkGLContext *context)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  if (context != NULL)
    {
      if (priv->context == NULL)
        {
          priv->context = g_object_ref (context);
          priv->context_count = 1;
        }
      else if (priv->context == context)
        priv->context_count++;
      else
        g_warning ("Can't use a GthreeScene with several contexts");
    }
  else
    {
      if (priv->context != NULL)
        {
          priv->context_count--;
          if (priv->context_count == 0)
            {
              /* TODO: Unrealize scene */
              g_object_unref (priv->context);
              priv->context = NULL;
            }
          else if (priv->context_count < 0)
            g_warning ("Non-paired context in GthreeScene");
        }
    }
}

GList *
gthree_scene_get_lights (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  return priv->lights;
}

static void
gthree_scene_class_init (GthreeSceneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_scene_finalize;

  GTHREE_OBJECT_CLASS (klass)->added_child = gthree_scene_added_child;
  GTHREE_OBJECT_CLASS (klass)->removed_child = gthree_scene_removed_child;
}
