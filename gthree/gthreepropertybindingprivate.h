#ifndef __GTHREE_PROPERTY_BINDING_H__
#define __GTHREE_PROPERTY_BINDING_H__

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gthree/gthreetypes.h>
#include <gthree/gthreeenums.h>
#include <gthree/gthreeobject.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_PROPERTY_BINDING      (gthree_property_binding_get_type ())
#define GTHREE_PROPERTY_BINDING(inst)     (G_TYPE_CHECK_INSTANCE_CAST ((inst),  \
                                                  GTHREE_TYPE_PROPERTY_BINDING, \
                                                  GthreePropertyBinding))
#define GTHREE_PROPERTY_BINDING_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTHREE_TYPE_PROPERTY_BINDING, GthreePropertyBindingClass))
#define GTHREE_IS_PROPERTY_BINDING(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),  \
                                                  GTHREE_TYPE_PROPERTY_BINDING))
#define GTHREE_PROPERTY_BINDING_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst), GTHREE_TYPE_PROPERTY_BINDING, GthreePropertyBindingClass))

typedef struct {
  char *node_name;
  char *object_name;
  char *object_index;
  char *property_name;
  char *property_index;
} GthreeParsedPath;

void gthree_parsed_path_free (GthreeParsedPath *path);
GthreeParsedPath *gthree_parsed_path_dup (GthreeParsedPath *path);
GthreeParsedPath * gthree_parsed_path_new (const char *node_name,
                                           const char *object_name,
                                           const char *object_index,
                                           const char *property_name,
                                           const char *property_index);

GthreeParsedPath * gthree_parsed_path_parse (const char *name);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreeParsedPath, gthree_parsed_path_free)


typedef struct {
  GObject parent;
} GthreePropertyBinding;

typedef struct {
  GObjectClass parent_class;
} GthreePropertyBindingClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePropertyBinding, g_object_unref)

GType gthree_property_binding_get_type (void) G_GNUC_CONST;

GthreePropertyBinding *gthree_property_binding_new (GthreeObject     *root,
                                                    const char       *path,
                                                    GthreeParsedPath *parsed_path);

GthreeParsedPath *gthree_property_binding_get_parsed_path (GthreePropertyBinding *binding);
void              ghtree_property_binding_get_value       (GthreePropertyBinding *binding,
                                                           float                 *buffer,
                                                           int                    offset);
void              ghtree_property_binding_set_value       (GthreePropertyBinding *binding,
                                                           float                 *buffer,
                                                           int                    offset);

G_END_DECLS

#endif /* __GTHREE_PROPERTY_BINDING_H__ */
