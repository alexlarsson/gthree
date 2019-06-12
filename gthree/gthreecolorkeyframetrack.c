#include "gthreecolorkeyframetrack.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeColorKeyframeTrack, gthree_color_keyframe_track, GTHREE_TYPE_KEYFRAME_TRACK)

GthreeKeyframeTrack *
gthree_color_keyframe_track_new (const char *name,
                                  GthreeAttributeArray *times,
                                  GthreeAttributeArray *values)
{
  GthreeKeyframeTrack *track;

  track = gthree_keyframe_track_create (GTHREE_TYPE_COLOR_KEYFRAME_TRACK,
                                        name, times, values);
  return track;
}

static void
gthree_color_keyframe_track_init (GthreeColorKeyframeTrack *keyframe_track)
{
}

static void
gthree_color_keyframe_track_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_color_keyframe_track_parent_class)->finalize (obj);
}

static void
gthree_color_keyframe_track_class_init (GthreeColorKeyframeTrackClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_color_keyframe_track_finalize;
}
