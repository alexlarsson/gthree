#include <math.h>
#include <epoxy/gl.h>

#include "gthreescene.h"
#include "gthreelight.h"

#include "gthreeobjectprivate.h"

typedef struct {
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

  return g_object_ref_sink (scene);
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

GthreeMaterial *
gthree_scene_get_override_material (GthreeScene *scene)
{
  // TODO
  return NULL;
}

static void
gthree_scene_class_init (GthreeSceneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_scene_finalize;
}
