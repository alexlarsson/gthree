#include "gthreecubecamera.h"
#include "gthreeperspectivecamera.h"
#include "gthreeprivate.h"

typedef struct {
  GthreePerspectiveCamera *cameras[6];
  GthreeRenderTarget *render_target;
} GthreeCubeCameraPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeCubeCamera, gthree_cube_camera, GTHREE_TYPE_OBJECT)

static void
gthree_cube_camera_init (GthreeCubeCamera *cube_camera)
{
}

static void
gthree_cube_camera_finalize (GObject *obj)
{
  GthreeCubeCamera *cube_camera = GTHREE_CUBE_CAMERA (obj);
  GthreeCubeCameraPrivate *priv = gthree_cube_camera_get_instance_private (cube_camera);

  g_clear_object (&priv->render_target);

  G_OBJECT_CLASS (gthree_cube_camera_parent_class)->finalize (obj);
}

static void
gthree_cube_camera_class_init (GthreeCubeCameraClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_cube_camera_finalize;
}

GthreeCubeCamera *
gthree_cube_camera_new (float               near,
                        float               far,
                        GthreeRenderTarget *render_target)
{
  GthreeCubeCamera *cube_camera;
  GthreeCubeCameraPrivate *priv;
  graphene_vec3_t up, look_at;
  int i;

  cube_camera = g_object_new (GTHREE_TYPE_CUBE_CAMERA, NULL);
  priv = gthree_cube_camera_get_instance_private (cube_camera);

  priv->render_target = g_object_ref (render_target);

  for (i = 0; i < 6; i++)
    {
      priv->cameras[i] = gthree_perspective_camera_new (-90.0f, 1.0f, near, far);
      gthree_object_add_child (GTHREE_OBJECT (cube_camera),
                               GTHREE_OBJECT (priv->cameras[i]));
    }

  /* WebGL coordinate system cube map face orientations matching
     GL_TEXTURE_CUBE_MAP_POSITIVE_X + i ordering */

  /* PX: +X */
  graphene_vec3_init (&up, 0, 1, 0);
  graphene_vec3_init (&look_at, 1, 0, 0);
  gthree_object_set_up (GTHREE_OBJECT (priv->cameras[0]), &up);
  gthree_object_look_at (GTHREE_OBJECT (priv->cameras[0]), &look_at);

  /* NX: -X */
  graphene_vec3_init (&up, 0, 1, 0);
  graphene_vec3_init (&look_at, -1, 0, 0);
  gthree_object_set_up (GTHREE_OBJECT (priv->cameras[1]), &up);
  gthree_object_look_at (GTHREE_OBJECT (priv->cameras[1]), &look_at);

  /* PY: +Y */
  graphene_vec3_init (&up, 0, 0, -1);
  graphene_vec3_init (&look_at, 0, 1, 0);
  gthree_object_set_up (GTHREE_OBJECT (priv->cameras[2]), &up);
  gthree_object_look_at (GTHREE_OBJECT (priv->cameras[2]), &look_at);

  /* NY: -Y */
  graphene_vec3_init (&up, 0, 0, 1);
  graphene_vec3_init (&look_at, 0, -1, 0);
  gthree_object_set_up (GTHREE_OBJECT (priv->cameras[3]), &up);
  gthree_object_look_at (GTHREE_OBJECT (priv->cameras[3]), &look_at);

  /* PZ: +Z */
  graphene_vec3_init (&up, 0, 1, 0);
  graphene_vec3_init (&look_at, 0, 0, 1);
  gthree_object_set_up (GTHREE_OBJECT (priv->cameras[4]), &up);
  gthree_object_look_at (GTHREE_OBJECT (priv->cameras[4]), &look_at);

  /* NZ: -Z */
  graphene_vec3_init (&up, 0, 1, 0);
  graphene_vec3_init (&look_at, 0, 0, -1);
  gthree_object_set_up (GTHREE_OBJECT (priv->cameras[5]), &up);
  gthree_object_look_at (GTHREE_OBJECT (priv->cameras[5]), &look_at);

  return cube_camera;
}

void
gthree_cube_camera_update (GthreeCubeCamera *cube_camera,
                           GthreeRenderer   *renderer,
                           GthreeScene      *scene)
{
  GthreeCubeCameraPrivate *priv = gthree_cube_camera_get_instance_private (cube_camera);
  gboolean generate_mipmaps;
  int i;

  gthree_object_update_matrix_world (GTHREE_OBJECT (cube_camera), FALSE);

  generate_mipmaps = gthree_texture_get_generate_mipmaps (
    gthree_render_target_get_texture (priv->render_target));

  /* Disable mipmap generation for the first 5 faces, enable for the last
     so mipmaps are generated once after all faces are rendered */
  gthree_texture_set_generate_mipmaps (
    gthree_render_target_get_texture (priv->render_target), FALSE);

  for (i = 0; i < 6; i++)
    {
      if (i == 5)
        gthree_texture_set_generate_mipmaps (
          gthree_render_target_get_texture (priv->render_target), generate_mipmaps);

      gthree_renderer_set_render_target (renderer, priv->render_target, i, 0);
      gthree_renderer_render (renderer, scene, GTHREE_CAMERA (priv->cameras[i]));
    }

  gthree_renderer_set_render_target (renderer, NULL, 0, 0);

  gthree_texture_set_needs_pmrem_update (
    gthree_render_target_get_texture (priv->render_target));
}

/**
 * gthree_cube_camera_get_render_target:
 *
 * Returns: (transfer none): The cube render target
 */
GthreeRenderTarget *
gthree_cube_camera_get_render_target (GthreeCubeCamera *cube_camera)
{
  GthreeCubeCameraPrivate *priv = gthree_cube_camera_get_instance_private (cube_camera);
  return priv->render_target;
}
