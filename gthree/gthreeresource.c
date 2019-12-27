#include <math.h>
#include <epoxy/gl.h>

#include "gthreeresource.h"
#include "gthreeprivate.h"
#include "gthreeenums.h"

typedef struct {
  GthreeRenderer *renderer;
  gboolean used;
} GthreeResourcePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeResource, gthree_resource, G_TYPE_OBJECT);


static void
gthree_resource_init (GthreeResource *resource)
{
}

static void
gthree_resource_finalize (GObject *obj)
{
  GthreeResource *resource = GTHREE_RESOURCE (obj);
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  if (priv->renderer != NULL)
    gthree_resource_unrealize (resource, priv->renderer);

  g_assert (priv->renderer == NULL);

  G_OBJECT_CLASS (gthree_resource_parent_class)->finalize (obj);
}

static void
gthree_resource_real_set_used (GthreeResource *resource,
                               gboolean used)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  priv->used = used;
}

static void
gthree_resource_class_init (GthreeResourceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_resource_finalize;
  klass->set_used = gthree_resource_real_set_used;
}

gboolean
gthree_resource_is_realized (GthreeResource *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  return priv->renderer != NULL;
}

void
gthree_resource_set_realized_for (GthreeResource *resource,
                                  GthreeRenderer   *renderer)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  g_assert (renderer != NULL);
  g_assert (priv->renderer == NULL);

  priv->renderer = g_object_ref (renderer);

  gthree_renderer_mark_realized (renderer, resource);
}

void
gthree_resource_unrealize (GthreeResource *resource,
                           GthreeRenderer *renderer)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  GthreeResourceClass *class = GTHREE_RESOURCE_GET_CLASS(resource);

  g_assert (priv->renderer == renderer);

  class->unrealize (resource, renderer);

  gthree_renderer_mark_unrealized (renderer, resource);

  g_object_unref (priv->renderer);
  priv->renderer = NULL;
}

gboolean
gthree_resource_get_used (GthreeResource *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  return priv->used;
}

void
gthree_resource_set_used (GthreeResource *resource,
                          gboolean used)
{

  GthreeResourceClass *class = GTHREE_RESOURCE_GET_CLASS(resource);
  class->set_used (resource, used);
}
