#include <math.h>
#include <epoxy/gl.h>

#include "gthreerenderer.h"
#include "gthreeobjectprivate.h"
#include "gthreeshader.h"
#include "gthreematerial.h"
#include "gthreeprivate.h"

typedef struct {
  int width;
  int height;
  gboolean auto_clear;
  gboolean auto_clear_color;
  gboolean auto_clear_depth;
  gboolean auto_clear_stencil;
  GdkRGBA clear_color;
  gboolean sort_objects;
  GthreeMaterial *override_material;

  float viewport_x;
  float viewport_y;
  float viewport_width;
  float viewport_height;

  /* Render state */
  graphene_frustum_t frustum;
  graphene_matrix_t proj_screen_matrix;

  guint used_texture_units;

  gboolean lights_need_update;
  GthreeLightSetup light_setup;

  gboolean old_flip_sided;
  gboolean old_double_sided;
  gboolean old_depth_test;
  gboolean old_depth_write;
  float old_line_width;
  gboolean old_polygon_offset;
  float old_polygon_offset_factor;
  float old_polygon_offset_units;
  GthreeBlendMode old_blending;
  guint old_blend_equation;
  guint old_blend_src;
  guint old_blend_dst;
  GthreeProgram *current_program;
  GthreeMaterial *current_material;
  GthreeCamera *current_camera;

  GPtrArray *opaque_objects;
  GPtrArray *transparent_objects;

  guint8 new_attributes[8];
  guint8 enabled_attributes[8];

  int max_textures;
  int max_vertex_textures;
  int max_texture_size;
  int max_cubemap_size;
  float max_anisotropy;

  gboolean supports_vertex_textures;
  gboolean supports_bone_textures;

} GthreeRendererPrivate;

static void gthree_set_default_gl_state (GthreeRenderer *renderer);

G_DEFINE_TYPE_WITH_PRIVATE (GthreeRenderer, gthree_renderer, G_TYPE_OBJECT);

GthreeRenderer *
gthree_renderer_new ()
{
  GthreeRenderer *renderer;

  renderer = g_object_new (gthree_renderer_get_type (),
                           NULL);

  return renderer;
}

static void
gthree_renderer_init (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear = TRUE;
  priv->auto_clear_color = TRUE;
  priv->auto_clear_depth = TRUE;
  priv->auto_clear_stencil = TRUE;
  priv->sort_objects = TRUE;
  priv->width = 1;
  priv->height = 1;
  priv->lights_need_update = TRUE;

  priv->light_setup.dir_len = 0;
  priv->light_setup.dir_colors = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.dir_positions = g_array_new (FALSE, TRUE, sizeof (float));

  priv->light_setup.point_len = 0;
  priv->light_setup.point_colors = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.point_positions = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.point_distances = g_array_new (FALSE, TRUE, sizeof (float));

  priv->light_setup.spot_len = 0;
  priv->light_setup.spot_colors = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.spot_positions = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.spot_distances = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.spot_directions = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.spot_angles_cos = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.spot_exponents = g_array_new (FALSE, TRUE, sizeof (float));

  priv->light_setup.hemi_len = 0;
  priv->light_setup.hemi_sky_colors = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.hemi_ground_colors = g_array_new (FALSE, TRUE, sizeof (float));
  priv->light_setup.hemi_positions = g_array_new (FALSE, TRUE, sizeof (float));
  
  priv->opaque_objects = g_ptr_array_new ();
  priv->transparent_objects = g_ptr_array_new ();

  priv->old_blending = -1;
  priv->old_blend_equation = -1;
  priv->old_blend_src = -1;
  priv->old_blend_dst = -1;
  priv->old_depth_write = -1;
  priv->old_depth_test = -1;

  gthree_set_default_gl_state (renderer);

  // GPU capabilities
  glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS, &priv->max_textures);
  glGetIntegerv (GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &priv->max_vertex_textures);
  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &priv->max_texture_size);
  glGetIntegerv (GL_MAX_CUBE_MAP_TEXTURE_SIZE, &priv->max_cubemap_size);

  priv->max_anisotropy = 0.0f;
  if (epoxy_has_gl_extension("GL_EXT_texture_filter_anisotropic"))
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &priv->max_anisotropy);

  priv->supports_vertex_textures = priv->max_vertex_textures > 0;
  priv->supports_bone_textures =
    priv->supports_vertex_textures &&
    epoxy_has_gl_extension("GL_ARB_texture_float");

  //priv->compressed_texture_formats = _glExtensionCompressedTextureS3TC ? glGetParameter( _gl.COMPRESSED_TEXTURE_FORMATS ) : [];

}

static void
gthree_renderer_finalize (GObject *obj)
{
  GthreeRenderer *renderer = GTHREE_RENDERER (obj);
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  g_array_free (priv->light_setup.dir_colors, TRUE);
  g_array_free (priv->light_setup.dir_positions, TRUE);

  g_array_free (priv->light_setup.point_colors, TRUE);
  g_array_free (priv->light_setup.point_positions, TRUE);
  g_array_free (priv->light_setup.point_distances, TRUE);

  g_array_free (priv->light_setup.spot_colors, TRUE);
  g_array_free (priv->light_setup.spot_positions, TRUE);
  g_array_free (priv->light_setup.spot_distances, TRUE);
  g_array_free (priv->light_setup.spot_directions, TRUE);
  g_array_free (priv->light_setup.spot_angles_cos, TRUE);
  g_array_free (priv->light_setup.spot_exponents, TRUE);

  g_array_free (priv->light_setup.hemi_sky_colors, TRUE);
  g_array_free (priv->light_setup.hemi_ground_colors, TRUE);
  g_array_free (priv->light_setup.hemi_positions, TRUE);

  g_ptr_array_free (priv->opaque_objects, TRUE);
  g_ptr_array_free (priv->transparent_objects, TRUE);
  
  G_OBJECT_CLASS (gthree_renderer_parent_class)->finalize (obj);
}

