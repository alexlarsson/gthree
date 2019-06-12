#ifndef __GTHREE_ANIMATION_CLIP_H__
#define __GTHREE_ANIMATION_CLIP_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthreeenums.h>
#include <gthreekeyframetrack.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_ANIMATION_CLIP      (gthree_animation_clip_get_type ())
#define GTHREE_ANIMATION_CLIP(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_ANIMATION_CLIP, \
                                                  GthreeAnimationClip))
#define GTHREE_ANIMATION_CLIP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_ANIMATION_CLIP, GthreeAnimationClipClass))
#define GTHREE_IS_ANIMATION_CLIP(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_ANIMATION_CLIP))
#define GTHREE_ANIMATION_CLIP_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_ANIMATION_CLIP, GthreeAnimationClipClass))


typedef struct {
  GObject parent;
} GthreeAnimationClip;

typedef struct {
  GObjectClass parent_class;
} GthreeAnimationClipClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAnimationClip, g_object_unref)

GType gthree_animation_clip_get_type (void) G_GNUC_CONST;

GthreeAnimationClip *gthree_animation_clip_new          (const char          *name,
                                                         float                duration);
void                 gthree_animation_clip_add_track    (GthreeAnimationClip *clip,
                                                         GthreeKeyframeTrack *track);
void                 gthree_animation_reset_duration    (GthreeAnimationClip *clip);
void                 gthree_animation_optimize          (GthreeAnimationClip *clip);
void                 gthree_animation_trim              (GthreeAnimationClip *clip);
const char *         gthree_animation_clip_get_name     (GthreeAnimationClip *clip);
float                gthree_animation_clip_get_duration (GthreeAnimationClip *clip);
int                  gthree_animation_clip_get_n_tracks (GthreeAnimationClip *clip);
GthreeKeyframeTrack *gthree_animation_clip_get_track    (GthreeAnimationClip *clip,
                                                         int                  i);

G_END_DECLS

#endif /* __GTHREE_ANIMATION_CLIP_H__ */
