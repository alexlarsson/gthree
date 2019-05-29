#include <math.h>
#include <epoxy/gl.h>

#include "gthreematerial.h"
#include "gthreeprivate.h"

typedef struct {
  gboolean transparent;
  float opacity;
  gboolean visible;
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
  gboolean vertex_colors;

  GthreeShader *shader;
  gboolean needs_update;

  /* modified by the renderer to track state */
  GthreeMaterialProperties properties;
} GthreeMaterialPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMaterial, gthree_material, G_TYPE_OBJECT);

enum {
  PROP_0,

  PROP_VERTEX_COLORS,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

static void
gthree_material_init (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->needs_update = TRUE;

  priv->visible = TRUE;
  priv->transparent = FALSE;
  priv->opacity = 1.0;
  priv->blend_mode = GTHREE_BLEND_NORMAL;
  priv->blend_equation = GL_FUNC_ADD;
  priv->blend_src_factor = GL_SRC_ALPHA;
  priv->blend_dst_factor = GL_ONE_MINUS_SRC_ALPHA;
  priv->depth_test = TRUE;
  priv->depth_write = TRUE;
  priv->vertex_colors = FALSE;

  priv->polygon_offset = FALSE;
  priv->polygon_offset_factor = 0;
  priv->polygon_offset_units = 0;
  priv->alpha_test = 0;
  priv->side = GTHREE_SIDE_FRONT;

  priv->properties.light_hash.num_point = -1; // Ensure we fill it once
}

static void
gthree_material_finalize (GObject *obj)
{
  GthreeMaterial *material = GTHREE_MATERIAL (obj);
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  g_clear_object (&priv->properties.program);

  G_OBJECT_CLASS (gthree_material_parent_class)->finalize (obj);
}

static void
gthree_material_set_property (GObject *obj,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
  GthreeMaterial *material = GTHREE_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_VERTEX_COLORS:
      gthree_material_set_vertex_colors (material, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_material_get_property (GObject *obj,
                              guint prop_id,
                              GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMaterial *material = GTHREE_MATERIAL (obj);
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  switch (prop_id)
    {
    case PROP_VERTEX_COLORS:
      g_value_set_boolean (value, priv->vertex_colors);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
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
  params->alpha_test = priv->alpha_test;
  params->vertex_colors = priv->vertex_colors;
}

void
gthree_material_set_uniforms (GthreeMaterial *material,
                              GthreeUniforms *uniforms,
                              GthreeCamera   *camera)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  return class->set_uniforms (material, uniforms, camera);
}

static void
gthree_material_real_set_uniforms (GthreeMaterial *material,
                                   GthreeUniforms *uniforms,
                                   GthreeCamera   *camera)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);
  GthreeUniform *uni;

  uni = gthree_uniforms_lookup_from_string (uniforms, "opacity");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->opacity);
}

gboolean
gthree_material_needs_camera_pos (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_camera_pos)
    return class->needs_camera_pos (material);

  return FALSE;
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
gthree_material_needs_lights (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->needs_lights)
    return class->needs_lights (material);

  return FALSE;
}

static void
gthree_material_class_init (GthreeMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_material_set_property;
  gobject_class->get_property = gthree_material_get_property;
  gobject_class->finalize = gthree_material_finalize;

  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_material_real_set_uniforms;

  obj_props[PROP_VERTEX_COLORS] =
    g_param_spec_boolean ("vertex-colors", "Vertex Colors", "Vertex Colors",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
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

  priv->needs_update = TRUE;
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
  priv->needs_update = TRUE;
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
  priv->needs_update = TRUE;
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

  priv->needs_update = TRUE;
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

gboolean
gthree_material_get_needs_update (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->needs_update;
}

void
gthree_material_set_needs_update (GthreeMaterial *material, gboolean needs_update)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  priv->needs_update = needs_update;
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

  priv->needs_update = TRUE;
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

  priv->needs_update = TRUE;
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

  priv->needs_update = TRUE;
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

  priv->needs_update = TRUE;
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

  priv->needs_update = TRUE;
}

void
gthree_material_set_vertex_colors (GthreeMaterial *material,
                                   gboolean vertex_colors)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  vertex_colors = !!vertex_colors;
  if (priv->vertex_colors == vertex_colors)
    return;

  priv->vertex_colors = vertex_colors;

  gthree_material_set_needs_update (material, TRUE);

  g_object_notify_by_pspec (G_OBJECT (material), obj_props[PROP_VERTEX_COLORS]);
}

gboolean
gthree_material_get_vertex_colors (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->vertex_colors;
}

GthreeShader *
gthree_material_get_shader (GthreeMaterial *material)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  if (priv->shader == NULL)
    {
      if (class->get_shader)
        priv->shader = class->get_shader (material);
      else
        priv->shader = gthree_clone_shader_from_library ("basic");
    }

  return priv->shader;
}

void
gthree_material_load_default_attribute (GthreeMaterial       *material,
                                        int                   attribute_location,
                                        GQuark                attribute)
{
  GthreeMaterialClass *class = GTHREE_MATERIAL_GET_CLASS(material);

  if (class->load_default_attribute)
    class->load_default_attribute (material, attribute_location, attribute);
}

GthreeMaterialProperties *
gthree_material_get_properties (GthreeMaterial  *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);
  return &priv->properties;
}
