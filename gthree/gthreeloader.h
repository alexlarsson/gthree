#ifndef __GTHREE_LOADER_H__
#define __GTHREE_LOADER_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreegeometry.h>
#include <gthree/gthreematerial.h>
#include <gthree/gthreeanimationclip.h>

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


GTHREE_API
GQuark gthree_loader_error_quark (void);
GTHREE_API
GType gthree_loader_get_type (void) G_GNUC_CONST;

GTHREE_API
int                  gthree_loader_get_n_scenes     (GthreeLoader *loader);
GTHREE_API
GthreeScene *        gthree_loader_get_scene        (GthreeLoader *loader,
                                                     int           index);
GTHREE_API
int                  gthree_loader_get_n_materials  (GthreeLoader *loader);
GTHREE_API
GthreeMaterial *     gthree_loader_get_material     (GthreeLoader *loader,
                                                     int           index);
GTHREE_API
int                  gthree_loader_get_n_animations (GthreeLoader *loader);
GTHREE_API
GthreeAnimationClip *gthree_loader_get_animation    (GthreeLoader *loader,
                                                     int           index);

GTHREE_API
GthreeLoader *gthree_loader_parse_gltf (GBytes *data, GFile *base_path, GError **error);

GTHREE_API
GthreeGeometry *gthree_load_geometry_from_json (const char *data, GError **error);
G_END_DECLS

#endif /* __GTHREE_LOADER_H__ */
