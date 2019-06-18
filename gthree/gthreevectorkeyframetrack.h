#ifndef __GTHREE_VECTOR_KEYFRAME_TRACK_H__
#define __GTHREE_VECTOR_KEYFRAME_TRACK_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreekeyframetrack.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_VECTOR_KEYFRAME_TRACK      (gthree_vector_keyframe_track_get_type ())
#define GTHREE_VECTOR_KEYFRAME_TRACK(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                 GTHREE_TYPE_VECTOR_KEYFRAME_TRACK, \
                                                 GthreeVectorKeyframeTrack))
#define GTHREE_IS_VECTOR_KEYFRAME_TRACK(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                 GTHREE_TYPE_VECTOR_KEYFRAME_TRACK))
#define GTHREE_VECTOR_KEYFRAME_TRACK_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_VECTOR_KEYFRAME_TRACK, GthreeVectorKeyframeTrackClass))


typedef struct {
  GthreeKeyframeTrack parent;
} GthreeVectorKeyframeTrack;

typedef struct {
  GthreeKeyframeTrackClass parent_class;
} GthreeVectorKeyframeTrackClass;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeVectorKeyframeTrack, g_object_unref)

GType gthree_vector_keyframe_track_get_type (void) G_GNUC_CONST;

GthreeKeyframeTrack *gthree_vector_keyframe_track_new (const char *name,
                                                       GthreeAttributeArray *times,
                                                       GthreeAttributeArray *values);

G_END_DECLS

#endif /* __GTHREE_VECTOR_KEYFRAME_TRACK_H__ */
