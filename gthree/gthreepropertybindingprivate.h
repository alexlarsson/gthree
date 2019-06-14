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
  GObject parent;
} GthreePropertyBinding;

typedef struct {
  GObjectClass parent_class;
} GthreePropertyBindingClass;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GthreePropertyBinding, g_object_unref)

GType gthree_property_binding_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __GTHREE_PROPERTY_BINDING_H__ */
