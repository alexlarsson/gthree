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

GType gthree_animation_mixer_get_type (void) G_GNUC_CONST;

void               gthree_animation_mixer_activate_action            (GthreeAnimationMixer  *mixer,
                                                                      GthreeAnimationAction *action);
void               gthree_animation_mixer_deactivate_action          (GthreeAnimationMixer  *mixer,
                                                                      GthreeAnimationAction *action);
gboolean           gthree_animation_mixer_is_active_action           (GthreeAnimationMixer  *mixer,
                                                                      GthreeAnimationAction *action);
void               gthree_action_mixer_take_back_control_interpolant (GthreeAnimationMixer  *mixer,
                                                                      GthreeInterpolant     *interpolant);
GthreeInterpolant *gthree_action_mixer_lend_control_interpolant      (GthreeAnimationMixer  *mixer);
float              gthree_action_mixer_get_time                      (GthreeAnimationMixer  *mixer);
GthreeObject *     gthree_action_mixer_get_root                      (GthreeAnimationMixer  *mixer);

void gthree_action_mixer_displatch_event                      (GthreeAnimationMixer  *mixer,
                                                               const char *type /*, ... */);



G_END_DECLS

#endif /* __GTHREE_ANIMATION_MIXER_H__ */
