#include <gthree/gthree.h>
#include <gthree/gthreearea.h>
#include <gtk/gtk.h>

#define GTHREE_TYPE_ORBIT_CONTROLS      (gthree_orbit_controls_get_type ())
#define GTHREE_ORBIT_CONTROLS(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), GTHREE_TYPE_ORBIT_CONTROLS, GthreeOrbitControls))
#define GTHREE_IS_ORBIT_CONTROLS(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), GTHREE_TYPE_ORBIT_CONTROLS))

typedef struct _GthreeOrbitControlsClass GthreeOrbitControlsClass;
typedef struct _GthreeOrbitControls GthreeOrbitControls;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeOrbitControls, g_object_unref)

GType gthree_orbit_controls_get_type (void) G_GNUC_CONST;

GthreeOrbitControls *gthree_orbit_controls_new (GthreeObject *target, GtkWidget *darea);

void gthree_orbit_controls_set_enable_damping       (GthreeOrbitControls   *orbit,
                                                     gboolean               enable_damping);
void gthree_orbit_controls_set_damping_factor       (GthreeOrbitControls   *orbit,
                                                     float                  damping_factor);
void gthree_orbit_controls_set_auto_rotate          (GthreeOrbitControls   *orbit,
                                                     gboolean               auto_rotate);
void gthree_orbit_controls_set_auto_rotate_speed    (GthreeOrbitControls   *orbit,
                                                     float                  speed);
void gthree_orbit_controls_set_target               (GthreeOrbitControls   *orbit,
                                                     const graphene_vec3_t *target);
void gthree_orbit_controls_set_min_distance         (GthreeOrbitControls   *orbit,
                                                     float                  min);
void gthree_orbit_controls_set_max_distance         (GthreeOrbitControls   *orbit,
                                                     float                  max);
void gthree_orbit_controls_set_min_zoom             (GthreeOrbitControls   *orbit,
                                                     float                  min);
void gthree_orbit_controls_set_max_zoom             (GthreeOrbitControls   *orbit,
                                                     float                  max);
void gthree_orbit_controls_set_min_polar_angle      (GthreeOrbitControls   *orbit,
                                                     float                  min);
void gthree_orbit_controls_set_max_polar_angle      (GthreeOrbitControls   *orbit,
                                                     float                  max);
void gthree_orbit_controls_set_enable_zoom          (GthreeOrbitControls   *orbit,
                                                     gboolean               enable);
void gthree_orbit_controls_set_enable_rotate        (GthreeOrbitControls   *orbit,
                                                     gboolean               enable);
void gthree_orbit_controls_set_enable_pan           (GthreeOrbitControls   *orbit,
                                                     gboolean               enable);
void gthree_orbit_controls_set_screen_space_panning (GthreeOrbitControls   *orbit,
                                                     gboolean               screen_space_panning);
