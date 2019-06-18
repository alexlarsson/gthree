#ifndef __GTHREE_NUMBER_KEYFRAME_TRACK_H__
#define __GTHREE_NUMBER_KEYFRAME_TRACK_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreekeyframetrack.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_NUMBER_KEYFRAME_TRACK      (gthree_number_keyframe_track_get_type ())
#define GTHREE_NUMBER_KEYFRAME_TRACK(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                 GTHREE_TYPE_NUMBER_KEYFRAME_TRACK, \
                                                 GthreeNumberKeyframeTrack))
#define GTHREE_IS_NUMBER_KEYFRAME_TRACK(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                 GTHREE_TYPE_NUMBER_KEYFRAME_TRACK))
#define GTHREE_NUMBER_KEYFRAME_TRACK_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_NUMBER_KEYFRAME_TRACK, GthreeNumberKeyframeTrackClass))


typedef struct {
  GthreeKeyframeTrack parent;
} GthreeNumberKeyframeTrack;

typedef struct {
  GthreeKeyframeTrackClass parent_class;
} GthreeNumberKeyframeTrackClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeNumberKeyframeTrack, g_object_unref)

GType gthree_number_keyframe_track_get_type (void) G_GNUC_CONST;

GthreeKeyframeTrack *gthree_number_keyframe_track_new (const char *name,
                                                       GthreeAttributeArray *times,
                                                       GthreeAttributeArray *values);

G_END_DECLS

#endif /* __GTHREE_NUMBER_KEYFRAME_TRACK_H__ */
