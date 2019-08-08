#include <math.h>
#include <epoxy/gl.h>

#include "gthreepoints.h"
#include "gthreepointsmaterial.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;
} GthreePointsPrivate;

enum {
  PROP_0,

  PROP_GEOMETRY,
  PROP_MATERIAL,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreePoints, gthree_points, GTHREE_TYPE_OBJECT)

GthreePoints *
gthree_points_new (GthreeGeometry *geometry,
                   GthreeMaterial *material)
{
  GthreePoints *points;
  gboolean free_material = FALSE;

  if (material == NULL)
    {
      material = GTHREE_MATERIAL (gthree_points_material_new ());
      free_material = TRUE;
    }

  points =
    g_object_new (gthree_points_get_type (),
                  "geometry", geometry,
                  "material", material,
                  NULL);

  if (free_material)
    g_object_unref (material);

  return points;
}

static void
gthree_points_init (GthreePoints *points)
{
}

static void
gthree_points_finalize (GObject *obj)
{
  GthreePoints *points = GTHREE_POINTS (obj);
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_points_parent_class)->finalize (obj);
}

static void
gthree_points_update (GthreeObject *object)
{
  GthreePoints *points = GTHREE_POINTS (object);
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry);

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_points_fill_render_list (GthreeObject   *object,
                                GthreeRenderList *list)
{
  GthreePoints *points = GTHREE_POINTS (object);
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  gthree_geometry_fill_render_list (priv->geometry, list, priv->material, NULL, object);
}

static gboolean
gthree_points_in_frustum (GthreeObject *object,
                          const graphene_frustum_t *frustum)
{
  GthreePoints *points = GTHREE_POINTS (object);
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);
  graphene_sphere_t sphere;

  if (!priv->geometry)
    return FALSE;

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static void
gthree_points_set_property (GObject *obj,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GthreePoints *points = GTHREE_POINTS (obj);
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  switch (prop_id)
    {
     case PROP_GEOMETRY:
      g_set_object (&priv->geometry, g_value_get_object (value));
      break;

   case PROP_MATERIAL:
      gthree_points_set_material (points, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_points_get_property (GObject *obj,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  GthreePoints *points = GTHREE_POINTS (obj);
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

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

GthreeMaterial *
gthree_points_get_material (GthreePoints *points)
{
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  return priv->material;
}

void
gthree_points_set_material (GthreePoints *points,
                            GthreeMaterial *material)
{
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  g_set_object (&priv->material, material);
}


GthreeGeometry *
gthree_points_get_geometry (GthreePoints *points)
{
  GthreePointsPrivate *priv = gthree_points_get_instance_private (points);

  return priv->geometry;
}

static void
gthree_points_class_init (GthreePointsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_points_set_property;
  gobject_class->get_property = gthree_points_get_property;
  gobject_class->finalize = gthree_points_finalize;

  object_class->in_frustum = gthree_points_in_frustum;
  object_class->update = gthree_points_update;
  object_class->fill_render_list = gthree_points_fill_render_list;

  obj_props[PROP_GEOMETRY] =
    g_param_spec_object ("geometry", "Geometry", "Geometry",
                         GTHREE_TYPE_GEOMETRY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MATERIAL] =
    g_param_spec_object ("material", "Material", "Material",
                        GTHREE_TYPE_MATERIAL,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