static void
gthree_renderer_class_init (GthreeRendererClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_renderer_finalize;
}

void
gthree_renderer_set_viewport (GthreeRenderer *renderer,
                              float           x,
                              float           y,
                              float           width,
                              float           height)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->viewport_x = x;
  priv->viewport_y = y;
  priv->viewport_width = width;
  priv->viewport_height = height;

  glViewport (x, y, width, height);
}

void
gthree_renderer_set_size (GthreeRenderer *renderer,
                          int             width,
                          int             height)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->width = width;
  priv->height = height;

  gthree_renderer_set_viewport (renderer, 0, 0, width, height);
}

void
gthree_renderer_set_autoclear (GthreeRenderer *renderer,
                               gboolean        auto_clear)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear = !!auto_clear;
}

void
gthree_renderer_set_autoclear_color (GthreeRenderer *renderer,
                                     gboolean        clear_color)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear_color = !!clear_color;
}

void
gthree_renderer_set_autoclear_depth (GthreeRenderer *renderer,
                                     gboolean        clear_depth)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear_depth = !!clear_depth;
}

void
gthree_renderer_set_autoclear_stencil (GthreeRenderer *renderer,
                                       gboolean        clear_stencil)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->auto_clear_stencil = !!clear_stencil;
}

void
gthree_renderer_set_clear_color (GthreeRenderer *renderer,
                                 GdkRGBA        *color)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->clear_color = *color;

  glClearColor (priv->clear_color.red, priv->clear_color.green, priv->clear_color.blue, priv->clear_color.alpha );
}

static void
gthree_set_default_gl_state (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  glClearColor (0, 0, 0, 1);
  glClearDepth (1);
  glClearStencil (0);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  glFrontFace (GL_CCW);
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);

  glEnable (GL_BLEND);
  glBlendEquation (GL_FUNC_ADD);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport (priv->viewport_x, priv->viewport_y, priv->viewport_width, priv->viewport_height);
  glClearColor (priv->clear_color.red, priv->clear_color.green, priv->clear_color.blue, priv->clear_color.alpha );
};

void
gthree_renderer_clear (GthreeRenderer *renderer)
{
}

static gint
painter_sort_stable (gconstpointer  _a, gconstpointer  _b)
{
  const GthreeBuffer *a = *(GthreeBuffer **)_a;
  const GthreeBuffer *b = *(GthreeBuffer **)_b;

  if (a->z != b->z)
    {
      if (b->z > a->z)
        return 1;
      else
        return -1;
    }
  else if (a->object != b->object)
    {
      if ((gsize)a->object > (gsize)b->object)
        return 1;
      else
        return -1;
    }

  return 0;
}

static gint
reverse_painter_sort_stable (gconstpointer _a, gconstpointer _b)
{
  const GthreeBuffer *a = *(GthreeBuffer **)_a;
  const GthreeBuffer *b = *(GthreeBuffer **)_b;

  if (a->z != b->z)
    {
      if (a->z > b->z)
        return 1;
      else
        return -1;
    }
  else if (a->object != b->object)
    {
      if ((gsize)a->object > (gsize)b->object)
        return 1;
      else
        return -1;
    }

  return 0;
}

static void
set_material_faces (GthreeRenderer *renderer,
                    GthreeMaterial *material)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeSide side = gthree_material_get_side (material);
  gboolean double_sided = side == GTHREE_SIDE_DOUBLE;
  gboolean flip_sided = side == GTHREE_SIDE_BACK;

  if (priv->old_double_sided != double_sided )
    {
      if (double_sided)
        glDisable (GL_CULL_FACE);
      else
        glEnable (GL_CULL_FACE);

      priv->old_double_sided = double_sided;
    }

  if (priv->old_flip_sided != flip_sided ) {
    if (flip_sided)
      glFrontFace (GL_CW);
    else
      glFrontFace (GL_CCW);

    priv->old_flip_sided = flip_sided;
  }
}

static void
set_depth_test (GthreeRenderer *renderer,
                gboolean depth_test)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_depth_test != depth_test)
    {
      if (depth_test)
        glEnable (GL_DEPTH_TEST);
      else
        glDisable (GL_DEPTH_TEST);

      priv->old_depth_test = depth_test;
    }
}

static void
set_depth_write (GthreeRenderer *renderer,
                gboolean depth_write)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_depth_write != depth_write)
    {
      glDepthMask (depth_write);
      priv->old_depth_write = depth_write;
    }
}

static void
set_line_width (GthreeRenderer *renderer,
                float line_width)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_line_width != line_width)
    {
      glLineWidth (line_width);
      priv->old_line_width = line_width;
    }
}

static void
set_polygon_offset (GthreeRenderer *renderer,
                    gboolean polygon_offset,
                    float factor, float units)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (priv->old_polygon_offset != polygon_offset)
    {
      if (polygon_offset)
        glEnable (GL_POLYGON_OFFSET_FILL);
      else
        glDisable (GL_POLYGON_OFFSET_FILL);

      priv->old_polygon_offset = polygon_offset;
    }

  if (polygon_offset && (priv->old_polygon_offset_factor != factor || priv->old_polygon_offset_units != units ))
    {
      glPolygonOffset (factor, units);
      priv->old_polygon_offset_factor = factor;
      priv->old_polygon_offset_units = units;
  }
}

