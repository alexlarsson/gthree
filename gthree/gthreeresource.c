#include <math.h>
#include <epoxy/gl.h>

#include "gthreeresource.h"
#include "gthreeprivate.h"
#include "gthreeenums.h"

typedef struct {
  GArray *realize_data;
  gboolean used;
} GthreeResourcePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeResource, gthree_resource, G_TYPE_OBJECT);


static void
gthree_resource_init (GthreeResource *resource)
{
}

/* Assumes the data exists */
static GthreeResourceRealizeData *
gthree_resource_peek_data_at (GthreeResource  *resource, guint32 id)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  GthreeResourceClass *klass = GTHREE_RESOURCE_GET_CLASS(resource);

  return (GthreeResourceRealizeData *)(priv->realize_data->data + id * klass->realize_data_size);
}

static guint32
gthree_resource_get_n_realize_data (GthreeResource  *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  if (priv->realize_data)
    return priv->realize_data->len;

  return 0;
}

/* returns NULL if not exists */
static GthreeResourceRealizeData *
gthree_resource_peek_data_for (GthreeResource  *resource,
                               GthreeRenderer   *renderer)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  guint32 id = gthree_renderer_get_resource_id (renderer);

  if (priv->realize_data == NULL ||
      priv->realize_data->len < id + 1)
    return NULL;

  return gthree_resource_peek_data_at (resource, id);
}

/* Creates data if needed */
static GthreeResourceRealizeData *
gthree_resource_get_data_at (GthreeResource  *resource, guint32 id)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  if (priv->realize_data == NULL)
    priv->realize_data = g_array_new (FALSE, TRUE, GTHREE_RESOURCE_GET_CLASS(resource)->realize_data_size);

  if (priv->realize_data->len < id + 1)
    g_array_set_size (priv->realize_data, id + 1);

  return gthree_resource_peek_data_at (resource, id);
}

gpointer
gthree_resource_get_data_for (GthreeResource  *resource,
                              GthreeRenderer   *renderer)
{
  return gthree_resource_get_data_at (resource, gthree_renderer_get_resource_id (renderer));
}

static void
gthree_resource_finalize (GObject *obj)
{
  GthreeResource *resource = GTHREE_RESOURCE (obj);
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  guint32 id, n_data;

  n_data = gthree_resource_get_n_realize_data (resource);
  for (id = 0; id < n_data; id++)
    {
      GthreeResourceRealizeData *data = gthree_resource_peek_data_at (resource, id);

      if (data->realized_for)
        gthree_resource_unrealize (resource, data->realized_for);
    }

  if (priv->realize_data)
    g_array_unref (priv->realize_data);

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

  klass->realize_data_size = sizeof (GthreeResourceRealizeData);

  gobject_class->finalize = gthree_resource_finalize;
  klass->set_used = gthree_resource_real_set_used;
}

gboolean
gthree_resource_is_realized_for (GthreeResource  *resource,
                                 GthreeRenderer   *renderer)
{
  GthreeResourceRealizeData *data = gthree_resource_peek_data_for (resource, renderer);

  return data != NULL && data->realized_for != NULL;
}

void
gthree_resource_set_realized_for (GthreeResource *resource,
                                  GthreeRenderer *renderer)
{
  GthreeResourceRealizeData *data;

  g_assert (renderer != NULL);

  data = gthree_resource_get_data_for (resource, renderer);

  g_assert (data->realized_for == NULL);

  /* We don't need a ref here, beause we know that if the renderere is
     finalized it must be unrealized first which will NULL this out */
  data->realized_for = renderer;
  data->dirty = TRUE;
  gthree_renderer_mark_realized (renderer, resource);
}

void
gthree_resource_mark_dirty (GthreeResource *resource)
{
  guint32 id, n_data;

  n_data = gthree_resource_get_n_realize_data (resource);
  for (id = 0; id < n_data; id++)
    {
      GthreeResourceRealizeData *data = gthree_resource_peek_data_at (resource, id);

      if (data->realized_for)
        data->dirty = TRUE;
    }
}

void
gthree_resource_mark_clean_for (GthreeResource *resource,
                                GthreeRenderer *renderer)
{
  GthreeResourceRealizeData *data = gthree_resource_peek_data_for (resource, renderer);

  if (data)
    data->dirty = FALSE;
}

gboolean
gthree_resource_get_dirty_for (GthreeResource  *resource,
                               GthreeRenderer   *renderer)
{
  GthreeResourceRealizeData *data = gthree_resource_peek_data_for (resource, renderer);

  return
    data != NULL &&
    data->realized_for != NULL &&
    data->dirty;
}


void
gthree_resource_unrealize (GthreeResource *resource,
                           GthreeRenderer *renderer)
{
  GthreeResourceClass *class = GTHREE_RESOURCE_GET_CLASS(resource);

  class->unrealize (resource, renderer);

  gthree_renderer_mark_unrealized (renderer, resource);
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
