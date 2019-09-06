/* gthreeanimationaction.h: Animation action
 *
 * Copyright 2019  Alexander Larsson
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreetypes.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeanimationclip.h>
#include <gthree/gthreeobject.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_ANIMATION_ACTION (gthree_animation_action_get_type())

GTHREE_API
G_DECLARE_DERIVABLE_TYPE (GthreeAnimationAction, gthree_animation_action,
                          GTHREE, ANIMATION_ACTION,
                          GObject)

struct _GthreeAnimationActionClass
{
  GObjectClass parent_class;
};

GTHREE_API
GthreeAnimationAction * gthree_animation_action_new (GthreeAnimationMixer *mixer,
                                                     GthreeAnimationClip *clip,
                                                     GthreeObject *local_root);

GTHREE_API
void                  gthree_animation_action_play                     (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_stop                     (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_reset                    (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_set_enabled              (GthreeAnimationAction *action,
                                                                        gboolean               enabled);
GTHREE_API
gboolean              gthree_animation_action_get_enabled              (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_set_paused               (GthreeAnimationAction *action,
                                                                        gboolean               paused);
GTHREE_API
gboolean              gthree_animation_action_get_paused               (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_set_time                 (GthreeAnimationAction *action,
                                                                        float                  time);
GTHREE_API
float                 gthree_animation_action_get_time                 (GthreeAnimationAction *action);
GTHREE_API
gboolean              gthree_animation_action_is_running               (GthreeAnimationAction *action);
GTHREE_API
gboolean              gthree_animation_action_is_scheduled             (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_start_at                 (GthreeAnimationAction *action,
                                                                        float                  time);
GTHREE_API
void                  gthree_animation_action_set_loop_mode            (GthreeAnimationAction *action,
                                                                        GthreeLoopMode         loop_mode,
                                                                        int                    repetitions);
GTHREE_API
void                  gthree_animation_action_set_effective_weight     (GthreeAnimationAction *action,
                                                                        float                  weight);
GTHREE_API
float                 gthree_animation_action_get_effective_weight     (GthreeAnimationAction *action);
GTHREE_API
float                 gthree_animation_action_get_weight               (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_fade_in                  (GthreeAnimationAction *action,
                                                                        float                  duration);
GTHREE_API
void                  gthree_animation_action_fade_out                 (GthreeAnimationAction *action,
                                                                        float                  duration);
GTHREE_API
void                  gthree_animation_action_cross_fade_from          (GthreeAnimationAction *action,
                                                                        GthreeAnimationAction *fade_out_action,
                                                                        float                  duration,
                                                                        gboolean               warp);
GTHREE_API
void                  gthree_animation_action_cross_fade_to            (GthreeAnimationAction *action,
                                                                        GthreeAnimationAction *fade_in_action,
                                                                        float                  duration,
                                                                        gboolean               warp);
GTHREE_API
void                  gthree_animation_action_stop_fading              (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_set_effective_time_scale (GthreeAnimationAction *action,
                                                                        float                  time_scale);
GTHREE_API
float                 gthree_animation_action_get_effective_time_scale (GthreeAnimationAction *action);
GTHREE_API
float                 gthree_animation_action_get_time_scale            (GthreeAnimationAction *action);
GTHREE_API
void                  gthree_animation_action_set_duration             (GthreeAnimationAction *action,
                                                                        float                  duration);
GTHREE_API
void                  gthree_animation_action_sync_with                (GthreeAnimationAction *action,
                                                                        GthreeAnimationAction *other_action);
GTHREE_API
void                  gthree_animation_action_halt                     (GthreeAnimationAction *action,
                                                                        float                  duration);
GTHREE_API
void                  gthree_animation_action_warp                     (GthreeAnimationAction *action,
                                                                        float                  start_time_scale,
                                                                        float                  end_time_scale,
                                                                        float                  duration);
GTHREE_API
void                  gthree_animation_action_stop_warping             (GthreeAnimationAction *action);
GTHREE_API
GthreeAnimationMixer *gthree_animation_action_get_mixer                (GthreeAnimationAction *action);
GTHREE_API
GthreeAnimationClip * gthree_animation_action_get_clip                 (GthreeAnimationAction *action);
GTHREE_API
GthreeObject *        gthree_animation_action_get_root                 (GthreeAnimationAction *action);

G_END_DECLS