static void
set_blending (GthreeRenderer *renderer,
              GthreeBlendMode blending,
              guint blend_equation,
              guint blend_src,
              guint blend_dst)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  if (blending != priv->old_blending)
    {
      switch (blending)
        {
        default:
        case GTHREE_BLEND_NO:
          glDisable (GL_BLEND);
          break;

        case GTHREE_BLEND_NORMAL:
          glEnable (GL_BLEND);
          glBlendEquationSeparate (GL_FUNC_ADD, GL_FUNC_ADD);
          glBlendFuncSeparate (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          break;

        case GTHREE_BLEND_ADDITIVE:
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_SRC_ALPHA, GL_ONE);
          break;

        case GTHREE_BLEND_SUBTRACTIVE:
          // TODO: Find blendFuncSeparate() combination
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
          break;

        case GTHREE_BLEND_MULTIPLY:
          // TODO: Find blendFuncSeparate() combination
          glEnable (GL_BLEND);
          glBlendEquation (GL_FUNC_ADD);
          glBlendFunc (GL_ZERO, GL_SRC_COLOR);
          break;

        case GTHREE_BLEND_CUSTOM:
          glEnable (GL_BLEND);
          break;
        }
      priv->old_blending = blending;
    }

  if (blending == GTHREE_BLEND_CUSTOM)
    {
      if (blend_equation != priv->old_blend_equation)
        {
          glBlendEquation (blend_equation);
          priv->old_blend_equation = blend_equation;
        }

      if (blend_src != priv->old_blend_src || blend_dst != priv->old_blend_dst)
        {
          glBlendFunc (blend_src, blend_dst);

          priv->old_blend_src = blend_src;
          priv->old_blend_dst = blend_dst;
        }
    }
  else
    {
      priv->old_blend_equation = -1;
      priv->old_blend_src = -1;
      priv->old_blend_dst = -1;
    }
}

static void
resolve_buffer_material (GthreeRenderer *renderer,
                         GthreeBuffer *buffer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeMaterial *material = gthree_buffer_resolve_material (buffer);

  if (material)
    {
      if (gthree_material_get_is_transparent (material))
        g_ptr_array_add (priv->transparent_objects, buffer);
      else
        g_ptr_array_add (priv->opaque_objects, buffer);
    }
}

static void
project_object (GthreeRenderer *renderer,
                GthreeScene    *scene,
                GthreeObject   *object,
                GthreeCamera   *camera)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GList *l, *buffers;
  GthreeObject *child;
  GthreeObjectIter iter;

  if (!gthree_object_get_visible (object))
    return;

  buffers = gthree_object_get_buffers (object);

  if (buffers != NULL && (!gthree_object_get_is_frustum_culled (object) || gthree_object_is_in_frustum (object, &priv->frustum)))
    {
      gthree_object_update (object);

      for (l = buffers; l != NULL; l = l->next)
        {
          GthreeBuffer *buffer = l->data;

          resolve_buffer_material (renderer, buffer);

          if (priv->sort_objects)
            {
              /* TODO
              if (object.renderDepth != null)
                {
                  buffer->z = object.renderDepth;
                }
                else */
                {
                  graphene_vec4_t vector;

                  /* Get position */
                  graphene_matrix_get_row (gthree_object_get_world_matrix (object), 3, &vector);

                  /* project object position to screen */
                  graphene_matrix_transform_vec4 (&priv->proj_screen_matrix, &vector, &vector);
                  graphene_vec4_normalize (&vector, &vector);

                  buffer->z = graphene_vec4_get_z (&vector);
                }
            }
        }
    }

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    project_object (renderer, scene, child, camera);
}

static GthreeProgram *
init_material (GthreeRenderer *renderer,
               GthreeMaterial *material,
               GList *lights,
               gpointer fog,
               GthreeObject *object)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GList *l;
  //char *shader_id;
  GthreeProgram *program;
  GthreeShader *shader;
  GthreeProgramParameters parameters = {0};
  gpointer code;

  shader = gthree_material_get_shader (material);

  //material.addEventListener( 'dispose', onMaterialDispose );
  //var u, a, identifiers, i, parameters, maxBones, maxShadows, shaderID;

  // heuristics to create shader parameters according to lights in the scene
  // (not to blow over maxLights budget)

#ifdef TODO
  maxShadows = allocateShadows( lights );
  maxBones = allocateBones( object );
#endif

  parameters.precision = GTHREE_PRECISION_HIGH;
  parameters.supports_vertex_textures = priv->supports_vertex_textures;

  gthree_material_set_params (material, &parameters);
  for (l = lights; l != NULL; l = l->next)
    gthree_light_set_params (l->data, &parameters);

#ifdef TODO
  parameters =
    {
    precision: _precision,

    map: !! material.map,
    envMap: !! material.envMap,
    lightMap: !! material.lightMap,
    bumpMap: !! material.bumpMap,
    normalMap: !! material.normalMap,
    specularMap: !! material.specularMap,
    alphaMap: !! material.alphaMap,

    vertexColors: material.vertexColors,

    fog: fog,
    useFog: material.fog,
    fogExp: fog instanceof THREE.FogExp2,

    sizeAttenuation: material.sizeAttenuation,
    logarithmicDepthBuffer: _logarithmicDepthBuffer,

    skinning: material.skinning,
    maxBones: maxBones,
    useVertexTexture: _supportsBoneTextures && object && object.skeleton && object.skeleton.useVertexTexture,

    morphTargets: material.morphTargets,
    morphNormals: material.morphNormals,
    maxMorphTargets: this.maxMorphTargets,
    maxMorphNormals: this.maxMorphNormals,

    maxShadows: maxShadows,
    shadowMapEnabled: this.shadowMapEnabled && object.receiveShadow && maxShadows > 0,
    shadowMapType: this.shadowMapType,
    shadowMapDebug: this.shadowMapDebug,
    shadowMapCascade: this.shadowMapCascade,

    alphaTest: material.alphaTest,
    metal: material.metal,
    wrapAround: material.wrapAround,
    };
