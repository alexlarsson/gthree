#include <math.h>
#include <epoxy/gl.h>

#include "gthreelinesegments.h"
#include "gthreemultimaterial.h"
#include "gthreebasicmaterial.h"
#include "gthreegeometrygroupprivate.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;
} GthreeLineSegmentsPrivate;

enum {
  PROP_0,

  PROP_GEOMETRY,
  PROP_MATERIAL,

  N_PROPS
};

static GQuark q_color;

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLineSegments, gthree_line_segments, GTHREE_TYPE_OBJECT)

GthreeLineSegments *
gthree_line_segments_new (GthreeGeometry *geometry,
                          GthreeMaterial *material)
{
  return g_object_new (gthree_line_segments_get_type (),
                       "geometry", geometry,
                       "material", material,
                       NULL);
}

static void
gthree_line_segments_init (GthreeLineSegments *line_segments)
{
}

static void
gthree_line_segments_finalize (GObject *obj)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (obj);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_line_segments_parent_class)->finalize (obj);
}

static void
gthree_line_segments_update (GthreeObject *object)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (object);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry, priv->material);

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_line_segments_realize (GthreeObject *object)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (object);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);

  gthree_geometry_realize (priv->geometry, priv->material);
  gthree_geometry_add_buffers_to_object (priv->geometry, priv->material, object);
}

static void
gthree_line_segments_unrealize (GthreeObject *object)
{
  /* TODO: unrealize the geometry? */
}

static gboolean
gthree_line_segments_in_frustum (GthreeObject *object,
                                 const graphene_frustum_t *frustum)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (object);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);
  graphene_sphere_t sphere;

  if (!priv->geometry)
    return FALSE;

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static gboolean
gthree_line_segments_real_has_attribute_data (GthreeObject *object,
                                              GQuark        attribute)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (object);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);

  if (!priv->geometry)
    return FALSE;

  if (attribute == q_color)
    return gthree_geometry_get_n_colors (priv->geometry) > 0 || gthree_geometry_get_n_faces (priv->geometry);

  return FALSE;
}

static void
gthree_line_segments_set_property (GObject *obj,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (obj);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);

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
gthree_line_segments_get_property (GObject *obj,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  GthreeLineSegments *line_segments = GTHREE_LINE_SEGMENTS (obj);
  GthreeLineSegmentsPrivate *priv = gthree_line_segments_get_instance_private (line_segments);

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
gthree_line_segments_class_init (GthreeLineSegmentsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_line_segments_set_property;
  gobject_class->get_property = gthree_line_segments_get_property;
  gobject_class->finalize = gthree_line_segments_finalize;

  object_class->in_frustum = gthree_line_segments_in_frustum;
  object_class->has_attribute_data = gthree_line_segments_real_has_attribute_data;
  object_class->update = gthree_line_segments_update;
  object_class->realize = gthree_line_segments_realize;
  object_class->unrealize = gthree_line_segments_unrealize;

  obj_props[PROP_GEOMETRY] =
    g_param_spec_object ("geometry", "Geometry", "Geometry",
                         GTHREE_TYPE_GEOMETRY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MATERIAL] =
    g_param_spec_object ("material", "Material", "Material",
                         GTHREE_TYPE_MATERIAL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);

#define INIT_QUARK(name) q_##name = g_quark_from_static_string (#name)
  INIT_QUARK(color);
}
