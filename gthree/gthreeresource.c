#include <math.h>
#include <epoxy/gl.h>

#include "gthreeresource.h"
#include "gthreeprivate.h"
#include "gthreeenums.h"

typedef struct _ListNode ListNode;

/* This is a circular doubly linked list where the first is the head */
struct _ListNode {
  ListNode *next;
  ListNode *prev;
};

static ListNode *
list_init (ListNode *head)
{
  head->next = head;
  head->prev = head;
  return head;
}

ListNode *
list_head_new (void)
{
  ListNode *head = g_new0 (ListNode, 1);
  return list_init (head);
}

static void
list_node_insert_before (ListNode *old_node,
                         ListNode *new_node)
{
  ListNode *old_prev = old_node->prev;

  new_node->next = old_node;
  new_node->prev = old_prev;

  old_node->prev = new_node;
  old_prev->next = new_node;
}

static void
list_node_unlink (ListNode *node)
{
  ListNode *prev = node->prev;
  ListNode *next = node->next;

  g_assert (node->next != NULL);
  g_assert (node->prev != NULL);

  prev->next = next;
  next->prev = next;

  node->prev = NULL;
  node->next = NULL;
}

static void
list_append (ListNode *head,
             ListNode *element)
{
  list_node_insert_before (head, element);
}

static GQuark used_list_head_q;
static GQuark unused_list_head_q;

static void
gl_context_init (void)
{
  used_list_head_q = g_quark_from_static_string ("gthree-resource-used-head");
  unused_list_head_q = g_quark_from_static_string ("gthree-resource-unused-head");
}

static ListNode *
gl_context_get_used_list_head (GdkGLContext  *context)
{
  ListNode *head;

  head = g_object_get_qdata (G_OBJECT (context), used_list_head_q);
  if (head == NULL)
    {
      head = list_head_new ();
      g_object_set_qdata_full (G_OBJECT (context),  used_list_head_q, head, g_free);
    }

  return head;
}

static ListNode *
gl_context_get_unused_list_head (GdkGLContext  *context)
{
  ListNode *head;

  head = g_object_get_qdata (G_OBJECT (context), unused_list_head_q);
  if (head == NULL)
    {
      head = list_head_new ();
      g_object_set_qdata_full (G_OBJECT (context),  unused_list_head_q, head, g_free);
    }

  return head;
}

static ListNode *
gl_context_get_list_head (GdkGLContext  *context,
                          gboolean used)
{
  if (used)
    return gl_context_get_used_list_head (context);
  else
    return gl_context_get_unused_list_head (context);
}

typedef struct {
  GdkGLContext *gl_context;
  int use_count;

  ListNode resource_list;
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

  g_assert (priv->gl_context == NULL);
  g_assert (priv->use_count == 0);

  G_OBJECT_CLASS (gthree_resource_parent_class)->finalize (obj);
}

static void
gthree_resource_class_init (GthreeResourceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gl_context_init ();

  gobject_class->finalize = gthree_resource_finalize;
}

gboolean
gthree_resource_is_realized (GthreeResource *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  return priv->gl_context != NULL;
}

static GthreeResource *
node_to_resource (ListNode *node)
{
  GthreeResource *resource;

  gsize offset = G_STRUCT_OFFSET(GthreeResourcePrivate, resource_list) + GthreeResource_private_offset;

  resource = G_STRUCT_MEMBER_P (node, -offset);

  /* TODO: Remove checks */
  {
    GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
    g_assert (&priv->resource_list == node);
  }

  return resource;
}

void
gthree_resources_unrealize_all_for (GdkGLContext *context)
{
  ListNode *head, *node;

  g_assert (gdk_gl_context_get_current () == context);

  gthree_resources_unrealize_unused_for (context);

  head = gl_context_get_used_list_head (context);
  node = head->next;
  while (node != head)
    {
      GthreeResource *resource = node_to_resource (node);

      /* Step to next before unrealizing and unlinking */
      node = node->next;

      gthree_resource_unrealize (resource);
    }
}

void
gthree_resources_unrealize_unused_for (GdkGLContext *context)
{
  ListNode *head, *node;

  g_assert (gdk_gl_context_get_current () == context);

  head = gl_context_get_unused_list_head (context);
  node = head->next;
  while (node != head)
    {
      GthreeResource *resource = node_to_resource (node);

      /* Step to next before unrealizing and unlinking */
      node = node->next;

      gthree_resource_unrealize (resource);
    }
}

void
gthree_resource_set_realized_for (GthreeResource *resource,
                                  GdkGLContext   *context)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  ListNode *head;

  g_assert (context != NULL);
  g_assert (priv->gl_context == NULL);

  priv->gl_context = g_object_ref (context);

  head = gl_context_get_list_head (context, priv->use_count > 0);
  list_append (head, &priv->resource_list);

  /* The context keeps this alive this until unrealize to avoid it being finalized without unrealizing.
   * This is a circular reference, but its broken by the explicit unrealize call. */
  g_object_ref (resource);
}

void
gthree_resource_unrealize (GthreeResource *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);
  GthreeResourceClass *class = GTHREE_RESOURCE_GET_CLASS(resource);

  g_assert (priv->gl_context != NULL);
  g_assert (gdk_gl_context_get_current () == priv->gl_context);

  class->unrealize (resource);

  list_node_unlink (&priv->resource_list);

  g_object_unref (priv->gl_context);
  priv->gl_context = NULL;

  g_object_unref (resource); /* Drop ownership from context */
}


void
gthree_resource_use (GthreeResource *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  if (priv->gl_context && priv->use_count == 0)
    {
      list_node_unlink (&priv->resource_list);
      list_append (gl_context_get_used_list_head (priv->gl_context),
                   &priv->resource_list);
    }

  priv->use_count++;
}

void
gthree_resource_unuse (GthreeResource *resource)
{
  GthreeResourcePrivate *priv = gthree_resource_get_instance_private (resource);

  g_assert (priv->use_count > 0);

  priv->use_count--;

  if (priv->gl_context && priv->use_count == 0 )
    {
      list_node_unlink (&priv->resource_list);
      list_append (gl_context_get_unused_list_head (priv->gl_context),
                   &priv->resource_list);
    }
}