#endif

  code = NULL;

#ifdef TODO

  // Generate code
  var chunks = [];

  if (shader_id)
    {
      chunks.push( shader_id );
    }
  else
    {
      chunks.push( material.fragmentShader );
      chunks.push( material.vertexShader );
    }

  for ( var d in material.defines )
    {
      chunks.push( d );
      chunks.push( material.defines[ d ] );
    }

  for ( var p in parameters )
    {
      chunks.push( p );
      chunks.push( parameters[ p ] );
    }

  var code = chunks.join();
#endif

  program = NULL;

#ifdef TODO
  // Check if code has been already compiled
  for ( var p = 0, pl = _programs.length; p < pl; p ++ )
    {
      var programInfo = _programs[ p ];
      if (programInfo.code == code)
        {
          program = programInfo;
          program.usedTimes ++;
          break;
        }
    }
#endif

  if (program == NULL)
    {
      program = gthree_program_new (code, shader, &parameters);
      //_programs.push( program );
    }

  material->program = program;

#ifdef TODO
  var attributes = material.program.attributes;
  if (material.morphTargets)
    {
      material.numSupportedMorphTargets = 0;
      var id, base = 'morphTarget';
      for ( i = 0; i < this.maxMorphTargets; i ++ )
        {
          id = base + i;
          if ( attributes[ id ] >= 0 )
            {
              material.numSupportedMorphTargets ++;
            }

        }
    }

  if (material.morphNormals )
    {
      material.numSupportedMorphNormals = 0;
      var id, base = 'morphNormal';

      for ( i = 0; i < this.maxMorphNormals; i ++ )
        {
          id = base + i;
          if ( attributes[ id ] >= 0 )
            material.numSupportedMorphNormals ++;
        }
    }
#endif

  gthree_shader_update_uniform_locations_for_program (shader, program);

  return NULL;
}

#if 0
static void
print_matrix4 (float *s)
{
  int i,j;
  for (i = 0; i < 4; i++)
    {
      if (i == 0)
        g_print("[ ");
      else
        g_print("  ");
      for (j = 0; j < 4; j++)
        {
          if (j != 0)
            g_print (", ");
          g_print ("%f", *s++);
        }
      if (i == 3)
        g_print("]\n");
      else
        g_print("\n");
    }
}
#endif

static void
load_uniforms_matrices (GthreeRenderer *renderer,
                        GthreeProgram *program,
                        GthreeObject *object)
{
  float matrix[16];
  int mvm_location = gthree_program_lookup_uniform_location (program, "modelViewMatrix");
  int nm_location = gthree_program_lookup_uniform_location (program, "normalMatrix");

  gthree_object_get_model_view_matrix_floats (object, matrix);
  glUniformMatrix4fv (mvm_location, 1, FALSE, matrix);

  if (nm_location >= 0)
    {
      gthree_object_get_normal_matrix3_floats (object, matrix);
      glUniformMatrix3fv (nm_location, 1, FALSE, matrix);
    }
}

static void
setup_lights (GthreeRenderer *renderer, GList *lights)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeLightSetup *setup = &priv->light_setup;
  GList *l;
  int i;

  setup->ambient.red = 0;
  setup->ambient.green = 0;
  setup->ambient.blue = 0;
  setup->ambient.alpha = 1;

  setup->dir_len = 0;
  setup->dir_count = 0;
  setup->point_len = 0;
  setup->point_count = 0;
  setup->spot_len = 0;
  setup->spot_count = 0;
  setup->hemi_len = 0;
  setup->hemi_count = 0;

  for (l = lights; l != NULL; l = l->next)
    {
      GthreeLight *light = l->data;

      if (gthree_light_get_is_only_shadow (light))
	continue;

      gthree_light_setup (light, &priv->light_setup);

#if TODO
      else if ( light instanceof THREE.SpotLight )
	{
	  spotCount += 1;

	  if ( ! light.visible ) continue;

	  spotOffset = spotLength * 3;

	  if ( _this.gammaInput )
	    setColorGamma( spotColors, spotOffset, color, intensity * intensity );
	  else
	    setColorLinear( spotColors, spotOffset, color, intensity );

	  _vector3.setFromMatrixPosition( light.matrixWorld );

	  spotPositions[ spotOffset ]     = _vector3.x;
	  spotPositions[ spotOffset + 1 ] = _vector3.y;
	  spotPositions[ spotOffset + 2 ] = _vector3.z;

	  spotDistances[ spotLength ] = distance;

	  _direction.copy( _vector3 );
	  _vector3.setFromMatrixPosition( light.target.matrixWorld );
	  _direction.sub( _vector3 );
	  _direction.normalize();

	  spotDirections[ spotOffset ]     = _direction.x;
	  spotDirections[ spotOffset + 1 ] = _direction.y;
	  spotDirections[ spotOffset + 2 ] = _direction.z;

	  spotAnglesCos[ spotLength ] = Math.cos( light.angle );
	  spotExponents[ spotLength ] = light.exponent;

	  spotLength += 1;
	}
      else if ( light instanceof THREE.HemisphereLight )
	{
	  hemiCount += 1;

	  if ( ! light.visible ) continue;

	  _direction.setFromMatrixPosition( light.matrixWorld );
	  _direction.normalize();

	  hemiOffset = hemiLength * 3;

	  hemiPositions[ hemiOffset ]     = _direction.x;
	  hemiPositions[ hemiOffset + 1 ] = _direction.y;
	  hemiPositions[ hemiOffset + 2 ] = _direction.z;

	  skyColor = light.color;
	  groundColor = light.groundColor;

	  if ( _this.gammaInput )
	    {
	      intensitySq = intensity * intensity;

	      setColorGamma( hemiSkyColors, hemiOffset, skyColor, intensitySq );
	      setColorGamma( hemiGroundColors, hemiOffset, groundColor, intensitySq );
	    }
	  else
	    {
	      setColorLinear( hemiSkyColors, hemiOffset, skyColor, intensity );
	      setColorLinear( hemiGroundColors, hemiOffset, groundColor, intensity );
	    }

	  hemiLength += 1;
	}
#endif
    }


  // null eventual remains from removed lights
  // (this is to avoid if in shader)
  g_array_set_size (setup->dir_colors, MAX(setup->dir_colors->len, setup->dir_count * 3));
  for (i = setup->dir_len * 3; i < setup->dir_colors->len; i++)
    g_array_index (setup->dir_colors, float, i) = 0.0;
	 
  g_array_set_size (setup->point_colors, MAX(setup->point_colors->len, setup->point_count * 3));
  for (i = setup->point_len * 3; i < setup->point_colors->len; i++)
    g_array_index (setup->point_colors, float, i) = 0.0;
	 
  g_array_set_size (setup->spot_colors, MAX(setup->spot_colors->len, setup->spot_count * 3));
  for (i = setup->spot_len * 3; i < setup->spot_colors->len; i++)
    g_array_index (setup->spot_colors, float, i) = 0.0;

  g_array_set_size (setup->hemi_sky_colors, MAX(setup->hemi_sky_colors->len, setup->hemi_count * 3));
  for (i = setup->hemi_len * 3; i < setup->hemi_sky_colors->len; i++)
    g_array_index (setup->hemi_sky_colors, float, i) = 0.0;

  g_array_set_size (setup->hemi_ground_colors, MAX(setup->hemi_ground_colors->len, setup->hemi_count * 3));
  for (i = setup->hemi_len * 3; i < setup->hemi_ground_colors->len; i++)
    g_array_index (setup->hemi_ground_colors, float, i) = 0.0;
}

