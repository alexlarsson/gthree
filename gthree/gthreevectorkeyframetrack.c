#include "gthreevectorkeyframetrack.h"
#include "gthreeprivate.h"

G_DEFINE_TYPE (GthreeVectorKeyframeTrack, gthree_vector_keyframe_track, GTHREE_TYPE_KEYFRAME_TRACK)

GthreeKeyframeTrack *
gthree_vector_keyframe_track_new (const char *name,
                                  GthreeAttributeArray *times,
                                  GthreeAttributeArray *values)
{
  GthreeKeyframeTrack *track;

  track = gthree_keyframe_track_create (GTHREE_TYPE_VECTOR_KEYFRAME_TRACK,
                                        name, times, values);
  return track;
}

static void
gthree_vector_keyframe_track_init (GthreeVectorKeyframeTrack *keyframe_track)
{
}

static void
gthree_vector_keyframe_track_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_vector_keyframe_track_parent_class)->finalize (obj);
}

static void
gthree_vector_keyframe_track_class_init (GthreeVectorKeyframeTrackClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_vector_keyframe_track_finalize;
  GTHREE_KEYFRAME_TRACK_CLASS(klass)->value_type = GTHREE_VALUE_TYPE_VECTOR;
}
