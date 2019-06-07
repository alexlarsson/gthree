#include <math.h>

#include "gthreeskeleton.h"
#include "gthreeattribute.h"


typedef struct {
  GPtrArray *bones;
  graphene_matrix_t *bone_inverses;
  float *bone_matrices;
} GthreeSkeletonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSkeleton, gthree_skeleton, G_TYPE_OBJECT)

static void
gthree_skeleton_init (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  priv->bones = g_ptr_array_new_with_free_func (g_object_unref);
}

static void
gthree_skeleton_finalize (GObject *obj)
{
  GthreeSkeleton *skeleton = GTHREE_SKELETON (obj);
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  g_ptr_array_unref (priv->bones);
  g_free (priv->bone_inverses);
  g_free (priv->bone_matrices);

  G_OBJECT_CLASS (gthree_skeleton_parent_class)->finalize (obj);
}

static void
gthree_skeleton_class_init (GthreeSkeletonClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_skeleton_finalize;
}

GthreeSkeleton *
gthree_skeleton_new  (GthreeBone **bones,
                      int n_bones,
                      graphene_matrix_t *bone_inverses)
{
  GthreeSkeleton *skeleton;
  GthreeSkeletonPrivate *priv;
  int i;

  skeleton = g_object_new (gthree_skeleton_get_type (), NULL);
  priv = gthree_skeleton_get_instance_private (skeleton);

  priv->bone_inverses = g_new (graphene_matrix_t, n_bones);
  priv->bone_matrices = g_new (float, 16 * n_bones);

  for (i = 0; i < n_bones; i++)
    {
      g_ptr_array_add (priv->bones, g_object_ref (G_OBJECT (bones[i])));

      if (bone_inverses)
        graphene_matrix_init_from_matrix (&priv->bone_inverses[i], &bone_inverses[i]);
      else
        graphene_matrix_init_identity (&priv->bone_inverses[i]);
    }

  if (bone_inverses == NULL)
    gthree_skeleton_calculate_inverses (skeleton);

  return skeleton;
}

int
gthree_skeleton_get_n_bones  (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return priv->bones->len;
}

GthreeBone *
gthree_skeleton_get_bone (GthreeSkeleton *skeleton,
                          int           index)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return g_ptr_array_index (priv->bones, index);
}

void
gthree_skeleton_calculate_inverses  (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);
  int i;

  for (i = 0; i < priv->bones->len; i++)
    {
      GthreeBone *bone = g_ptr_array_index (priv->bones, i);
      const graphene_matrix_t *matrix_world = gthree_object_get_world_matrix (GTHREE_OBJECT (bone));
      graphene_matrix_inverse (matrix_world, &priv->bone_inverses[i]);
    }
}

void
gthree_skeleton_pose  (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);
  int i;

  for (i = 0; i < priv->bones->len; i++)
    {
      GthreeBone *bone = g_ptr_array_index (priv->bones, i);
      graphene_matrix_t inverse_inverse;

      graphene_matrix_inverse (&priv->bone_inverses[i], &inverse_inverse);
      gthree_object_set_world_matrix (GTHREE_OBJECT (bone), &inverse_inverse);
    }

  // compute the local matrices, positions, rotations and scales
  for (i = 0; i < priv->bones->len; i++)
    {
      GthreeBone *bone = g_ptr_array_index (priv->bones, i);
      GthreeObject *parent = gthree_object_get_parent (GTHREE_OBJECT (bone));
      const graphene_matrix_t *bone_matrix_world = gthree_object_get_world_matrix (GTHREE_OBJECT (bone));
      if (parent != NULL && GTHREE_IS_BONE (parent))
        {
          const graphene_matrix_t *parent_matrix_world =
            gthree_object_get_world_matrix (GTHREE_OBJECT (parent));
          graphene_matrix_t parent_matrix_world_inv, m;

          graphene_matrix_inverse (parent_matrix_world, &parent_matrix_world_inv);
          graphene_matrix_multiply (bone_matrix_world, &parent_matrix_world_inv, &m);

          gthree_object_set_matrix (GTHREE_OBJECT (bone), &m);
        }
      else
        {
          gthree_object_set_matrix (GTHREE_OBJECT (bone), bone_matrix_world);
        }

      //TODO:
      // bone.matrix.decompose( bone.position, bone.quaternion, bone.scale );
      // This should happen in gthree_object_set_world_matrix ()
      g_warning ("not supporting matrix decomposition");
    }
}

void
gthree_skeleton_update  (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);
  int i;

  for (i = 0; i < priv->bones->len; i++)
    {
      GthreeBone *bone = g_ptr_array_index (priv->bones, i);
      const graphene_matrix_t *matrix_world = gthree_object_get_world_matrix (GTHREE_OBJECT (bone));
      graphene_matrix_t offset_matrix;

      graphene_matrix_multiply (&priv->bone_inverses[i], matrix_world, &offset_matrix);
      graphene_matrix_to_float (&offset_matrix, (priv->bone_matrices + i * 16));
    }

#ifdef TODO
  if ( boneTexture !== undefined )
    boneTexture.needsUpdate = true;
#endif
}

float *
gthree_skeleton_get_bone_matrices (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return priv->bone_matrices;
}
