#include <math.h>

#include "gthreeanimationclip.h"
#include "gthreeattribute.h"
#include "gthreediscreteinterpolant.h"
#include "gthreelinearinterpolant.h"
#include "gthreecubicinterpolant.h"


typedef struct {
  char *name;
  float duration;
  GPtrArray *tracks;
} GthreeAnimationClipPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeAnimationClip, gthree_animation_clip, G_TYPE_OBJECT)

static void
gthree_animation_clip_init (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  priv->tracks = g_ptr_array_new_with_free_func (g_object_unref);
}

static void
gthree_animation_clip_finalize (GObject *obj)
{
  GthreeAnimationClip *clip = GTHREE_ANIMATION_CLIP (obj);
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  g_free (priv->name);
  g_ptr_array_unref (priv->tracks);

  G_OBJECT_CLASS (gthree_animation_clip_parent_class)->finalize (obj);
}

static void
gthree_animation_clip_class_init (GthreeAnimationClipClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_animation_clip_finalize;
}


GthreeAnimationClip *
gthree_animation_clip_new (const char *name, float duration)
{
  GthreeAnimationClip *clip = g_object_new (GTHREE_TYPE_ANIMATION_CLIP, NULL);
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  priv->name = g_strdup (name);
  priv->duration = duration;

  return clip;
}

void
gthree_animation_clip_add_track (GthreeAnimationClip *clip,
                                 GthreeKeyframeTrack *track)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  g_ptr_array_add (priv->tracks, g_object_ref (track));
}

void
gthree_animation_clip_reset_duration (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);
  int i;
  float duration = 0;

  for (i = 0; i < priv->tracks->len; i++)
    {
      GthreeKeyframeTrack *track = g_ptr_array_index (priv->tracks, i);
      float track_time = gthree_keyframe_track_get_end_time (track);
      duration = MAX (duration, track_time);
    }

  priv->duration = duration;
}

void
gthree_animation_clip_optimize (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);
  int i;

  for (i = 0; i < priv->tracks->len; i++)
    {
      GthreeKeyframeTrack *track = g_ptr_array_index (priv->tracks, i);
      gthree_keyframe_track_optimize (track);
    }
}

void
gthree_animation_clip_trim (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);
  int i;

  for (i = 0; i < priv->tracks->len; i++)
    {
      GthreeKeyframeTrack *track = g_ptr_array_index (priv->tracks, i);
      gthree_keyframe_track_trim (track, 0, priv->duration);
    }
}

const char *
gthree_animation_clip_get_name (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  return priv->name;
}

float
gthree_animation_clip_get_duration (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  return priv->duration;
}

int
gthree_animation_clip_get_n_tracks (GthreeAnimationClip *clip)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  return priv->tracks->len;
}

GthreeKeyframeTrack *
gthree_animation_clip_get_track (GthreeAnimationClip *clip,
                                 int                  i)
{
  GthreeAnimationClipPrivate *priv = gthree_animation_clip_get_instance_private (clip);

  return g_ptr_array_index (priv->tracks, i);
}
