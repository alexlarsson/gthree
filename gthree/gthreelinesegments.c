#include <math.h>
#include <epoxy/gl.h>

#include "gthreelinesegments.h"
#include "gthreemeshbasicmaterial.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeLineSegments, gthree_line_segments, GTHREE_TYPE_LINE)

GthreeLineSegments *
gthree_line_segments_new (GthreeGeometry *geometry,
                          GthreeMaterial *material)
{
  return g_object_new (gthree_line_segments_get_type (),
                       "geometry", geometry,
                       "material", material,
                       NULL);
}

static void
gthree_line_segments_init (GthreeLineSegments *line_segments)
{
}

static void
gthree_line_segments_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_line_segments_parent_class)->finalize (obj);
}

static void
gthree_line_segments_class_init (GthreeLineSegmentsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_line_segments_finalize;
}