static void
refresh_uniforms_lights (GthreeUniforms *uniforms, GthreeLightSetup *lights_setup)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, "ambientLightColor");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &lights_setup->ambient);


  uni = gthree_uniforms_lookup_from_string (uniforms, "directionalLightColor");
  if (uni != NULL)
    gthree_uniform_set_float3_array (uni, lights_setup->dir_colors);

  uni = gthree_uniforms_lookup_from_string (uniforms, "directionalLightDirection");
  if (uni != NULL)
    gthree_uniform_set_float3_array (uni, lights_setup->dir_positions);

  
  uni = gthree_uniforms_lookup_from_string (uniforms, "pointLightColor");
  if (uni != NULL)
    gthree_uniform_set_float3_array (uni, lights_setup->point_colors);

  uni = gthree_uniforms_lookup_from_string (uniforms, "pointLightPosition");
  if (uni != NULL)
    gthree_uniform_set_float3_array (uni, lights_setup->point_positions);

  uni = gthree_uniforms_lookup_from_string (uniforms, "pointLightDistance");
  if (uni != NULL)
    gthree_uniform_set_float_array (uni, lights_setup->point_distances);
  
#ifdef TODO
  uniforms.spotLightColor.value = lights_setup->spot.colors;
  uniforms.spotLightPosition.value = lights_setup->spot.positions;
  uniforms.spotLightDistance.value = lights_setup->spot.distances;
  uniforms.spotLightDirection.value = lights_setup->spot.directions;
  uniforms.spotLightAngleCos.value = lights_setup->spot.anglesCos;
  uniforms.spotLightExponent.value = lights_setup->spot.exponents;

  uniforms.hemisphereLightSkyColor.value = lights_setup->hemi.skyColors;
  uniforms.hemisphereLightGroundColor.value = lights_setup->hemi.groundColors;
  uniforms.hemisphereLightDirection.value = lights_setup->hemi.positions;
#endif
};

// If uniforms are marked as clean, they don't need to be loaded to the GPU.
static void
mark_uniforms_lights_needs_update (GthreeUniforms *uniforms, gboolean needs_update)
{
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, "ambientLightColor");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
    
  uni = gthree_uniforms_lookup_from_string (uniforms, "directionalLightColor");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "directionalLightDirection");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
    
  uni = gthree_uniforms_lookup_from_string (uniforms, "pointLightColor");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "pointLightPosition");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "pointLightDistance");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
    
  uni = gthree_uniforms_lookup_from_string (uniforms, "spotLightColor");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "spotLightPosition");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "spotLightDistance");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "spotLightDirection");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "spotLightAngleCos");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "spotLightExponent");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  
  uni = gthree_uniforms_lookup_from_string (uniforms, "hemisphereLightSkyColor");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "hemisphereLightGroundColor");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
  uni = gthree_uniforms_lookup_from_string (uniforms, "hemisphereLightDirection");
  if (uni)
    gthree_uniform_set_needs_update (uni, needs_update);
}

