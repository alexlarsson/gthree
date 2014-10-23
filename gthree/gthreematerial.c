#include <math.h>
#include <epoxy/gl.h>

#include "gthreematerial.h"

typedef struct {
  gboolean transparent;
  float opacity;
  gboolean visible;
  gboolean wireframe;
  float wireframe_line_width;
  GthreeBlendMode blend_mode;
  guint blend_equation;
  guint blend_src_factor;
  guint blend_dst_factor;
  gboolean polygon_offset;
  float polygon_offset_factor;
  float polygon_offset_units;
  gboolean depth_test;
  gboolean depth_write;
  float alpha_test;
  GthreeSide side;
} GthreeMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMaterial, gthree_material, G_TYPE_OBJECT);

GthreeMaterial *
gthree_material_new ()
{
  GthreeMaterial *material;

  material = g_object_new (gthree_material_get_type (),
                         NULL);


  return material;
}

static void
gthree_material_init (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  material->needs_update = TRUE;

  priv->visible = TRUE;
  priv->transparent = FALSE;
  priv->opacity = 1.0;
  priv->blend_mode = GTHREE_BLEND_NORMAL;
  priv->blend_equation = GL_FUNC_ADD;
  priv->blend_src_factor = GL_SRC_ALPHA;
  priv->blend_dst_factor = GL_ONE_MINUS_SRC_ALPHA;
  priv->depth_test = TRUE;
  priv->depth_write = TRUE;

  priv->polygon_offset = FALSE;
  priv->polygon_offset_factor = 0;
  priv->polygon_offset_units = 0;
  priv->alpha_test = 0;
  priv->side = GTHREE_SIDE_FRONT;

  priv->wireframe = FALSE;
  priv->wireframe_line_width = 1;
}


static void
gthree_material_finalize (GObject *obj)
{
  GthreeMaterial *material = GTHREE_MATERIAL (obj);
  //GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  g_clear_object (&material->program);
  g_clear_object (&material->shader);

  G_OBJECT_CLASS (gthree_material_parent_class)->finalize (obj);
}

GthreeMaterial *
gthree_material_resolve (GthreeMaterial *material,
                         int index)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->resolve)
    return class->resolve (material, index);

  return material;
}

void
gthree_material_set_params (GthreeMaterial *material,
                            GthreeProgramParameters *params)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  return class->set_params (material, params);
}


static void
gthree_material_real_set_params (GthreeMaterial *material,
                                 GthreeProgramParameters *params)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  params->double_sided = priv->side == GTHREE_SIDE_DOUBLE;
  params->flip_sided = priv->side == GTHREE_SIDE_BACK;
}

void
gthree_material_set_uniforms (GthreeMaterial *material,
                              GthreeUniforms *uniforms)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  return class->set_uniforms (material, uniforms);
}

static void
gthree_material_real_set_uniforms (GthreeMaterial *material,
                                   GthreeUniforms *uniforms)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, "opacity");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->opacity);
}

gboolean
gthree_material_needs_view_matrix (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_view_matrix)
    return class->needs_view_matrix (material);

  return FALSE;
}

gboolean
gthree_material_needs_uv (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_uv)
    return class->needs_uv (material);

  return FALSE;
}

gboolean
gthree_material_needs_lights (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_lights)
    return class->needs_lights (material);

  return FALSE;
}

GthreeShadingType
gthree_material_needs_normals (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_normals)
    return class->needs_normals (material);

  return GTHREE_SHADING_NONE;
}

GthreeColorType
gthree_material_needs_colors  (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_colors)
    return class->needs_colors (material);

  return GTHREE_COLOR_NONE;
}

static void
gthree_material_class_init (GthreeMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_material_real_set_uniforms;
}

gboolean
gthree_material_get_is_visible (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->visible;
}

void
gthree_material_set_is_visible (GthreeMaterial *material,
                                gboolean visible)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->visible = !!visible;

  material->needs_update = TRUE;
}

gboolean
gthree_material_get_is_wireframe (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->wireframe;
}

void
gthree_material_set_is_wireframe (GthreeMaterial *material,
                                  gboolean wireframe)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->wireframe = wireframe;
  material->needs_update = TRUE;
}

float
gthree_material_get_wireframe_line_width (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->wireframe_line_width;
}

void
gthree_material_set_wireframe_line_width (GthreeMaterial *material,
                                          float line_width)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->wireframe_line_width = line_width;
  material->needs_update = TRUE;
}

float
gthree_material_get_opacity (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->opacity;
}

void
gthree_material_set_opacity (GthreeMaterial *material,
                             float opacity)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->opacity = opacity;
  material->needs_update = TRUE;
}

float
gthree_material_get_alpha_test (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->alpha_test;
}

void
gthree_material_set_alpha_test (GthreeMaterial *material,
                                float alpha_test)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->alpha_test = alpha_test;
  material->needs_update = TRUE;
}

gboolean
gthree_material_get_is_transparent (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->transparent;
}

void
gthree_material_set_is_transparent (GthreeMaterial *material,
                                    gboolean transparent)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->transparent = !!transparent;

  material->needs_update = TRUE;
}

GthreeBlendMode
gthree_material_get_blend_mode (GthreeMaterial *material,
                                guint *equation,
                                guint *src_factor,
                                guint *dst_factor)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  if (equation)
    *equation = priv->blend_equation;
  if (src_factor)
    *src_factor = priv->blend_src_factor;
  if (dst_factor)
    *dst_factor = priv->blend_dst_factor;

  return priv->blend_mode;
}

void
gthree_material_set_blend_mode (GthreeMaterial       *material,
                                GthreeBlendMode       mode,
                                guint                 equation,
                                guint                 src_factor,
                                guint                 dst_factor)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->blend_mode = mode;
  priv->blend_equation = equation;
  priv->blend_src_factor = src_factor;
  priv->blend_dst_factor = dst_factor;

  material->needs_update = TRUE;
}


gboolean
gthree_material_get_polygon_offset (GthreeMaterial *material,
                                    float *factor, float *units)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  if (factor)
    *factor = priv->polygon_offset_factor;

  if (units)
    *units = priv->polygon_offset_units;

  return priv->polygon_offset;
}

void
gthree_material_set_polygon_offset (GthreeMaterial       *material,
                                    gboolean              polygon_offset,
                                    float                 factor,
                                    float                 units)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->polygon_offset = polygon_offset;
  priv->polygon_offset_factor = factor;
  priv->polygon_offset_units = units;

  material->needs_update = TRUE;
}

gboolean
gthree_material_get_depth_test (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->depth_test;
}

void
gthree_material_set_depth_test (GthreeMaterial       *material,
                                gboolean              depth_test)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->depth_test = depth_test;

  material->needs_update = TRUE;
}

gboolean
gthree_material_get_depth_write (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->depth_write;
}

void
gthree_material_set_depth_write (GthreeMaterial       *material,
                                 gboolean              depth_write)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->depth_write = depth_write;

  material->needs_update = TRUE;
}


GthreeSide
gthree_material_get_side (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->side;
}

void
gthree_material_set_side (GthreeMaterial *material,
                          GthreeSide side)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->side = side;

  material->needs_update = TRUE;
}

GthreeShader *
gthree_material_get_shader (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (material->shader == NULL)
    {
      if (class->get_shader)
	material->shader = class->get_shader (material);
      else
	material->shader = gthree_clone_shader_from_library ("basic");
    }

  return material->shader;
}
