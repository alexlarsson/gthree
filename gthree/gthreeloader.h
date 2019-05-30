#ifndef __GTHREE_LOADER_H__
#define __GTHREE_LOADER_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreegeometry.h>
#include <gthree/gthreematerial.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_LOADER      (gthree_loader_get_type ())
#define GTHREE_LOADER(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                                             GTHREE_TYPE_LOADER, \
                                                             GthreeLoader))
#define GTHREE_IS_LOADER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),    \
                                                             GTHREE_TYPE_LOADER))


typedef struct {
  GObject parent;
} GthreeLoader;

typedef struct {
  GObjectClass parent_class;

} GthreeLoaderClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeLoader, g_object_unref)

typedef enum {
  GTHREE_LOADER_ERROR_FAIL,
} GthreeLoaderError;

#define GTHREE_LOADER_ERROR               (gthree_loader_error_quark ())


GQuark gthree_loader_error_quark (void);
GType gthree_loader_get_type (void) G_GNUC_CONST;

GthreeLoader *gthree_loader_new_from_json (const char *data, GFile *texture_path, GError **error);

GthreeGeometry *gthree_load_geometry_from_json (const char *data, GError **error);

G_END_DECLS

#endif /* __GTHREE_LOADER_H__ */