static GthreeProgram *
set_program (GthreeRenderer *renderer,
             GthreeCamera *camera,
             GList *lights,
             gpointer fog,
             GthreeMaterial *material,
             GthreeObject *object)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  gboolean refreshProgram = false;
  gboolean refreshMaterial = false;
  gboolean refreshLights = false;
  GthreeProgram *program;
  GthreeShader *shader;
  GthreeUniforms *m_uniforms;
  int location;

  priv->used_texture_units = 0;

  if (gthree_material_get_needs_update (material))
    {
      g_clear_object (&material->program);
      init_material (renderer, material, lights, fog, object);
      gthree_material_set_needs_update (material, FALSE);
    }

  program = material->program;
  shader = gthree_material_get_shader (material);
  m_uniforms = gthree_shader_get_uniforms (shader);

  if (program != priv->current_program )
    {
      gthree_program_use (program);
      priv->current_program = program;

      refreshProgram = TRUE;
      refreshMaterial = TRUE;
      refreshLights = TRUE;
    }

  if (material != priv->current_material)
    {
      if (priv->current_material == NULL)
        refreshLights = TRUE;
      priv->current_material = material;
      refreshMaterial = TRUE;
    }

  if (refreshProgram || camera != priv->current_camera)
    {
      const graphene_matrix_t *projection_matrix = gthree_camera_get_projection_matrix (camera);
      float projection_matrixv[16];
      gint proction_matrix_location = gthree_program_lookup_uniform_location (program, "projectionMatrix");

      graphene_matrix_to_float (projection_matrix, projection_matrixv);
      glUniformMatrix4fv (proction_matrix_location, 1, FALSE, projection_matrixv);

#ifdef TODO
      if ( _logarithmicDepthBuffer )
        glUniform1f (uniform_locations.logDepthBufFC, 2.0 / ( Math.log( camera.far + 1.0 ) / Math.LN2 ));

#endif
      if (camera != priv->current_camera)
        priv->current_camera = camera;

      // load material specific uniforms
      // (shader material also gets them for the sake of genericity)

      if (gthree_material_needs_camera_pos (material)
#if TODO
          || material.envMap
#endif
          )
        {
          gint camera_position_location = gthree_program_lookup_uniform_location (program, "cameraPosition");
          if (camera_position_location >= 0)
            {
              const graphene_matrix_t *camera_matrix_world = gthree_object_get_world_matrix (GTHREE_OBJECT (camera));
              graphene_vec4_t pos;
              graphene_matrix_get_row (camera_matrix_world, 3, &pos);
              glUniform3f (camera_position_location,
                           graphene_vec4_get_x (&pos), graphene_vec4_get_y (&pos), graphene_vec4_get_z (&pos));
            }
        }

      if (gthree_material_needs_view_matrix (material)
#if TODO
	  || material.skinning
#endif
	  )
        {
	  gint view_matrix_location = gthree_program_lookup_uniform_location (program, "viewMatrix");
          if (view_matrix_location >= 0)
            {
	      const graphene_matrix_t *m = gthree_camera_get_world_inverse_matrix (camera);
	      float floats[16];
	      graphene_matrix_to_float (m, floats);
              glUniformMatrix4fv (view_matrix_location, 1, FALSE, floats);
            }
        }
    }

  // skinning uniforms must be set even if material didn't change
  // auto-setting of texture unit for bone texture must go before other textures
  // not sure why, but otherwise weird things happen

#if TODO
  if ( material.skinning )
    {
      if ( object.bindMatrix && uniform_locations.bindMatrix !== null )
        glUniformMatrix4fv( uniform_locations.bindMatrix, false, object.bindMatrix.elements );

      if ( object.bindMatrixInverse && uniform_locations.bindMatrixInverse !== null )
        glUniformMatrix4fv( uniform_locations.bindMatrixInverse, false, object.bindMatrixInverse.elements );

      if ( _supportsBoneTextures && object.skeleton && object.skeleton.useVertexTexture )
        {
          if ( uniform_locations.boneTexture !== null )
            {
              var textureUnit = getTextureUnit();
              glUniform1i( uniform_locations.boneTexture, textureUnit );
              _this.setTexture( object.skeleton.boneTexture, textureUnit );
            }

          if ( uniform_locations.boneTextureWidth !== null )
            glUniform1i( uniform_locations.boneTextureWidth, object.skeleton.boneTextureWidth );

          if ( uniform_locations.boneTextureHeight !== null )
            glUniform1i( uniform_locations.boneTextureHeight, object.skeleton.boneTextureHeight );

        }
      else if ( object.skeleton && object.skeleton.boneMatrices )
        {
          if ( uniform_locations.boneGlobalMatrices !== null )
            glUniformMatrix4fv( uniform_locations.boneGlobalMatrices, false, object.skeleton.boneMatrices );
        }
    }
#endif

  if ( refreshMaterial )
    {
      gthree_material_set_uniforms (material, m_uniforms, camera);

#if TODO
    // refresh uniforms common to several materials
      if ( fog && material.fog )
        refreshUniformsFog( m_uniforms, fog );
#endif

      if (gthree_material_needs_lights (material))
        {
          if (priv->lights_need_update )
            {
              refreshLights = TRUE;
              setup_lights (renderer, lights);
              priv->lights_need_update = false;
            }

          if (refreshLights)
            {
              refresh_uniforms_lights (m_uniforms, &priv->light_setup);
              mark_uniforms_lights_needs_update (m_uniforms, TRUE);
            }
          else
            {
              mark_uniforms_lights_needs_update (m_uniforms, FALSE);
            }
        }

      // refresh single material specific uniforms

#if TODO
      if ( material instanceof THREE.LineBasicMaterial )
        {
          refreshUniformsLine( m_uniforms, material );
        }
      else if ( material instanceof THREE.LineDashedMaterial )
        {
          refreshUniformsLine( m_uniforms, material );
          refreshUniformsDash( m_uniforms, material );
        }
      else if ( material instanceof THREE.PointCloudMaterial )
        {
          refreshUniformsParticle( m_uniforms, material );
        }

      if ( object.receiveShadow && ! material._shadowPass )
        {
          refreshUniformsShadow( m_uniforms, lights );
        }
#endif

      // load common uniforms
      gthree_uniforms_load (m_uniforms, renderer);
    }

  load_uniforms_matrices (renderer, program, object);

  location = gthree_program_lookup_uniform_location (program, "modelMatrix");
  if (location >= 0)
    {
      float matrix[16];
      gthree_object_get_world_matrix_floats (object, matrix);
      glUniformMatrix4fv (location, 1, FALSE, matrix);
    }

  return program;
}

