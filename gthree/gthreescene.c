#include <math.h>
#include <epoxy/gl.h>

#include "gthreescene.h"
#include "gthreelight.h"

#include "gthreeobjectprivate.h"

typedef struct {
  graphene_vec3_t bg_color;
  gboolean bg_color_is_set;
  float bg_alpha;
  GthreeTexture *bg_texture;
  GthreeMaterial *override_material;
  GthreeFog *fog;
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
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  priv->bg_alpha = -1;
  gthree_object_set_matrix_auto_update (GTHREE_OBJECT (scene), FALSE);
}

static void
gthree_scene_finalize (GObject *obj)
{
  GthreeScene *scene = GTHREE_SCENE (obj);
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  g_clear_object (&priv->bg_texture);

  g_clear_object (&priv->override_material);

  G_OBJECT_CLASS (gthree_scene_parent_class)->finalize (obj);
}

const graphene_vec3_t *
gthree_scene_get_background_color (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  if (priv->bg_color_is_set)
    return &priv->bg_color;
  return NULL;
}

void
gthree_scene_set_background_color (GthreeScene   *scene,
                                   const graphene_vec3_t *color)
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

/* negative alpha means unset */
float
gthree_scene_get_background_alpha (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  return priv->bg_alpha;
}

/* negative alpha means unset */
void
gthree_scene_set_background_alpha (GthreeScene   *scene,
                                   float alpha)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);
  priv->bg_alpha = alpha;
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

  g_object_ref (texture);
  g_clear_object (&priv->bg_texture);
  priv->bg_texture = texture;
}

GthreeMaterial *
gthree_scene_get_override_material (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  return priv->override_material;
}

void
gthree_scene_set_override_material (GthreeScene *scene,
                                    GthreeMaterial *material)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  if (material)
    g_object_ref (material);
  g_clear_object (&priv->override_material);
  priv->override_material = material;
}

GthreeFog *
gthree_scene_get_fog (GthreeScene *scene)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  return priv->fog;
}

void
gthree_scene_set_fog (GthreeScene *scene,
                      GthreeFog *fog)
{
  GthreeScenePrivate *priv = gthree_scene_get_instance_private (scene);

  g_set_object (&priv->fog, fog);
}

static void
gthree_scene_class_init (GthreeSceneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_scene_finalize;
}
