#ifndef __GTHREE_ANIMATION_ACTION_H__
#define __GTHREE_ANIMATION_ACTION_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreetypes.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeanimationclip.h>
#include <gthree/gthreeobject.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_ANIMATION_ACTION      (gthree_animation_action_get_type ())
#define GTHREE_ANIMATION_ACTION(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_ANIMATION_ACTION, \
                                                  GthreeAnimationAction))
#define GTHREE_ANIMATION_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_ANIMATION_ACTION, GthreeAnimationActionClass))
#define GTHREE_IS_ANIMATION_ACTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_ANIMATION_ACTION))
#define GTHREE_ANIMATION_ACTION_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_ANIMATION_ACTION, GthreeAnimationActionClass))


struct _GthreeAnimationAction {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;
} GthreeAnimationActionClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAnimationAction, g_object_unref)

GType gthree_animation_action_get_type (void) G_GNUC_CONST;

GthreeAnimationAction * gthree_animation_action_new (GthreeAnimationMixer *mixer,
                                                     GthreeAnimationClip *clip,
                                                     GthreeObject *local_root);

void                  gthree_animation_action_play                     (GthreeAnimationAction *action);
void                  gthree_animation_action_stop                     (GthreeAnimationAction *action);
void                  gthree_animation_action_reset                    (GthreeAnimationAction *action);
gboolean              gthree_animation_action_is_running               (GthreeAnimationAction *action);
gboolean              gthree_animation_action_is_scheduled             (GthreeAnimationAction *action);
void                  gthree_animation_action_start_at                 (GthreeAnimationAction *action,
                                                                        float                  time);
void                  gthree_animation_action_set_loop_mode            (GthreeAnimationAction *action,
                                                                        GthreeLoopMode         loop_mode,
                                                                        int                    repetitions);
void                  gthree_animation_action_set_effective_weight     (GthreeAnimationAction *action,
                                                                        float                  weight);
float                 gthree_animation_action_get_effective_weight     (GthreeAnimationAction *action);
void                  gthree_animation_action_fade_in                  (GthreeAnimationAction *action,
                                                                        float                  duration);
void                  gthree_animation_action_fade_out                 (GthreeAnimationAction *action,
                                                                        float                  duration);
void                  gthree_animation_action_cross_fade_from          (GthreeAnimationAction *action,
                                                                        GthreeAnimationAction *fade_out_action,
                                                                        float                  duration,
                                                                        gboolean               warp);
void                  gthree_animation_action_cross_fade_to            (GthreeAnimationAction *action,
                                                                        GthreeAnimationAction *fade_in_action,
                                                                        float                  duration,
                                                                        gboolean               warp);
void                  gthree_animation_action_stop_fading              (GthreeAnimationAction *action);
void                  gthree_animation_action_set_effective_time_scale (GthreeAnimationAction *action,
                                                                        float                  time_scale);
float                 gthree_animation_action_get_effective_time_scale (GthreeAnimationAction *action);
void                  gthree_animation_action_set_duration             (GthreeAnimationAction *action,
                                                                        float                  duration);
void                  gthree_animation_action_sync_with                (GthreeAnimationAction *action,
                                                                        GthreeAnimationAction *other_action);
void                  gthree_animation_action_halt                     (GthreeAnimationAction *action,
                                                                        float                  duration);
void                  gthree_animation_action_warp                     (GthreeAnimationAction *action,
                                                                        float                  start_time_scale,
                                                                        float                  end_time_scale,
                                                                        float                  duration);
void                  gthree_animation_action_stop_warping             (GthreeAnimationAction *action);
GthreeAnimationMixer *gthree_animation_action_get_mixer                (GthreeAnimationAction *action);
GthreeAnimationClip * gthree_animation_action_get_clip                 (GthreeAnimationAction *action);
GthreeObject *        gthree_animation_action_get_root                 (GthreeAnimationAction *action);


G_END_DECLS

#endif /* __GTHREE_ANIMATION_ACTION_H__ */
