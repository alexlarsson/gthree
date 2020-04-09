#include <gthree/gthree.h>
#include <gthree/gthreearea.h>
#include <gtk/gtk.h>

GdkPixbuf *examples_load_pixbuf (const char *file);
GthreeGeometry *examples_load_geometry (const char *name);
void examples_load_cube_pixbufs (const char *dir,
                                 GdkPixbuf *pixbufs[6]);

GthreeLoader *examples_load_gltl (const char *name, GError **error);

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

GtkWidget *examples_init (const char *title,
                          GtkWidget **box,
                          gboolean   *done);
GtkEventController *motion_controller_for (GtkWidget *widget);
GtkEventController *click_controller_for (GtkWidget *widget);
GtkEventController *scroll_controller_for (GtkWidget *widget);
GtkEventController *drag_controller_for (GtkWidget *widget);
