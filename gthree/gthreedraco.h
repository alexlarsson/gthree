#ifndef __GTHREE_DRACO_H__
#define __GTHREE_DRACO_H__

#include <glib.h>
#include <gthree/gthreeattribute.h>
#include <gthree/gthreegeometry.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

gboolean gthree_draco_decode (const guint8   *data,
                              gsize           len,
                              GthreeGeometry *geometry,
                              JsonObject     *draco_ext,
                              JsonObject     *gltf_attributes,
                              GError        **error);

G_END_DECLS

#endif /* __GTHREE_DRACO_H__ */
