#include <math.h>

#include "gthreepropertybindingprivate.h"


typedef struct {
} GthreePropertyBindingPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreePropertyBinding, gthree_property_binding, G_TYPE_OBJECT)

static void
gthree_property_binding_init (GthreePropertyBinding *binding)
{
  //GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
}

static void
gthree_property_binding_finalize (GObject *obj)
{
  //GthreePropertyBinding *binding = GTHREE_PROPERTY_BINDING (obj);
  //GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);

  G_OBJECT_CLASS (gthree_property_binding_parent_class)->finalize (obj);
}

static void
gthree_property_binding_class_init (GthreePropertyBindingClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_property_binding_finalize;
}
