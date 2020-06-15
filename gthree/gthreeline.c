#include <math.h>
#include <epoxy/gl.h>

#include "gthreeline.h"
#include "gthreemeshbasicmaterial.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;
  GPtrArray *object_buffers;
} GthreeLinePrivate;

enum {
  PROP_0,

  PROP_GEOMETRY,
  PROP_MATERIAL,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLine, gthree_line, GTHREE_TYPE_OBJECT)

GthreeLine *
gthree_line_new (GthreeGeometry *geometry,
                          GthreeMaterial *material)
{
  return g_object_new (gthree_line_get_type (),
                       "geometry", geometry,
                       "material", material,
                       NULL);
}

static void
gthree_line_init (GthreeLine *line)
{
}

static void
gthree_line_finalize (GObject *obj)
{
  GthreeLine *line = GTHREE_LINE (obj);
  GthreeLinePrivate *priv = gthree_line_get_instance_private (line);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_line_parent_class)->finalize (obj);
}

static void
gthree_line_update (GthreeObject *object,
                    GthreeRenderer *renderer)
{
  GthreeLine *line = GTHREE_LINE (object);
  GthreeLinePrivate *priv = gthree_line_get_instance_private (line);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry, renderer);

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_line_fill_render_list (GthreeObject   *object,
                              GthreeRenderList *list)
{
  GthreeLine *line = GTHREE_LINE (object);
  GthreeLinePrivate *priv = gthree_line_get_instance_private (line);

  gthree_geometry_fill_render_list (priv->geometry, list, priv->material, NULL, object);
}

static gboolean
gthree_line_in_frustum (GthreeObject *object,
                                 const graphene_frustum_t *frustum)
{
  GthreeLine *line = GTHREE_LINE (object);
  GthreeLinePrivate *priv = gthree_line_get_instance_private (line);
  graphene_sphere_t sphere;

  if (!priv->geometry)
    return FALSE;

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static void
gthree_line_set_property (GObject *obj,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GthreeLine *line = GTHREE_LINE (obj);
  GthreeLinePrivate *priv = gthree_line_get_instance_private (line);

  switch (prop_id)
    {
    case PROP_GEOMETRY:
      g_set_object (&priv->geometry, g_value_get_object (value));
      break;

    case PROP_MATERIAL:
      g_set_object (&priv->material, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_line_get_property (GObject *obj,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  GthreeLine *line = GTHREE_LINE (obj);
  GthreeLinePrivate *priv = gthree_line_get_instance_private (line);

  switch (prop_id)
    {
    case PROP_GEOMETRY:
      g_value_set_object (value, priv->geometry);
      break;

    case PROP_MATERIAL:
      g_value_set_object (value, priv->material);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_line_class_init (GthreeLineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_line_set_property;
  gobject_class->get_property = gthree_line_get_property;
  gobject_class->finalize = gthree_line_finalize;

  object_class->in_frustum = gthree_line_in_frustum;
  object_class->update = gthree_line_update;
  object_class->fill_render_list = gthree_line_fill_render_list;

  obj_props[PROP_GEOMETRY] =
    g_param_spec_object ("geometry", "Geometry", "Geometry",
                         GTHREE_TYPE_GEOMETRY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MATERIAL] =
    g_param_spec_object ("material", "Material", "Material",
                         GTHREE_TYPE_MATERIAL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
