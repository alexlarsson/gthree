#include <math.h>
#include <epoxy/gl.h>

#include "gthreemesh.h"
#include "gthreemultimaterial.h"
#include "gthreebasicmaterial.h"
#include "gthreegeometrygroupprivate.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;
} GthreeMeshPrivate;

enum {
  PROP_0,

  PROP_GEOMETRY,
  PROP_MATERIAL,

  N_PROPS
};

static GQuark q_color;
static GQuark q_uv;
static GQuark q_uv2;

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMesh, gthree_mesh, GTHREE_TYPE_OBJECT)

GthreeMesh *
gthree_mesh_new (GthreeGeometry *geometry,
                 GthreeMaterial *material)
{
  return g_object_new (gthree_mesh_get_type (),
                       "geometry", geometry,
                       "material", material,
                       NULL);
}

static void
gthree_mesh_init (GthreeMesh *mesh)
{
}

static void
gthree_mesh_finalize (GObject *obj)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_mesh_parent_class)->finalize (obj);
}

static void
gthree_mesh_update (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry, priv->material);

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_mesh_realize (GthreeObject *object)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  gthree_geometry_realize (priv->geometry, priv->material);
  gthree_geometry_add_buffers_to_object (priv->geometry, priv->material, object);
}

static void
gthree_mesh_unrealize (GthreeObject *object)
{
  /* TODO: unrealize the geometry? */
}

static gboolean
gthree_mesh_in_frustum (GthreeObject *object,
                        const graphene_frustum_t *frustum)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);
  graphene_sphere_t sphere;

  if (!priv->geometry)
    return FALSE;

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    gthree_geometry_get_bounding_sphere (priv->geometry),
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static gboolean
gthree_mesh_real_has_attribute_data (GthreeObject *object,
                                     GQuark        attribute)
{
  GthreeMesh *mesh = GTHREE_MESH (object);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  if (!priv->geometry)
    return FALSE;

  if (attribute == q_color)
    return gthree_geometry_get_n_colors (priv->geometry) > 0 || gthree_geometry_get_n_faces (priv->geometry);
  else if (attribute == q_uv)
    return gthree_geometry_get_n_uv (priv->geometry) > 0;
  else if (attribute == q_uv2)
    return gthree_geometry_get_n_uv2 (priv->geometry) > 0;

  return FALSE;
}

static void
gthree_mesh_set_property (GObject *obj,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

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
gthree_mesh_get_property (GObject *obj,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  GthreeMesh *mesh = GTHREE_MESH (obj);
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

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
gthree_mesh_get_material (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->material;
}

GthreeGeometry *
gthree_mesh_get_geometry (GthreeMesh *mesh)
{
  GthreeMeshPrivate *priv = gthree_mesh_get_instance_private (mesh);

  return priv->geometry;
}

static void
gthree_mesh_class_init (GthreeMeshClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_mesh_set_property;
  gobject_class->get_property = gthree_mesh_get_property;
  gobject_class->finalize = gthree_mesh_finalize;

  object_class->in_frustum = gthree_mesh_in_frustum;
  object_class->has_attribute_data = gthree_mesh_real_has_attribute_data;
  object_class->update = gthree_mesh_update;
  object_class->realize = gthree_mesh_realize;
  object_class->unrealize = gthree_mesh_unrealize;

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
  INIT_QUARK(uv);
  INIT_QUARK(uv2);
}
