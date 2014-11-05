#include <math.h>
#include <epoxy/gl.h>

#include "gthreebufferprivate.h"

G_DEFINE_TYPE (GthreeBuffer, gthree_buffer, G_TYPE_OBJECT);

GthreeBuffer *
gthree_buffer_new ()
{
  GthreeBuffer *buffer;

  buffer = g_object_new (gthree_buffer_get_type (),
                         NULL);

  return buffer;
}

static void
gthree_buffer_init (GthreeBuffer *buffer)
{

}

static void
gthree_buffer_finalize (GObject *obj)
{
  GthreeBuffer *buffer = GTHREE_BUFFER (obj);

  if (buffer->vertex_buffer)
    glDeleteBuffers (1, &buffer->vertex_buffer);
  if (buffer->normal_buffer)
    glDeleteBuffers (1, &buffer->normal_buffer);
  if (buffer->tangent_buffer)
    glDeleteBuffers (1, &buffer->tangent_buffer);
  if (buffer->color_buffer)
    glDeleteBuffers (1, &buffer->color_buffer);
  if (buffer->uv_buffer)
    glDeleteBuffers (1, &buffer->uv_buffer);
  if (buffer->uv2_buffer)
    glDeleteBuffers (1, &buffer->uv2_buffer);
  if (buffer->line_distance_buffer)
    glDeleteBuffers (1, &buffer->line_distance_buffer);
  if (buffer->face_buffer)
    glDeleteBuffers (1, &buffer->face_buffer);
  if (buffer->line_buffer)
    glDeleteBuffers (1, &buffer->line_buffer);

  G_OBJECT_CLASS (gthree_buffer_parent_class)->finalize (obj);
}

static void
gthree_buffer_class_init (GthreeBufferClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_buffer_finalize;

}
