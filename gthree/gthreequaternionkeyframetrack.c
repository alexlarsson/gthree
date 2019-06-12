#include "gthreequaternionkeyframetrack.h"
#include "gthreeprivate.h"
#include "gthreequaternioninterpolant.h"

G_DEFINE_TYPE (GthreeQuaternionKeyframeTrack, gthree_quaternion_keyframe_track, GTHREE_TYPE_KEYFRAME_TRACK)

GthreeKeyframeTrack *
gthree_quaternion_keyframe_track_new (const char *name,
                                      GthreeAttributeArray *times,
                                      GthreeAttributeArray *values)
{
  GthreeKeyframeTrack *track;

  track = gthree_keyframe_track_create (GTHREE_TYPE_QUATERNION_KEYFRAME_TRACK,
                                        name, times, values);
  return track;
}

static void
gthree_quaternion_keyframe_track_init (GthreeQuaternionKeyframeTrack *keyframe_track)
{
}

static void
gthree_quaternion_keyframe_track_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_quaternion_keyframe_track_parent_class)->finalize (obj);
}

static GthreeInterpolant *
create_linear_interpolant (GthreeKeyframeTrack *track)
{
  return gthree_quaternion_interpolant_new (gthree_keyframe_track_get_times (track),
                                            gthree_keyframe_track_get_values (track));
}

static void
gthree_quaternion_keyframe_track_class_init (GthreeQuaternionKeyframeTrackClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_quaternion_keyframe_track_finalize;

  GTHREE_KEYFRAME_TRACK_CLASS(klass)->create_linear_interpolant = create_linear_interpolant;
}
