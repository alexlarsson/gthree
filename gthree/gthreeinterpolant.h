#ifndef __GTHREE_INTERPOLANT_H__
#define __GTHREE_INTERPOLANT_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreeattribute.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_INTERPOLANT            (gthree_interpolant_get_type ())
#define GTHREE_INTERPOLANT(inst)           (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_INTERPOLANT, \
                                                             GthreeInterpolant))
#define GTHREE_INTERPOLANT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_INTERPOLANT, GthreeInterpolantClass))
#define GTHREE_IS_INTERPOLANT(inst)        (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_INTERPOLANT))
#define GTHREE_INTERPOLANT_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_INTERPOLANT, GthreeInterpolantClass))

typedef struct _GthreeInterpolantSettings GthreeInterpolantSettings;

GTHREE_API
GType gthree_interpolant_settings_get_type (void) G_GNUC_CONST;

typedef struct {
  GObject parent;
} GthreeInterpolant;

typedef struct {
  GObjectClass parent_class;

  void (*interval_changed) (GthreeInterpolant *interpolant, int i1, float t0, float t1);
  void (*interpolate) (GthreeInterpolant *interpolant, int i1, float t0, float t, float t1, GthreeAttributeArray *dest);

} GthreeInterpolantClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeInterpolant, g_object_unref)

GTHREE_API
GType gthree_interpolant_get_type (void) G_GNUC_CONST;

GTHREE_API
int                        gthree_interpolant_get_n_positions         (GthreeInterpolant *interpolant);
GTHREE_API
int                        gthree_interpolant_get_sample_size         (GthreeInterpolant *interpolant);
GTHREE_API
GthreeAttributeType        gthree_interpolant_get_sample_type         (GthreeInterpolant *interpolant);
GTHREE_API
GthreeAttributeArray *     gthree_interpolant_get_parameter_positions (GthreeInterpolant *interpolant);
GTHREE_API
GthreeAttributeArray *     gthree_interpolant_get_sample_values       (GthreeInterpolant *interpolant);
GTHREE_API
GthreeAttributeArray *     gthree_interpolant_evaluate                (GthreeInterpolant *interpolant,
                                                                       float              t);
GTHREE_API
GthreeEndingMode           gthree_interpolant_get_start_ending_mode   (GthreeInterpolant *interpolant);
GTHREE_API
GthreeEndingMode           gthree_interpolant_get_end_ending_mode     (GthreeInterpolant *interpolant);
GTHREE_API
GthreeInterpolantSettings *gthree_interpolant_get_settings            (GthreeInterpolant *interpolant);
GTHREE_API
void                       gthree_interpolant_set_settings            (GthreeInterpolant *interpolant,
                                                                       GthreeInterpolantSettings *settings);

GTHREE_API
GthreeInterpolantSettings *gthree_interpolant_settings_new (void);
GTHREE_API
GthreeEndingMode      gthree_interpolant_settings_get_start_ending_mode   (GthreeInterpolantSettings *settings);
GTHREE_API
void                  gthree_interpolant_settings_set_start_ending_mode   (GthreeInterpolantSettings *settings,
                                                                           GthreeEndingMode   mode);
GTHREE_API
GthreeEndingMode      gthree_interpolant_settings_get_end_ending_mode     (GthreeInterpolantSettings *settings);
GTHREE_API
void                  gthree_interpolant_settings_set_end_ending_mode     (GthreeInterpolantSettings *settings,
                                                                           GthreeEndingMode   mode);





G_END_DECLS

#endif /* __GTHREE_INTERPOLANT_H__ */
