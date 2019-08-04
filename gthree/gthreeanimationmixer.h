#ifndef __GTHREE_ANIMATION_MIXER_H__
#define __GTHREE_ANIMATION_MIXER_H__

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

#define GTHREE_TYPE_ANIMATION_MIXER      (gthree_animation_mixer_get_type ())
#define GTHREE_ANIMATION_MIXER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_ANIMATION_MIXER, \
                                                  GthreeAnimationMixer))
#define GTHREE_ANIMATION_MIXER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_ANIMATION_MIXER, GthreeAnimationMixerClass))
#define GTHREE_IS_ANIMATION_MIXER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_ANIMATION_MIXER))
#define GTHREE_ANIMATION_MIXER_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_ANIMATION_MIXER, GthreeAnimationMixerClass))


struct _GthreeAnimationMixer {
  GObject parent;
};

typedef struct {
  GObjectClass parent_class;
} GthreeAnimationMixerClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeAnimationMixer, g_object_unref)

GTHREE_API
GType gthree_animation_mixer_get_type (void) G_GNUC_CONST;

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

#endif /* __GTHREE_ANIMATION_MIXER_H__ */
