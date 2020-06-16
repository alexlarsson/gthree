#include <gthree/gthree.h>
#include <gthree/gthreearea.h>
#include <gtk/gtk.h>

#ifndef USE_GTK4
static inline void gtk_box_append (GtkBox *box, GtkWidget *child) {
  gtk_container_add (GTK_CONTAINER (box), child);
}
#endif

GdkPixbuf *examples_load_pixbuf (const char *file);
GthreeTexture *examples_load_texture (const char *file);
GthreeGeometry *examples_load_geometry (const char *name);
void examples_load_cube_pixbufs (const char *dir,
                                 GdkPixbuf *pixbufs[6]);

GthreeLoader *examples_load_gltl (const char *name);

const graphene_vec3_t *black (void);
const graphene_vec3_t *white (void);
const graphene_vec3_t *red (void);
const graphene_vec3_t *green (void);
const graphene_vec3_t *blue (void);
const graphene_vec3_t *yellow (void);
const graphene_vec3_t *cyan (void);
const graphene_vec3_t *magenta (void);
const graphene_vec3_t *very_dark_grey (void);
const graphene_vec3_t *dark_grey (void);
const graphene_vec3_t *medium_grey (void);
const graphene_vec3_t *grey (void);
const graphene_vec3_t *light_grey (void);
const graphene_vec3_t *dark_green (void);
const graphene_vec3_t *orange (void);
void rgb_init_from_hsl (graphene_vec3_t *color,
                        double   hue,
                        double   saturation,
                        double   lightness);

GtkWidget *examples_init (const char *title,
                          GtkWidget **box,
                          gboolean   *done);
GtkEventController *motion_controller_for (GtkWidget *widget);
GtkEventController *click_controller_for (GtkWidget *widget);
GtkEventController *scroll_controller_for (GtkWidget *widget);
GtkEventController *drag_controller_for (GtkWidget *widget);

typedef struct {
  float radius;
  float phi;
  float theta;
} Spherical;

Spherical *spherical_init (Spherical *s,
                           float radius,
                           float phi,
                           float theta);
void spherical_set_from_vec3 (Spherical *s,
                              const graphene_vec3_t *v);
void spherical_make_safe (Spherical *s);

graphene_vec3_t *vec3_init_from_spherical (graphene_vec3_t *v,
                                           const Spherical *s);
void vec3_apply_quaternion (const graphene_vec3_t *v,
                            const graphene_quaternion_t *q,
                            graphene_vec3_t *res);

void quaternion_from_unit_vectors (graphene_quaternion_t *q,
                                   const graphene_vec3_t *from,
                                   const graphene_vec3_t *to);
