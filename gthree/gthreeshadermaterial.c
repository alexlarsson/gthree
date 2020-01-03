#include <math.h>
#include <epoxy/gl.h>

#include "gthreeshadermaterial.h"
#include "gthreetypebuiltins.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeShader *shader;

  GthreeShadingType shading_type;
  gboolean vertex_colors;

  gboolean use_lights;
  gboolean skinning;
  gboolean morphTargets;
  gboolean morphNormals;
  gboolean fog;
} GthreeShaderMaterialPrivate;

enum {
  PROP_0,

  PROP_SHADER,
  PROP_VERTEX_COLORS,
  PROP_SHADING_TYPE,
  PROP_USE_LIGHTS,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

static GQuark q_color;
static GQuark q_uv;
static GQuark q_uv2;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeShaderMaterial, gthree_shader_material, GTHREE_TYPE_MESH_MATERIAL)

GthreeShaderMaterial *
gthree_shader_material_new (GthreeShader *shader)
{
  return g_object_new (gthree_shader_material_get_type (),
                       "shader", shader,
                       NULL);
}

static void
gthree_shader_material_init (GthreeShaderMaterial *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  priv->vertex_colors = FALSE;
  priv->shading_type = GTHREE_SHADING_SMOOTH;
  priv->use_lights = FALSE;
}

static void
gthree_shader_material_finalize (GObject *obj)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (obj);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  g_clear_object (&priv->shader);

  G_OBJECT_CLASS (gthree_shader_material_parent_class)->finalize (obj);
}

static void
gthree_shader_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (material);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  GTHREE_MATERIAL_CLASS (gthree_shader_material_parent_class)->set_params (material, params);

  params->vertex_colors = priv->vertex_colors;
}

static void
gthree_shader_material_real_set_uniforms (GthreeMaterial *material,
                                          GthreeUniforms *uniforms,
                                          GthreeCamera   *camera,
                                          GthreeRenderer *renderer)
{
  GTHREE_MATERIAL_CLASS (gthree_shader_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);
}

static gboolean
gthree_shader_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_shader_material_needs_lights (GthreeMaterial *material)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (material);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->use_lights;
}

static gboolean
gthree_shader_material_needs_camera_pos (GthreeMaterial *material)
{
  return TRUE;
}

static void
gthree_shader_material_real_load_default_attribute (GthreeMaterial       *material,
                                                    int                   attribute_location,
                                                    GQuark                attribute)
{
  if (attribute == q_color)
    {
      float default_color[3] = {1,1,1};
      glVertexAttrib3fv (attribute_location, default_color);
    }
  else if (attribute == q_uv || attribute == q_uv2)
    {
      float default_uv[2] = {0,0};
      glVertexAttrib2fv (attribute_location, default_uv);
    }
}

GthreeShader *
gthree_shader_material_real_get_shader (GthreeMaterial *material)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (material);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->shader;
}

static void
gthree_shader_material_set_property (GObject *obj,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (obj);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  switch (prop_id)
    {
    case PROP_SHADER:
      g_set_object (&priv->shader, g_value_get_object (value));
      break;

    case PROP_SHADING_TYPE:
      gthree_shader_material_set_shading_type (shader, g_value_get_enum (value));
      break;

    case PROP_VERTEX_COLORS:
      gthree_shader_material_set_vertex_colors (shader, g_value_get_boolean (value));
      break;

    case PROP_USE_LIGHTS:
      gthree_shader_material_set_use_lights (shader, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_shader_material_get_property (GObject *obj,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (obj);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  switch (prop_id)
    {
    case PROP_SHADER:
      g_value_set_object (value, priv->shader);
      break;

    case PROP_SHADING_TYPE:
      g_value_set_enum (value, priv->shading_type);
      break;

    case PROP_VERTEX_COLORS:
      g_value_set_boolean (value, priv->vertex_colors);
      break;

    case PROP_USE_LIGHTS:
      g_value_set_boolean (value, priv->use_lights);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_shader_material_class_init (GthreeShaderMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_shader_material_set_property;
  gobject_class->get_property = gthree_shader_material_get_property;
  gobject_class->finalize = gthree_shader_material_finalize;

  material_class->apply_common_uniforms = FALSE;
  material_class->get_shader = gthree_shader_material_real_get_shader;
  material_class->load_default_attribute = gthree_shader_material_real_load_default_attribute;
  material_class->set_params = gthree_shader_material_real_set_params;
  material_class->set_uniforms = gthree_shader_material_real_set_uniforms;
  material_class->needs_lights = gthree_shader_material_needs_lights;
  material_class->needs_view_matrix = gthree_shader_material_needs_view_matrix;
  material_class->needs_camera_pos = gthree_shader_material_needs_camera_pos;

  obj_props[PROP_SHADER] =
    g_param_spec_object ("shader", "Shader", "Shader",
                         GTHREE_TYPE_SHADER,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHADING_TYPE] =
    g_param_spec_enum ("shading-type", "Shading Type", "Shading Type",
                       GTHREE_TYPE_SHADING_TYPE,
                       GTHREE_SHADING_SMOOTH,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_VERTEX_COLORS] =
    g_param_spec_boolean ("vertex-colors", "Vertex Colors", "Vertex Colors",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_USE_LIGHTS] =
    g_param_spec_boolean ("use-lights", "Use Lights", "Use Lights",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);

#define INIT_QUARK(name) q_##name = g_quark_from_static_string (#name)
  INIT_QUARK(color);
  INIT_QUARK(uv);
  INIT_QUARK(uv2);
}

GthreeShadingType
gthree_shader_material_get_shading_type (GthreeShaderMaterial *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->shading_type;
}

void
gthree_shader_material_set_shading_type (GthreeShaderMaterial *shader,
                                         GthreeShadingType     shading_type)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  if (priv->shading_type == shading_type)
    return;

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (shader));

  g_object_notify_by_pspec (G_OBJECT (shader), obj_props[PROP_SHADING_TYPE]);
}

void
gthree_shader_material_set_vertex_colors (GthreeShaderMaterial *shader,
                                          gboolean  vertex_colors)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  vertex_colors = !!vertex_colors;
  if (priv->vertex_colors == vertex_colors)
    return;

  priv->vertex_colors = vertex_colors;

  gthree_material_set_needs_update (GTHREE_MATERIAL (shader));

  g_object_notify_by_pspec (G_OBJECT (shader), obj_props[PROP_VERTEX_COLORS]);
}

gboolean
gthree_shader_material_get_vertex_colors (GthreeShaderMaterial *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->vertex_colors;
}

void
gthree_shader_material_set_use_lights (GthreeShaderMaterial *shader,
                                       gboolean use_lights)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  use_lights = !!use_lights;

  if (priv->use_lights == use_lights)
    return;

  priv->use_lights = use_lights;

  gthree_material_set_needs_update (GTHREE_MATERIAL (shader));

  g_object_notify_by_pspec (G_OBJECT (shader), obj_props[PROP_USE_LIGHTS]);
}

gboolean
gthree_shader_material_get_use_lights (GthreeShaderMaterial *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->use_lights;
}
