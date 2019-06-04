#include <math.h>
#include <epoxy/gl.h>

#include "gthreeskinnedmesh.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeSkeleton *skeleton;
  const char *bind_mode; // enum?
  graphene_matrix_t bind_matrix;
  graphene_matrix_t bind_matrix_inverse;
} GthreeSkinnedMeshPrivate;

enum {
  PROP_0,


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

  graphene_matrix_init_identity (&priv->bind_matrix);
  graphene_matrix_init_identity (&priv->bind_matrix_inverse);
}

static void
gthree_skinned_mesh_finalize (GObject *obj)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_SKINNED_MESH (obj);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  g_clear_object (&priv->skeleton);

  G_OBJECT_CLASS (gthree_skinned_mesh_parent_class)->finalize (obj);
}

static void
gthree_skinned_mesh_update (GthreeObject *object)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_SKINNED_MESH (object);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  GTHREE_OBJECT_CLASS (gthree_skinned_mesh_parent_class)->update (object);
}

static void
gthree_skinned_mesh_set_property (GObject *obj,
                                  guint prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_SKINNED_MESH (obj);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  switch (prop_id)
    {

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
  GthreeSkinnedMesh *mesh = GTHREE_SKINNED_SKINNED_MESH (obj);
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  switch (prop_id)
    {

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
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

  //g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}


void
gthree_skinned_mesh_bind (GthreeSkinnedMesh *mesh,
                          GthreeSkeleton *skeleton,
                          const graphene_matrix_t *bind_matrix)
{
#ifdef TODO
  this.skeleton = skeleton;
  if ( bindMatrix === undefined ) {
    this.updateMatrixWorld( true );
    this.skeleton.calculateInverses();
    bindMatrix = this.matrixWorld;
  }

  this.bindMatrix.copy( bindMatrix );
  this.bindMatrixInverse.getInverse( bindMatrix );
#endif
}

void
gthree_skinned_mesh_pose (GthreeSkinnedMesh *mesh)
{
  GthreeSkinnedMeshPrivate *priv = gthree_skinned_mesh_get_instance_private (mesh);

  if (priv->skeleton)
    gthree_skeleton_pose (priv->skeleton);
}

void
gthree_skinned_mesh_normalize_skin_weights (GthreeSkinnedMesh *mesh)
{
#ifdef TODO
  var vector = new Vector4();
  var skinWeight = this.geometry.attributes.skinWeight;
  for ( var i = 0, l = skinWeight.count; i < l; i ++ ) {
    vector.x = skinWeight.getX( i );
    vector.y = skinWeight.getY( i );
    vector.z = skinWeight.getZ( i );
    vector.w = skinWeight.getW( i );
    var scale = 1.0 / vector.manhattanLength();
    if ( scale !== Infinity ) {
      vector.multiplyScalar( scale );
    } else {
      vector.set( 1, 0, 0, 0 ); // do something reasonable
    }
    skinWeight.setXYZW( i, vector.x, vector.y, vector.z, vector.w );
  }
#endif
}

void
gthre_skinned_mesh_update_matrix_world (GthreeSkinnedMesh *mesh,
                                        gboolean force)
{
#ifdef TODO
  // Needs a vfunc in GthreeObject
  Mesh.prototype.updateMatrixWorld.call( this, force );
  if ( this.bindMode === 'attached' ) {
    this.bindMatrixInverse.getInverse( this.matrixWorld );
  } else if ( this.bindMode === 'detached' ) {
    this.bindMatrixInverse.getInverse( this.bindMatrix );
  } else {
    console.warn( 'THREE.SkinnedMesh: Unrecognized bindMode: ' + this.bindMode );
  }
#endif
}
