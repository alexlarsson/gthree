#include <math.h>
#include <epoxy/gl.h>

#include "gthreeskinnedmesh.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"
#include "gthreetypebuiltins.h"
#include "gthreeattribute.h"

typedef struct {
  GthreeSkeleton *skeleton;
  GthreeBindMode bind_mode;
  graphene_matrix_t bind_matrix;
  graphene_matrix_t bind_matrix_inverse;
} GthreeSkinnedMeshPrivate;

enum {
  PROP_0,

  PROP_BIND_MODE,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSkinnedMesh, gthree_skinned_mesh, GTHREE_TYPE_MESH)

GthreeSkinnedMesh *
gthree_skinned_mesh_new (GthreeGeometry *geometry,
                         GthreeMaterial *material)
{
  return g_object_new (gthree_skinned_mesh_get_type (),
                       "geometry", geometry,
                       "material", material,
                       NULL);
}

static void
gthree_skinned_mesh_init (GthreeSkinnedMesh *mesh)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  priv->bind_mode = GTHREE_BIND_MODE_ATTACHED;
  graphene_matrix_init_identity (&priv->bind_matrix);
  graphene_matrix_init_identity (&priv->bind_matrix_inverse);
}

static void
gthree_skinned_mesh_finalize (GObject *obj)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_MESH (obj);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  g_clear_object (&priv->skeleton);

  G_OBJECT_CLASS (gthree_skinned_mesh_parent_class)->finalize (obj);
}

static void
gthree_skinned_mesh_update (GthreeObject *object)
{
  GTHREE_OBJECT_CLASS (gthree_skinned_mesh_parent_class)->update (object);
}

static void
gthree_skinned_mesh_set_property (GObject *obj,
                                  guint prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_MESH (obj);

  switch (prop_id)
    {
    case PROP_BIND_MODE:
      gthree_skinned_mesh_set_bind_mode (mesh, g_value_get_enum (value));
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_skinned_mesh_get_property (GObject *obj,
                                  guint prop_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_MESH (obj);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  switch (prop_id)
    {
    case PROP_BIND_MODE:
      g_value_set_enum (value, priv->bind_mode);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}


GthreeSkeleton *
gthree_skinned_mesh_get_skeleton (GthreeSkinnedMesh *mesh)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  return priv->skeleton;
}

void
gthree_skinned_mesh_bind (GthreeSkinnedMesh *mesh,
                          GthreeSkeleton *skeleton,
                          const graphene_matrix_t *bind_matrix)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  g_object_ref (skeleton);
  g_clear_object (&priv->skeleton);
  priv->skeleton = skeleton;

  if (bind_matrix == NULL)
    {
      gthree_object_update_matrix_world (GTHREE_OBJECT (mesh), TRUE);
      gthree_skeleton_calculate_inverses  (priv->skeleton);
      graphene_matrix_init_from_matrix (&priv->bind_matrix,
                                        gthree_object_get_world_matrix (GTHREE_OBJECT (mesh)));
    }
  else
    {
      graphene_matrix_init_from_matrix (&priv->bind_matrix,
                                        bind_matrix);
    }

  graphene_matrix_inverse (&priv->bind_matrix, &priv->bind_matrix_inverse);
}

void
gthree_skinned_mesh_pose (GthreeSkinnedMesh *mesh)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  if (priv->skeleton)
    gthree_skeleton_pose (priv->skeleton);
}

const graphene_matrix_t *
gthree_skinned_mesh_get_bind_matrix (GthreeSkinnedMesh       *mesh)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  return &priv->bind_matrix;
}

const graphene_matrix_t *
gthree_skinned_mesh_get_inverse_bind_matrix (GthreeSkinnedMesh       *mesh)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  return &priv->bind_matrix_inverse;
}

void
gthree_skinned_mesh_set_bind_mode (GthreeSkinnedMesh *mesh,
                                   GthreeBindMode bind_mode)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  priv->bind_mode = bind_mode;
}

void
gthree_skinned_mesh_normalize_skin_weights (GthreeSkinnedMesh *mesh)
{
  GthreeAttribute *skin_weight;
  int count, i;

  skin_weight =
    gthree_geometry_get_attribute (gthree_mesh_get_geometry (GTHREE_MESH (mesh)),
                                   GTHREE_ATTRIBUTE_NAME_SKIN_WEIGHT);
  if (skin_weight == NULL)
    return;

  count = gthree_attribute_get_count (skin_weight);
  for (i = 0; i < count; i++)
    {
      graphene_vec4_t v;
      gthree_attribute_get_vec4 (skin_weight, i, &v);
      graphene_vec4_normalize (&v, &v);
      gthree_attribute_set_vec4 (skin_weight, i, &v);
    }
}

static gboolean
gthree_skinned_mesh_update_matrix_world (GthreeObject *object,
                                         gboolean force)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_MESH (object);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  force = GTHREE_OBJECT_CLASS (gthree_skinned_mesh_parent_class)->update_matrix_world (object, force);

  if (priv->bind_mode == GTHREE_BIND_MODE_ATTACHED)
    graphene_matrix_inverse (gthree_object_get_world_matrix (object),
                             &priv->bind_matrix_inverse);
  else
    graphene_matrix_inverse (&priv->bind_matrix, &priv->bind_matrix_inverse);

  return force;
}

static void
gthree_skinned_mesh_class_init (GthreeSkinnedMeshClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_skinned_mesh_set_property;
  gobject_class->get_property = gthree_skinned_mesh_get_property;
  gobject_class->finalize = gthree_skinned_mesh_finalize;

  object_class->update = gthree_skinned_mesh_update;
  object_class->update_matrix_world = gthree_skinned_mesh_update_matrix_world;

  obj_props[PROP_BIND_MODE] =
    g_param_spec_enum ("bind-mode", "Bind mode", "Bind mode",
                       GTHREE_TYPE_BIND_MODE,
                       GTHREE_BIND_MODE_ATTACHED,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
