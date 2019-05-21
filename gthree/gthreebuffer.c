#include <math.h>
#include <epoxy/gl.h>

#include "gthreebufferprivate.h"

G_DEFINE_TYPE (GthreeBuffer, gthree_buffer, G_TYPE_OBJECT)

GthreeBuffer *
gthree_buffer_new (void)
{
  return g_object_new (gthree_buffer_get_type (), NULL);
}

static void
gthree_buffer_init (GthreeBuffer *buffer)
{
}

void
gthree_buffer_unrealize (GthreeBuffer *buffer)
{
  buffer->realized = FALSE;

  if (buffer->vertex_buffer)
    {
      glDeleteBuffers (1, &buffer->vertex_buffer);
      buffer->vertex_buffer = 0;
    }
  if (buffer->normal_buffer)
    {
      glDeleteBuffers (1, &buffer->normal_buffer);
      buffer->normal_buffer = 0;
    }
  if (buffer->tangent_buffer)
    {
      glDeleteBuffers (1, &buffer->tangent_buffer);
      buffer->tangent_buffer = 0;
    }
  if (buffer->color_buffer)
    {
      glDeleteBuffers (1, &buffer->color_buffer);
      buffer->color_buffer = 0;
    }
  if (buffer->uv_buffer)
    {
      glDeleteBuffers (1, &buffer->uv_buffer);
      buffer->uv_buffer = 0;
    }
  if (buffer->uv2_buffer)
    {
      glDeleteBuffers (1, &buffer->uv2_buffer);
      buffer->uv2_buffer = 0;
    }
  if (buffer->line_distance_buffer)
    {
      glDeleteBuffers (1, &buffer->line_distance_buffer);
      buffer->line_distance_buffer = 0;
    }
  if (buffer->face_buffer)
    {
      glDeleteBuffers (1, &buffer->face_buffer);
      buffer->face_buffer = 0;
    }
  if (buffer->line_buffer)
    {
      glDeleteBuffers (1, &buffer->line_buffer);
      buffer->line_buffer = 0;
    }
}

static void
gthree_buffer_finalize (GObject *obj)
{
  GthreeBuffer *buffer = GTHREE_BUFFER (obj);
  g_assert (!buffer->realized);

  G_OBJECT_CLASS (gthree_buffer_parent_class)->finalize (obj);
}

static void
gthree_buffer_class_init (GthreeBufferClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_buffer_finalize;

}