static void
init_attributes (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  int i;

  for (i = 0; i < G_N_ELEMENTS(priv->new_attributes); i++)
    priv->new_attributes[i] = 0;
}

static void
enable_attribute (GthreeRenderer *renderer,
                  guint attribute)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);

  priv->new_attributes[attribute] = 1;
  if (priv->enabled_attributes[attribute] == 0)
    {
      glEnableVertexAttribArray(attribute);
      priv->enabled_attributes[attribute] = 1;
    }
}

static void
disable_unused_attributes (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  int i;

  for (i = 0; i < G_N_ELEMENTS(priv->new_attributes); i++)
    {
      if (priv->enabled_attributes[i] != priv->new_attributes[i])
        {
          glDisableVertexAttribArray(i);
          priv->enabled_attributes[i] = 0;
        }
    }
}

static void
render_buffer (GthreeRenderer *renderer,
               GthreeCamera *camera,
               GList *lights,
               gpointer fog,
               GthreeMaterial *material,
               GthreeBuffer *buffer)
{
  GthreeObject *object = buffer->object;
  GthreeProgram *program = set_program (renderer, camera, lights, fog, material, object);
  //var linewidth, a, attribute, i, il;
  //var attributes = program.attributes;
  gboolean updateBuffers = false;
  gint position_location, color_location, uv_location, uv2_location, normal_location;
  //guint32 wireframeBit = gthree_material_get_is_wireframe (material) ? 1 : 0;
  //guint32 geometryGroupHash = (guint32)buffer + (guint32)program * 2 + wireframeBit;

  if (!gthree_material_get_is_visible (material))
    return;

  if (TRUE /* geometryGroupHash !== _currentGeometryGroupHash */)
    {
      //_currentGeometryGroupHash = geometryGroupHash;
      updateBuffers = true;
    }

  if (updateBuffers)
    init_attributes (renderer);

  // vertices
  position_location = gthree_program_lookup_attribute_location (program, "position");
  if (/*!material.morphTargets && */ position_location >= 0)
    {
      if (updateBuffers)
        {
          glBindBuffer (GL_ARRAY_BUFFER, buffer->vertex_buffer);
          enable_attribute (renderer, position_location);
          glVertexAttribPointer (position_location, 3, GL_FLOAT, FALSE, 0, NULL);
        }
    }
  else
    {
      /*
      if (object.morphTargetBase)
        setupMorphTargets( material, geometryGroup, object );
      */
    }

  if (updateBuffers)
    {
      // custom attributes
      // Use the per-geometryGroup custom attribute arrays which are setup in initMeshBuffers
#if TODO
      if (geometryGroup.__webglCustomAttributesList )
        {
          for ( i = 0, il = geometryGroup.__webglCustomAttributesList.length; i < il; i ++ )
            {
              attribute = geometryGroup.__webglCustomAttributesList[ i ];
              if ( attributes[ attribute.buffer.belongsToAttribute ] >= 0 )
                {
                  _gl.bindBuffer( _gl.ARRAY_BUFFER, attribute.buffer );
                  enableAttribute( attributes[ attribute.buffer.belongsToAttribute ] );
                  _gl.vertexAttribPointer( attributes[ attribute.buffer.belongsToAttribute ], attribute.size, _gl.FLOAT, false, 0, 0 );
                }
            }
        }
#endif

      // colors
      color_location = gthree_program_lookup_attribute_location (program, "color");
      if (color_location >= 0)
        {
          if (gthree_object_has_attribute_data (object, "color"))
            {
              glBindBuffer (GL_ARRAY_BUFFER, buffer->color_buffer);
              enable_attribute (renderer, color_location);
              glVertexAttribPointer (color_location, 3, GL_FLOAT, FALSE, 0, NULL);
            }
          else
            gthree_material_load_default_attribute (material, color_location, "color");
        }

      // uvs
      uv_location = gthree_program_lookup_attribute_location (program, "uv");
      if (uv_location >= 0)
        {
          if (gthree_object_has_attribute_data (object, "uv"))
            {
              glBindBuffer (GL_ARRAY_BUFFER, buffer->uv_buffer);
              enable_attribute (renderer, uv_location);
              glVertexAttribPointer (uv_location, 2, GL_FLOAT, FALSE, 0, NULL);
            }
          else
            gthree_material_load_default_attribute (material, uv_location, "uv");
        }

      uv2_location = gthree_program_lookup_attribute_location (program, "uv2");
      if (uv2_location >= 0)
        {
          if (gthree_object_has_attribute_data (object, "uv2"))
            {
              glBindBuffer (GL_ARRAY_BUFFER, buffer->uv2_buffer);
              enable_attribute (renderer, uv2_location);
              glVertexAttribPointer (uv2_location, 2, GL_FLOAT, FALSE, 0, NULL);
            }
          else
            gthree_material_load_default_attribute (material, uv2_location, "uv2");
        }

      // normals
      normal_location = gthree_program_lookup_attribute_location (program, "normal");
      if (normal_location >= 0 )
        {
          glBindBuffer (GL_ARRAY_BUFFER, buffer->normal_buffer);
          enable_attribute (renderer, normal_location);
          glVertexAttribPointer (normal_location, 3, GL_FLOAT, FALSE, 0, NULL);
        }

#ifdef TODO
      // skinning

      // line distances
#endif

      disable_unused_attributes (renderer);

      // render mesh
      if (TRUE /* object instanceof THREE.Mesh */ )
        {
          if (gthree_material_get_is_wireframe (material))
            {
              // wireframe
              set_line_width (renderer, gthree_material_get_wireframe_line_width (material));
              if (updateBuffers)
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, buffer->line_buffer);
              glDrawElements (GL_LINES, buffer->line_count, GL_UNSIGNED_SHORT, 0 );
            }
          else
            {
              // triangles
              if (updateBuffers)
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, buffer->face_buffer);
              glDrawElements (GL_TRIANGLES, buffer->face_count, GL_UNSIGNED_SHORT, 0 );
            }
        }
    }
}

