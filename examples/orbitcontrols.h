#include <gthree/gthree.h>
#include <gthree/gthreearea.h>
#include <gtk/gtk.h>

typedef struct OrbitControls OrbitControls;

OrbitControls *orbit_controls_new (GthreeObject *target, GtkWidget *darea);
void orbit_controls_free (OrbitControls *orbit);

void orbit_controls_set_enable_damping       (OrbitControls         *orbit,
                                              gboolean               enable_damping);
void orbit_controls_set_damping_factor       (OrbitControls         *orbit,
                                              float                  damping_factor);
void orbit_controls_set_auto_rotate          (OrbitControls         *orbit,
                                              gboolean               auto_rotate);
void orbit_controls_set_auto_rotate_speed    (OrbitControls         *orbit,
                                              float                  speed);
void orbit_controls_set_target               (OrbitControls         *orbit,
                                              const graphene_vec3_t *target);
void orbit_controls_set_min_distance         (OrbitControls         *orbit,
                                              float                  min);
void orbit_controls_set_max_distance         (OrbitControls         *orbit,
                                              float                  max);
void orbit_controls_set_min_zoom             (OrbitControls         *orbit,
                                              float                  min);
void orbit_controls_set_max_zoom             (OrbitControls         *orbit,
                                              float                  max);
void orbit_controls_set_min_polar_angle      (OrbitControls         *orbit,
                                              float                  min);
void orbit_controls_set_max_polar_angle      (OrbitControls         *orbit,
                                              float                  max);
void orbit_controls_set_enable_zoom          (OrbitControls         *orbit,
                                              gboolean               enable);
void orbit_controls_set_enable_rotate        (OrbitControls         *orbit,
                                              gboolean               enable);
void orbit_controls_set_enable_pan           (OrbitControls         *orbit,
                                              gboolean               enable);
void orbit_controls_set_screen_space_panning (OrbitControls         *orbit,
                                              gboolean               screen_space_panning);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (OrbitControls, orbit_controls_free)
