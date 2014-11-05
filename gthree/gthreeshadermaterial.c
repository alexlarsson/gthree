#include <math.h>
#include <epoxy/gl.h>

#include "gthreeshadermaterial.h"

typedef struct {
  GthreeShader *shader;

  GthreeShadingType shading_type;
  GthreeColorType vertex_colors;

  gboolean use_lights;
  gboolean skinning;
  gboolean morphTargets;
  gboolean morphNormals;
  gboolean fog;
} GthreeShaderMaterialPrivate;

static GQuark q_color;
static GQuark q_uv;
static GQuark q_uv2;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeShaderMaterial, gthree_shader_material, GTHREE_TYPE_MATERIAL);

GthreeShaderMaterial *
gthree_shader_material_new (GthreeShader *shader)
{
  GthreeShaderMaterial *material;
  GthreeShaderMaterialPrivate *priv;

  material = g_object_new (gthree_shader_material_get_type (), NULL);
  priv = gthree_shader_material_get_instance_private (GTHREE_SHADER_MATERIAL (material));

  priv->shader = g_object_ref (shader);

  return material;
}

static void
gthree_shader_material_init (GthreeShaderMaterial *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  priv->vertex_colors = GTHREE_COLOR_NONE;
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
                                         GthreeCamera   *camera)
{
  GTHREE_MATERIAL_CLASS (gthree_shader_material_parent_class)->set_uniforms (material, uniforms, camera);
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
gthree_shader_material_needs_uv (GthreeMaterial *material)
{
  return TRUE;
}

static GthreeShadingType
gthree_shader_material_needs_normals (GthreeMaterial *material)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (material);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  if (priv->shading_type == GTHREE_SHADING_SMOOTH)
    return GTHREE_SHADING_SMOOTH;

  return GTHREE_SHADING_FLAT;
}

static gboolean
gthree_shader_material_needs_camera_pos (GthreeMaterial *material)
{
  return TRUE;
}

static GthreeColorType
gthree_shader_material_needs_colors  (GthreeMaterial *material)
{
  GthreeShaderMaterial *shader = GTHREE_SHADER_MATERIAL (material);
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->vertex_colors;
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
gthree_shader_material_class_init (GthreeShaderMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_shader_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_shader_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->load_default_attribute = gthree_shader_material_real_load_default_attribute;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_shader_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_shader_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_lights = gthree_shader_material_needs_lights;
  GTHREE_MATERIAL_CLASS(klass)->needs_view_matrix = gthree_shader_material_needs_view_matrix;
  GTHREE_MATERIAL_CLASS(klass)->needs_uv = gthree_shader_material_needs_uv;
  GTHREE_MATERIAL_CLASS(klass)->needs_normals = gthree_shader_material_needs_normals;
  GTHREE_MATERIAL_CLASS(klass)->needs_camera_pos = gthree_shader_material_needs_camera_pos;
  GTHREE_MATERIAL_CLASS(klass)->needs_colors = gthree_shader_material_needs_colors;

#define INIT_QUARK(name) q_##name = g_quark_from_static_string (#name)
  INIT_QUARK(color);
  INIT_QUARK(uv);
  INIT_QUARK(uv2);
}

GthreeShadingType
gthree_shader_material_get_shading_type    (GthreeShaderMaterial     *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->shading_type;
}

void
gthree_shader_material_set_shading_type    (GthreeShaderMaterial     *shader,
                                            GthreeShadingType         shading_type)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  priv->shading_type = shading_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (shader), TRUE);
}

void
gthree_shader_material_set_vertex_colors   (GthreeShaderMaterial     *shader,
                                            GthreeColorType           color_type)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  priv->vertex_colors = color_type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (shader), TRUE);
}

GthreeColorType
gthree_shader_material_get_vertex_colors   (GthreeShaderMaterial     *shader)
{
  GthreeShaderMaterialPrivate *priv = gthree_shader_material_get_instance_private (shader);

  return priv->vertex_colors;
}
