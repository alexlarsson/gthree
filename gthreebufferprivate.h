#ifndef __GTHREE_BUFFER_H__
#define __GTHREE_BUFFER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_BUFFER      (gthree_buffer_get_type ())
#define GTHREE_BUFFER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_BUFFER, \
                                                             GthreeBuffer))
#define GTHREE_IS_BUFFER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_BUFFER))

typedef struct {
  GObject parent;

  guint vertex_buffer;
  guint normal_buffer;
  guint tangent_buffer;
  guint color_buffer;
  guint uv_buffer;
  guint uv2_buffer;
  guint line_distance_buffer;

  guint face_buffer;
  guint line_buffer;
} GthreeBuffer;

typedef struct {
  GObjectClass parent_class;

} GthreeBufferClass;

GthreeBuffer *gthree_buffer_new ();
GType gthree_buffer_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GTHREE_BUFFER_H__ */
