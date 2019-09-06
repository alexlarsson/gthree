/* gthreeanimationmixer.h: Animation mixer
 *
 * Copyright 2019  Alex Larsson
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
#include <gthree/gthreeinterpolant.h>
#include <gthree/gthreeobject.h>
#include <gthree/gthreeanimationclip.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_ANIMATION_MIXER (gthree_animation_mixer_get_type())

GTHREE_API
G_DECLARE_DERIVABLE_TYPE (GthreeAnimationMixer, gthree_animation_mixer, GTHREE, ANIMATION_MIXER, GObject)

struct _GthreeAnimationMixerClass
{
  GObjectClass parent_class;
};

GTHREE_API
GthreeAnimationMixer *gthree_animation_mixer_new (GthreeObject *root);

GTHREE_API
GthreeAnimationAction *gthree_animation_mixer_clip_action     (GthreeAnimationMixer *mixer,
                                                               GthreeAnimationClip  *clip,
                                                               GthreeObject         *optional_root);
GTHREE_API
GthreeAnimationAction *gthree_animation_mixer_existing_action (GthreeAnimationMixer *mixer,
                                                               GthreeAnimationClip  *clip,
                                                               GthreeObject         *optional_root);
GTHREE_API
void                   gthree_animation_mixer_stop_all_action (GthreeAnimationMixer *mixer);
GTHREE_API
void                   gthree_animation_mixer_update          (GthreeAnimationMixer *mixer,
                                                               float                 delta_time);
GTHREE_API
void                   gthree_animation_mixer_uncache_clip    (GthreeAnimationMixer *mixer,
                                                               GthreeAnimationClip  *clip);
GTHREE_API
void                   gthree_animation_mixer_uncache_root    (GthreeAnimationMixer *mixer,
                                                               GthreeObject         *object);
GTHREE_API
void                   gthree_animation_mixer_uncache_action  (GthreeAnimationMixer *mixer,
                                                               GthreeAnimationClip  *clip,
                                                               GthreeObject         *optional_root);
GTHREE_API
float                  gthree_animation_mixer_get_time        (GthreeAnimationMixer *mixer);
GTHREE_API
void                   gthree_animation_mixer_set_time        (GthreeAnimationMixer *mixer,
                                                               float                 time);
GTHREE_API
float                  gthree_animation_mixer_get_time_scale  (GthreeAnimationMixer *mixer);
GTHREE_API
void                   gthree_animation_mixer_set_time_scale  (GthreeAnimationMixer *mixer,
                                                               float                 time_scale);
GTHREE_API
GthreeObject *         gthree_animation_mixer_get_root        (GthreeAnimationMixer *mixer);

G_END_DECLS