static void
render_objects (GthreeRenderer *renderer,
                GPtrArray *render_list,
                GthreeCamera *camera,
                GList *lights,
                gpointer fog,
                gboolean use_blending,
                GthreeMaterial *override_material)
{
  GthreeBuffer *buffer;
  GthreeMaterial *material;
  int i;

  for (i = 0; i < render_list->len; i++)
    {
      buffer = g_ptr_array_index (render_list, i);

      gthree_object_update_matrix_view (buffer->object, gthree_camera_get_world_inverse_matrix (camera));

      if (override_material)
        material = override_material;
      else
        material = gthree_buffer_resolve_material (buffer);

      if (material == NULL)
        continue;

      if (use_blending)
        {
          guint equation, src_factor, dst_factor;
          GthreeBlendMode mode = gthree_material_get_blend_mode (material, &equation, &src_factor, &dst_factor);

          set_blending (renderer, mode, equation, src_factor, dst_factor);
        }

      set_depth_test (renderer, gthree_material_get_depth_test (material));
      set_depth_write (renderer, gthree_material_get_depth_write (material));


      {
        gboolean polygon_offset;
        float factor, units;

        polygon_offset = gthree_material_get_polygon_offset (material, &factor, &units);
        set_polygon_offset (renderer, polygon_offset, factor, units);
      }
      set_material_faces (renderer, material);

      render_buffer (renderer, camera, lights, fog, material, buffer);
    }
}

static void
clear (gboolean color, gboolean depth, gboolean stencil)
{
  guint bits = 0;
  if (color)
    bits |= GL_COLOR_BUFFER_BIT;
  if (depth)
    bits |= GL_DEPTH_BUFFER_BIT;
  if (stencil)
    bits |= GL_STENCIL_BUFFER_BIT;

  glClear (bits);
}

void
gthree_renderer_render (GthreeRenderer *renderer,
                        GthreeScene    *scene,
                        GthreeCamera   *camera,
                        gboolean force_clear)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  GthreeMaterial *override_material;
  GList *lights;
  gpointer fog;

  lights = gthree_scene_get_lights (scene);
  fog = NULL;

  priv->current_material = NULL;
  priv->current_camera = NULL;
  priv->lights_need_update = TRUE;

  /* update scene graph */

  gthree_object_update_matrix_world (GTHREE_OBJECT (scene), FALSE);

  /* update camera matrices and frustum */

  if (gthree_object_get_parent (GTHREE_OBJECT (camera)) == NULL)
    gthree_object_update_matrix_world (GTHREE_OBJECT (camera), FALSE);

  gthree_camera_update_matrix (camera);

  gthree_camera_get_proj_screen_matrix (camera, &priv->proj_screen_matrix);
  graphene_frustum_init_from_matrix (&priv->frustum, &priv->proj_screen_matrix);

  gthree_scene_realize_objects (scene);

  g_ptr_array_set_size (priv->opaque_objects, 0);
  g_ptr_array_set_size (priv->transparent_objects, 0);

  project_object (renderer, scene, GTHREE_OBJECT (scene), camera);

  if (priv->sort_objects)
    {
      g_ptr_array_sort (priv->opaque_objects, painter_sort_stable);
      g_ptr_array_sort (priv->transparent_objects, reverse_painter_sort_stable);
    }

  //this.setRenderTarget( renderTarget );

  if (priv->auto_clear || force_clear )
    clear (priv->auto_clear_color, priv->auto_clear_depth, priv->auto_clear_stencil);

  /* set matrices for regular objects (frustum culled) */


  override_material = gthree_scene_get_override_material (scene);
  if (override_material)
    {
      gboolean polygon_offset;
      float factor, units;
      guint equation, src_factor, dst_factor;
      GthreeBlendMode mode = gthree_material_get_blend_mode (override_material, &equation, &src_factor, &dst_factor);

      set_blending (renderer, mode, equation, src_factor, dst_factor);

      set_depth_test (renderer, gthree_material_get_depth_test (override_material));
      set_depth_write (renderer, gthree_material_get_depth_write (override_material));
      polygon_offset = gthree_material_get_polygon_offset (override_material, &factor, &units);
      set_polygon_offset (renderer, polygon_offset, factor, units);

      render_objects (renderer, priv->opaque_objects, camera, lights, fog, TRUE, override_material );
      render_objects (renderer, priv->transparent_objects, camera, lights, fog, TRUE, override_material );
    }
  else
    {
      // opaque pass (front-to-back order)

      set_blending (renderer, GTHREE_BLEND_NO, 0, 0, 0);
      render_objects (renderer, priv->opaque_objects, camera, lights, fog, FALSE, NULL);

      // transparent pass (back-to-front order)
      render_objects (renderer, priv->transparent_objects, camera, lights, fog, TRUE, NULL);
    }
}

guint
gthree_renderer_allocate_texture_unit (GthreeRenderer *renderer)
{
  GthreeRendererPrivate *priv = gthree_renderer_get_instance_private (renderer);
  guint texture_unit = priv->used_texture_units;

  if (texture_unit >= priv->max_textures )
    g_warning ("Trying to use %dtexture units while this GPU supports only %d",  texture_unit,priv->max_textures);

  priv->used_texture_units += 1;

  return texture_unit;
}
