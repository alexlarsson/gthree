#ifndef __GTHREE_KEYFRAME_TRACK_H__
#define __GTHREE_KEYFRAME_TRACK_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthreeenums.h>
#include <gthreeinterpolant.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_KEYFRAME_TRACK      (gthree_keyframe_track_get_type ())
#define GTHREE_KEYFRAME_TRACK(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_KEYFRAME_TRACK, \
                                                  GthreeKeyframeTrack))
#define GTHREE_KEYFRAME_TRACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_KEYFRAME_TRACK, GthreeKeyframeTrackClass))
#define GTHREE_IS_KEYFRAME_TRACK(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_KEYFRAME_TRACK))
#define GTHREE_KEYFRAME_TRACK_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_KEYFRAME_TRACK, GthreeKeyframeTrackClass))


typedef struct {
  GObject parent;
} GthreeKeyframeTrack;

typedef struct {
  GObjectClass parent_class;

  GthreeInterpolationMode default_interpolation_mode;
  GthreeInterpolant * (*create_discrete_interpolant) (GthreeKeyframeTrack *track);
  GthreeInterpolant * (*create_linear_interpolant) (GthreeKeyframeTrack *track);
  GthreeInterpolant * (*create_smooth_interpolant) (GthreeKeyframeTrack *track);

} GthreeKeyframeTrackClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeKeyframeTrack, g_object_unref)

GType gthree_keyframe_track_get_type (void) G_GNUC_CONST;

const char *          gthree_keyframe_track_get_name           (GthreeKeyframeTrack     *track);
GthreeAttributeArray *gthree_keyframe_track_get_times          (GthreeKeyframeTrack     *track);
GthreeAttributeArray *gthree_keyframe_track_get_values         (GthreeKeyframeTrack     *track);
GthreeInterpolant *   gthree_keyframe_track_create_interpolant (GthreeKeyframeTrack     *track);
void                  gthree_keyframe_track_optimize           (GthreeKeyframeTrack     *track);
void                  gthree_keyframe_track_scale              (GthreeKeyframeTrack     *track,
                                                                float                    time_scale);
void                  gthree_keyframe_track_set_interpolation  (GthreeKeyframeTrack     *track,
                                                                GthreeInterpolationMode  interpolation);
void                  gthree_keyframe_track_trim               (GthreeKeyframeTrack     *track,
                                                                float                    start_time,
                                                                float                    end_time);

G_END_DECLS

#endif /* __GTHREE_KEYFRAME_TRACK_H__ */
