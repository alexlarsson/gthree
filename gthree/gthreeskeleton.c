#include <math.h>
#include <string.h>

#include "gthreeskeleton.h"
#include "gthreeattribute.h"
#include "gthreerawtexture.h"


typedef struct {
  GPtrArray *bones;
  graphene_matrix_t *bone_inverses;
  float *bone_matrices;
  GthreeRawTexture *bone_texture;
  int bone_texture_size;
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

  g_clear_object (&priv->bone_texture);

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

const graphene_matrix_t *
gthree_skeleton_get_bone_inverse (GthreeSkeleton *skeleton, int index)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return &priv->bone_inverses[index];
}

/**
 * gthree_skeleton_get_bone:
 *
 * Returns: (transfer none):
 */
GthreeBone *
gthree_skeleton_get_bone (GthreeSkeleton *skeleton,
                          int           index)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return g_ptr_array_index (priv->bones, index);
}

/**
 * gthree_skeleton_get_bone_by_name:
 *
 * Returns: (transfer none):
 */
GthreeBone *
gthree_skeleton_get_bone_by_name (GthreeSkeleton *skeleton,
                                  const char *name)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);
  int i;

  for (i = 0; i < priv->bones->len; i++)
    {
      GthreeBone *bone = g_ptr_array_index (priv->bones, i);
      if (g_strcmp0 (name, gthree_object_get_name (GTHREE_OBJECT (bone))) == 0)
        return bone;
    }

  return NULL;
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

static int
next_power_of_two (int n)
{
  int v = 1;
  while (v < n)
    v <<= 1;
  return v;
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

  if (priv->bone_texture == NULL)
    {
      int n_bones = priv->bones->len;
      int size = next_power_of_two ((int)ceil (sqrt (n_bones * 4)));

      priv->bone_texture_size = size;
      priv->bone_matrices = g_renew (float, priv->bone_matrices, size * size * 4);
      memset (priv->bone_matrices + n_bones * 16, 0,
              (size * size * 4 - n_bones * 16) * sizeof (float));

      priv->bone_texture = gthree_raw_texture_new_2d (size, size,
                                                      GL_RGBA32F, GL_RGBA,
                                                      priv->bone_matrices);
    }
  else
    {
      gthree_raw_texture_set_data (priv->bone_texture,
                                   priv->bone_texture_size, priv->bone_texture_size,
                                   0,
                                   priv->bone_matrices);
    }
}

float *
gthree_skeleton_get_bone_matrices (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return priv->bone_matrices;
}

GthreeRawTexture *
gthree_skeleton_get_bone_texture (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return priv->bone_texture;
}

int
gthree_skeleton_get_bone_texture_size (GthreeSkeleton *skeleton)
{
  GthreeSkeletonPrivate *priv = gthree_skeleton_get_instance_private (skeleton);

  return priv->bone_texture_size;
}
