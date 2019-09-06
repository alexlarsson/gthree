/* gthreeanimationclip.h: Animation clip
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
#include <gthree/gthreeenums.h>
#include <gthree/gthreekeyframetrack.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_ANIMATION_CLIP (gthree_animation_clip_get_type())

GTHREE_API
G_DECLARE_DERIVABLE_TYPE (GthreeAnimationClip, gthree_animation_clip, GTHREE, ANIMATION_CLIP, GObject)

struct _GthreeAnimationClipClass
{
  GObjectClass parent_class;
};

GTHREE_API
GthreeAnimationClip *gthree_animation_clip_new                  (const char          *name,
                                                                 float                duration);
GTHREE_API
void                 gthree_animation_clip_add_track            (GthreeAnimationClip *clip,
                                                                 GthreeKeyframeTrack *track);
GTHREE_API
void                 gthree_animation_clip_reset_duration       (GthreeAnimationClip *clip);
GTHREE_API
void                 gthree_animation_clip_optimize             (GthreeAnimationClip *clip);
GTHREE_API
void                 gthree_animation_clip_trim                 (GthreeAnimationClip *clip);
GTHREE_API
const char *         gthree_animation_clip_get_name             (GthreeAnimationClip *clip);
GTHREE_API
float                gthree_animation_clip_get_duration         (GthreeAnimationClip *clip);
GTHREE_API
int                  gthree_animation_clip_get_n_tracks         (GthreeAnimationClip *clip);
GTHREE_API
GthreeKeyframeTrack *gthree_animation_clip_get_track            (GthreeAnimationClip *clip,
                                                                 int                  i);

G_END_DECLS
