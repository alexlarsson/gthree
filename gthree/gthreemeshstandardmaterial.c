#include <math.h>
#include <epoxy/gl.h>

#include "gthreemeshstandardmaterial.h"
#include "gthreecubetexture.h"
#include "gthreetypebuiltins.h"

typedef struct {
  GdkRGBA color;

  GdkRGBA emissive;
  float emissive_intensity;
  GthreeTexture *emissive_map;

  float roughness;
  float metalness;

  GthreeTexture *map;

  GthreeTexture *light_map;
  float light_map_intensity;

  GthreeTexture *ao_map;
  float ao_map_intensity;

  GthreeTexture *bump_map;
  float bump_scale;

  GthreeTexture *normal_map;
  GthreeNormalMapType normal_map_type;
  graphene_vec2_t normal_scale;

  GthreeTexture *displacement_map;
  float displacement_scale;
  float displacement_bias;

  GthreeTexture *roughness_map;

  GthreeTexture *metalness_map;

  GthreeTexture *alpha_map;

  GthreeTexture *env_map;
  float env_map_intensity;
  float refraction_ratio;
} GthreeMeshStandardMaterialPrivate;


enum {
  PROP_0,

  PROP_COLOR,
  PROP_EMISSIVE_COLOR,
  PROP_EMISSIVE_INTENSITY,
  PROP_EMISSIVE_MAP,
  PROP_ROUGHNESS,
  PROP_METALNESS,
  PROP_MAP,
  PROP_LIGHT_MAP,
  PROP_LIGHT_MAP_INTENSITY,
  PROP_AO_MAP,
  PROP_AO_MAP_INTENSITY,
  PROP_BUMP_MAP,
  PROP_BUMP_SCALE,
  PROP_NORMAL_MAP,
  PROP_NORMAL_MAP_TYPE,
  //PROP_NORMAL_SCALE,
  PROP_DISPLACEMENT_MAP,
  PROP_DISPLACEMENT_SCALE,
  PROP_DISPLACEMENT_BIAS,
  PROP_ROUGHNESS_MAP,
  PROP_METALNESS_MAP,
  PROP_ALPHA_MAP,
  PROP_ENV_MAP,
  PROP_ENV_MAP_INTENSITY,
  PROP_REFRACTION_RATIO,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshStandardMaterial, gthree_mesh_standard_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshStandardMaterial *
gthree_mesh_standard_material_new ()
{
  GthreeMeshStandardMaterial *material;

  material = g_object_new (gthree_mesh_standard_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_standard_material_init (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;

  priv->emissive_intensity = 1;
  priv->roughness = 0.5;
  priv->metalness = 0.5;

  priv->light_map_intensity = 1.0;
  priv->ao_map_intensity = 1.0;
  priv->bump_scale = 1.0;
  priv->normal_map_type = GTHREE_NORMAL_MAP_TYPE_TANGENT_SPACE;
  graphene_vec2_init (&priv->normal_scale, 1.0, 1.0);
  priv->displacement_scale = 1.0;
  priv->displacement_bias = 0;
  priv->env_map_intensity = 1.0;
  priv->refraction_ratio = 0.98;
}

static void
gthree_mesh_standard_material_finalize (GObject *obj)
{
  GthreeMeshStandardMaterial *standard = GTHREE_STANDARD_MATERIAL (obj);
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (priv->map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->map));
      g_clear_object (&priv->map);
    }

  if (priv->env_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->env_map));
      g_clear_object (&priv->env_map);
    }

  if (priv->light_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->light_map));
      g_clear_object (&priv->light_map);
    }

  if (priv->ao_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->ao_map));
      g_clear_object (&priv->ao_map);
    }

  if (priv->bump_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->bump_map));
      g_clear_object (&priv->bump_map);
    }

  if (priv->normal_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->normal_map));
      g_clear_object (&priv->normal_map);
    }

  if (priv->displacement_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->displacement_map));
      g_clear_object (&priv->displacement_map);
    }

  if (priv->roughness_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->roughness_map));
      g_clear_object (&priv->roughness_map);
    }

  if (priv->metalness_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->metalness_map));
      g_clear_object (&priv->metalness_map);
    }

  if (priv->alpha_map)
    {
      gthree_resource_unuse (GTHREE_RESOURCE (priv->alpha_map));
      g_clear_object (&priv->alpha_map);
    }

  G_OBJECT_CLASS (gthree_mesh_standard_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_standard_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("standard");
}

static void
gthree_mesh_standard_material_real_set_params (GthreeMaterial *material,
                                               GthreeProgramParameters *params)
{
  GthreeMeshStandardMaterial *standard = GTHREE_STANDARD_MATERIAL (material);
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  params->map = priv->map != NULL;
  params->env_map = priv->env_map != NULL;
  if (params->env_map)
    params->env_map_mode = gthree_texture_get_mapping (priv->env_map);

  params->light_map = priv->light_map != NULL;
  params->ao_map = priv->ao_map != NULL;
  params->emissive_map = priv->emissive_map != NULL;
  params->bump_map = priv->bump_map != NULL;
  params->normal_map = priv->normal_map != NULL;
  params->object_space_normal_map = priv->normal_map_type == GTHREE_NORMAL_MAP_TYPE_OBJECT_SPACE;
  params->displacement_map = priv->displacement_map != NULL;
  params->roughness_map = priv->roughness_map != NULL;
  params->metalness_map = priv->metalness_map != NULL;
  params->alpha_map = priv->alpha_map != NULL;

  GTHREE_MATERIAL_CLASS (gthree_mesh_standard_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_standard_material_real_set_uniforms (GthreeMaterial *material,
                                                 GthreeUniforms *uniforms,
                                                 GthreeCamera   *camera)
{
  GthreeMeshStandardMaterial *standard = GTHREE_STANDARD_MATERIAL (material);
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_standard_material_parent_class)->set_uniforms (material, uniforms, camera);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->emissive);

  uni = gthree_uniforms_lookup_from_string (uniforms, "map");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->map);

  uni = gthree_uniforms_lookup_from_string (uniforms, "alphaMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->alpha_map);

  if (priv->env_map)
    {
      int max_mip_level;

      uni = gthree_uniforms_lookup_from_string (uniforms, "envMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->env_map);

      uni = gthree_uniforms_lookup_from_string (uniforms, "flipEnvMap");
      if (uni != NULL)
        gthree_uniform_set_float (uni, GTHREE_IS_CUBE_TEXTURE (priv->env_map) ? -1 : 1);

      uni = gthree_uniforms_lookup_from_string (uniforms, "refractionRatio");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->refraction_ratio);

      uni = gthree_uniforms_lookup_from_string (uniforms, "envMapIntensity");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->env_map_intensity);

      max_mip_level = gthree_texture_get_max_mip_level (priv->env_map);
      uni = gthree_uniforms_lookup_from_string (uniforms, "maxMipLevel");
      if (uni != NULL)
        gthree_uniform_set_int (uni, max_mip_level);
    }

  if (priv->light_map)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "lightMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->light_map);
      uni = gthree_uniforms_lookup_from_string (uniforms, "lightMapIntensity");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->light_map_intensity);
    }

  if (priv->ao_map)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "aoMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->ao_map);
      uni = gthree_uniforms_lookup_from_string (uniforms, "aoMapIntensity");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->ao_map_intensity);
    }

  // TODO: Apply uv scale from first material that has one

  uni = gthree_uniforms_lookup_from_string (uniforms, "roughness");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->roughness);

  uni = gthree_uniforms_lookup_from_string (uniforms, "metalness");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->metalness);

  uni = gthree_uniforms_lookup_from_string (uniforms, "roughnessMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->roughness_map);

  uni = gthree_uniforms_lookup_from_string (uniforms, "metalnessMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->metalness_map);

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissiveMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->emissive_map);

  if (priv->bump_map)
    {
      float bump_scale;

      uni = gthree_uniforms_lookup_from_string (uniforms, "bumpMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->bump_map);

      bump_scale = priv->bump_scale;
      if (gthree_material_get_side (GTHREE_MATERIAL (material)) == GTHREE_SIDE_BACK)
        bump_scale *= -1;
      uni = gthree_uniforms_lookup_from_string (uniforms, "bumpScale");
      if (uni != NULL)
        gthree_uniform_set_float (uni, bump_scale);
    }

  if (priv->normal_map)
    {
      float sign = 1.0;
      graphene_vec2_t normal_scale;

      uni = gthree_uniforms_lookup_from_string (uniforms, "normalMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->normal_map);

      if (gthree_material_get_side (GTHREE_MATERIAL (material)) == GTHREE_SIDE_BACK)
        sign = -1;

      graphene_vec2_scale (&priv->normal_scale, sign, &normal_scale);

      gthree_uniforms_set_vec2 (uniforms, "normalScale", &normal_scale);
    }

  if (priv->displacement_map)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "displacementMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->displacement_map);

      uni = gthree_uniforms_lookup_from_string (uniforms, "displacementScale");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->displacement_scale);
      uni = gthree_uniforms_lookup_from_string (uniforms, "displacementBias");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->displacement_bias);
    }
}

static gboolean
gthree_mesh_standard_material_needs_camera_pos (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_standard_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_standard_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}

static void
gthree_mesh_standard_material_set_property (GObject *obj,
                                            guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec)
{
  GthreeMeshStandardMaterial *standard = GTHREE_STANDARD_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_mesh_standard_material_set_color (standard, g_value_get_boxed (value));
      break;

    case PROP_EMISSIVE_COLOR:
      gthree_mesh_standard_material_set_emissive_color (standard, g_value_get_boxed (value));
      break;

    case PROP_EMISSIVE_INTENSITY:
      gthree_mesh_standard_material_set_emissive_intensity (standard, g_value_get_float (value));
      break;

    case PROP_EMISSIVE_MAP:
      gthree_mesh_standard_material_set_emissive_map (standard, g_value_get_object (value));
      break;

    case PROP_ROUGHNESS:
      gthree_mesh_standard_material_set_roughness (standard, g_value_get_float (value));
      break;

    case PROP_METALNESS:
      gthree_mesh_standard_material_set_metalness (standard, g_value_get_float (value));
      break;

    case PROP_MAP:
      gthree_mesh_standard_material_set_map (standard, g_value_get_object (value));
      break;

    case PROP_LIGHT_MAP:
      gthree_mesh_standard_material_set_light_map (standard, g_value_get_object (value));
      break;

    case PROP_LIGHT_MAP_INTENSITY:
      gthree_mesh_standard_material_set_light_map_intensity (standard, g_value_get_float (value));
      break;

    case PROP_AO_MAP:
      gthree_mesh_standard_material_set_ao_map (standard, g_value_get_object (value));
      break;

    case PROP_AO_MAP_INTENSITY:
      gthree_mesh_standard_material_set_ao_map_intensity (standard, g_value_get_float (value));
      break;

    case PROP_BUMP_MAP:
      gthree_mesh_standard_material_set_bump_map (standard, g_value_get_object (value));
      break;

    case PROP_BUMP_SCALE:
      gthree_mesh_standard_material_set_bump_scale (standard, g_value_get_float (value));
      break;

    case PROP_NORMAL_MAP:
      gthree_mesh_standard_material_set_normal_map (standard, g_value_get_object (value));
      break;

    case PROP_NORMAL_MAP_TYPE:
      gthree_mesh_standard_material_set_normal_map_type (standard, g_value_get_enum (value));
      break;

      /* Need to handle vec2 gobject properties..
    case PROP_NORMAL_SCALE:
      g_assert_not_reached ();
      break;
      */

    case PROP_DISPLACEMENT_MAP:
      gthree_mesh_standard_material_set_displacement_map (standard, g_value_get_object (value));
      break;

    case PROP_DISPLACEMENT_SCALE:
      gthree_mesh_standard_material_set_displacement_scale (standard, g_value_get_float (value));
      break;

    case PROP_DISPLACEMENT_BIAS:
      gthree_mesh_standard_material_set_displacement_bias (standard, g_value_get_float (value));
      break;

    case PROP_ROUGHNESS_MAP:
      gthree_mesh_standard_material_set_roughness_map (standard, g_value_get_object (value));
      break;

    case PROP_METALNESS_MAP:
      gthree_mesh_standard_material_set_metalness_map (standard, g_value_get_object (value));
      break;

    case PROP_ALPHA_MAP:
      gthree_mesh_standard_material_set_alpha_map (standard, g_value_get_object (value));
      break;

    case PROP_ENV_MAP:
      gthree_mesh_standard_material_set_env_map (standard, g_value_get_object (value));
      break;

    case PROP_ENV_MAP_INTENSITY:
      gthree_mesh_standard_material_set_env_map_intensity (standard, g_value_get_float (value));
      break;

    case PROP_REFRACTION_RATIO:
      gthree_mesh_standard_material_set_refraction_ratio (standard, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_standard_material_get_property (GObject *obj,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMeshStandardMaterial *standard = GTHREE_STANDARD_MATERIAL (obj);
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_EMISSIVE_COLOR:
      g_value_set_boxed (value, &priv->emissive);
      break;

    case PROP_EMISSIVE_MAP:
      g_value_set_object (value, priv->emissive_map);
      break;

    case PROP_EMISSIVE_INTENSITY:
      g_value_set_float (value, priv->emissive_intensity);
      break;

    case PROP_ROUGHNESS:
      g_value_set_float (value, priv->roughness);
      break;

    case PROP_METALNESS:
      g_value_set_float (value, priv->metalness);
      break;

    case PROP_MAP:
      g_value_set_object (value, priv->map);
      break;

    case PROP_LIGHT_MAP:
      g_value_set_object (value, priv->light_map);
      break;

    case PROP_LIGHT_MAP_INTENSITY:
      g_value_set_float (value, priv->light_map_intensity);
      break;

    case PROP_AO_MAP:
      g_value_set_object (value, priv->ao_map);
      break;

    case PROP_AO_MAP_INTENSITY:
      g_value_set_float (value, priv->ao_map_intensity);
      break;

    case PROP_BUMP_MAP:
      g_value_set_object (value, priv->bump_map);
      break;

    case PROP_BUMP_SCALE:
      g_value_set_float (value, priv->bump_scale);
      break;

    case PROP_NORMAL_MAP:
      g_value_set_object (value, priv->normal_map);
      break;

    case PROP_NORMAL_MAP_TYPE:
      g_value_set_enum (value, priv->normal_map_type);
      break;

      /* Need to handle vec2 gobject properties..
    case PROP_NORMAL_SCALE:
      g_assert_not_reached (); 
      break;
      */

    case PROP_DISPLACEMENT_MAP:
      g_value_set_object (value, priv->displacement_map);
      break;

    case PROP_DISPLACEMENT_SCALE:
      g_value_set_float (value, priv->displacement_scale);
      break;

    case PROP_DISPLACEMENT_BIAS:
      g_value_set_float (value, priv->displacement_bias);
      break;

    case PROP_ROUGHNESS_MAP:
      g_value_set_object (value, priv->roughness_map);
      break;

    case PROP_METALNESS_MAP:
      g_value_set_object (value, priv->metalness_map);
      break;

    case PROP_ALPHA_MAP:
      g_value_set_object (value, priv->alpha_map);
      break;

    case PROP_ENV_MAP:
      g_value_set_object (value, priv->env_map);
      break;

    case PROP_ENV_MAP_INTENSITY:
      g_value_set_float (value, priv->env_map_intensity);
      break;

    case PROP_REFRACTION_RATIO:
      g_value_set_float (value, priv->refraction_ratio);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}


static void
gthree_mesh_standard_material_class_init (GthreeMeshStandardMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->finalize = gthree_mesh_standard_material_finalize;
  gobject_class->set_property = gthree_mesh_standard_material_set_property;
  gobject_class->get_property = gthree_mesh_standard_material_get_property;

  material_class->get_shader = gthree_mesh_standard_material_real_get_shader;
  material_class->set_params = gthree_mesh_standard_material_real_set_params;
  material_class->set_uniforms = gthree_mesh_standard_material_real_set_uniforms;
  material_class->needs_camera_pos = gthree_mesh_standard_material_needs_camera_pos;
  material_class->needs_view_matrix = gthree_mesh_standard_material_needs_view_matrix;
  material_class->needs_lights = gthree_mesh_standard_material_needs_lights;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_COLOR] =
    g_param_spec_boxed ("emissive-color", "Emissive Color", "Emissive",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_INTENSITY] =
    g_param_spec_float ("emissive-intensity", "Emissive intensity", "Emissive intensity",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_MAP] =
    g_param_spec_object ("emissive-map", "Emissive map", "Emissive map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ROUGHNESS] =
    g_param_spec_float ("roughness", "Roughness", "Roughness",
                        0.f, 1.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_METALNESS] =
    g_param_spec_float ("metalness", "Metalness", "Metalness",
                        0.f, 1.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MAP] =
    g_param_spec_object ("map", "Map", "Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_LIGHT_MAP] =
    g_param_spec_object ("light-map", "Light map", "Light map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_LIGHT_MAP_INTENSITY] =
    g_param_spec_float ("light-map-intensity", "Light map intensity", "Light map intensity",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_AO_MAP] =
    g_param_spec_object ("ao-map", "Ao map", "Ao map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_AO_MAP_INTENSITY] =
    g_param_spec_float ("ao-map-intensity", "Ao map intensity", "Ao map intensity",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_BUMP_MAP] =
    g_param_spec_object ("bump-map", "Bump map", "Bump map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_BUMP_SCALE] =
    g_param_spec_float ("bump-scale", "Bump scale", "Bump scale",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_NORMAL_MAP] =
    g_param_spec_object ("normal-map", "Normal map", "Normal map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_NORMAL_MAP_TYPE] =
    g_param_spec_enum ("normal-map-type", "Normal map type", "Normal map type",
                       GTHREE_TYPE_NORMAL_MAP_TYPE,
                       GTHREE_NORMAL_MAP_TYPE_TANGENT_SPACE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  /* Need to handle vec2 gobject properties..
  obj_props[PROP_NORMAL_SCALE] =
    g_param_spec_float ("normal-scale", "Normal scale", "Normal scale",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  */
  obj_props[PROP_DISPLACEMENT_MAP] =
    g_param_spec_object ("displaement-map", "Displaement map", "Displacement map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_DISPLACEMENT_SCALE] =
    g_param_spec_float ("displacement-scale", "Displacement scale", "Displacement scale",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_DISPLACEMENT_BIAS] =
    g_param_spec_float ("displacement-bias", "Displacement bias", "Displacement bias",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ROUGHNESS_MAP] =
    g_param_spec_object ("roughness-map", "Roughness map", "Roughness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_METALNESS_MAP] =
    g_param_spec_object ("metalness-map", "Metalness map", "Metalness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ALPHA_MAP] =
    g_param_spec_object ("alpha-map", "Alpha map", "Alpha map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ENV_MAP] =
    g_param_spec_object ("env-map", "Env Map", "Env Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ENV_MAP_INTENSITY] =
    g_param_spec_float ("env-map-intensity", "Env map intensity", "Env map intensity",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_REFRACTION_RATIO] =
    g_param_spec_float ("refraction-ratio", "Refraction Ratio", "Refraction Ratio",
                        0.f, 1.f, 0.98f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

const GdkRGBA *
gthree_mesh_standard_material_get_emissive_color (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return &priv->emissive;
}

void
gthree_mesh_standard_material_set_emissive_color (GthreeMeshStandardMaterial *standard,
                                               const GdkRGBA *color)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->emissive = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);
}


GthreeTexture *
gthree_mesh_standard_material_get_emissive_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->emissive_map;
}

void
gthree_mesh_standard_material_set_emissive_map (GthreeMeshStandardMaterial *standard,
                                                GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->emissive_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->emissive_map));

  if (g_set_object (&priv->emissive_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_EMISSIVE_MAP]);
    }
}

float
gthree_mesh_standard_material_get_emissive_intensity (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->emissive_intensity;
}

void
gthree_mesh_standard_material_set_emissive_intensity (GthreeMeshStandardMaterial *standard,
                                                      float                value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->emissive_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_EMISSIVE_INTENSITY]);
}

float
gthree_mesh_standard_material_get_refraction_ratio (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->refraction_ratio;
}

void
gthree_mesh_standard_material_set_refraction_ratio (GthreeMeshStandardMaterial *standard,
                                                   float                ratio)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->refraction_ratio = ratio;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_REFRACTION_RATIO]);
}


const GdkRGBA *
gthree_mesh_standard_material_get_color (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return &priv->color;
}

void
gthree_mesh_standard_material_set_color (GthreeMeshStandardMaterial *standard,
                                         const GdkRGBA *color)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_COLOR]);
}

void
gthree_mesh_standard_material_set_map (GthreeMeshStandardMaterial *standard,
                                       GthreeTexture *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->map));

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_MAP]);
    }
}

GthreeTexture *
gthree_mesh_standard_material_get_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->map;
}

void
gthree_mesh_standard_material_set_env_map (GthreeMeshStandardMaterial *standard,
                                           GthreeTexture *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->env_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->env_map));

  if (g_set_object (&priv->env_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_ENV_MAP]);
    }
}

GthreeTexture *
gthree_mesh_standard_material_get_env_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->env_map;
}

float
gthree_mesh_standard_material_get_roughness  (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->roughness;
}

void
gthree_mesh_standard_material_set_roughness (GthreeMeshStandardMaterial *standard,
                                             float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->roughness = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_ROUGHNESS]);
}

float
gthree_mesh_standard_material_get_metalness  (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->metalness;
}

void
gthree_mesh_standard_material_set_metalness (GthreeMeshStandardMaterial *standard,
                                             float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);;

  priv->metalness = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_METALNESS]);
}

GthreeTexture *
gthree_mesh_standard_material_get_light_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->light_map;
}

void
gthree_mesh_standard_material_set_light_map (GthreeMeshStandardMaterial *standard,
                                             GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);


  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->light_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->light_map));

  if (g_set_object (&priv->light_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_LIGHT_MAP]);
    }
}

float
gthree_mesh_standard_material_get_light_map_intensity (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->light_map_intensity;
}

void
gthree_mesh_standard_material_set_light_map_intensity (GthreeMeshStandardMaterial *standard,
                                                       float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->light_map_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_LIGHT_MAP_INTENSITY]);
}

GthreeTexture *
gthree_mesh_standard_material_get_ao_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->ao_map;
}

void
gthree_mesh_standard_material_set_ao_map (GthreeMeshStandardMaterial *standard,
                                          GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->ao_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->ao_map));

  if (g_set_object (&priv->ao_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_AO_MAP]);
    }
}

float
gthree_mesh_standard_material_get_ao_map_intensity (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->ao_map_intensity;
}

void
gthree_mesh_standard_material_set_ao_map_intensity (GthreeMeshStandardMaterial *standard,
                                                    float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->ao_map_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_AO_MAP_INTENSITY]);

}

GthreeTexture *
gthree_mesh_standard_material_get_bump_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->bump_map;
}

void
gthree_mesh_standard_material_set_bump_map (GthreeMeshStandardMaterial *standard,
                                            GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->bump_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->bump_map));

  if (g_set_object (&priv->bump_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_BUMP_MAP]);
    }
}

float
gthree_mesh_standard_material_get_bump_scale (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->bump_scale;
}

void
gthree_mesh_standard_material_set_bump_scale (GthreeMeshStandardMaterial *standard,
                                              float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->bump_scale = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_BUMP_SCALE]);
}

GthreeTexture *
gthree_mesh_standard_material_get_normal_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->normal_map;
}

void
gthree_mesh_standard_material_set_normal_map (GthreeMeshStandardMaterial *standard,
                                              GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

 if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->normal_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->normal_map));

  if (g_set_object (&priv->normal_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_NORMAL_MAP]);
    }
}

GthreeNormalMapType
gthree_mesh_standard_material_get_normal_map_type (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->normal_map_type;
}

void
gthree_mesh_standard_material_set_normal_map_type (GthreeMeshStandardMaterial *standard,
                                                   GthreeNormalMapType         type)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->normal_map_type = type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_NORMAL_MAP_TYPE]);
}

const graphene_vec2_t *
gthree_mesh_standard_material_get_normal_map_scale (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return &priv->normal_scale;
}

void
gthree_mesh_standard_material_set_normal_map_scale (GthreeMeshStandardMaterial *standard,
                                                    graphene_vec2_t            *scale)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->normal_scale = *scale;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  //g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_NORM]);
}

GthreeTexture *
gthree_mesh_standard_material_get_displacement_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->displacement_map;
}

void
gthree_mesh_standard_material_set_displacement_map (GthreeMeshStandardMaterial *standard,
                                                    GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->displacement_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->displacement_map));

  if (g_set_object (&priv->displacement_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_DISPLACEMENT_MAP]);
    }
}

float
gthree_mesh_standard_material_get_displacement_scale (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->displacement_scale;
}

void
gthree_mesh_standard_material_set_displacement_scale  (GthreeMeshStandardMaterial *standard,
                                                       float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->displacement_scale = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_DISPLACEMENT_SCALE]);
}

float
gthree_mesh_standard_material_get_displacement_bias (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->displacement_bias;
}

void
gthree_mesh_standard_material_set_displacement_bias (GthreeMeshStandardMaterial *standard,
                                                     float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);


  priv->displacement_bias = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_DISPLACEMENT_BIAS]);
}

GthreeTexture *
gthree_mesh_standard_material_get_roughness_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->roughness_map;
}

void
gthree_mesh_standard_material_set_roughness_map (GthreeMeshStandardMaterial *standard,
                                                 GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->roughness_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->roughness_map));

  if (g_set_object (&priv->roughness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_ROUGHNESS_MAP]);
    }
}

GthreeTexture *
gthree_mesh_standard_material_get_metalness_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->metalness_map;
}

void
gthree_mesh_standard_material_set_metalness_map (GthreeMeshStandardMaterial *standard,
                                                 GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->metalness_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->metalness_map));

  if (g_set_object (&priv->metalness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_METALNESS_MAP]);
    }
}

GthreeTexture *
gthree_mesh_standard_material_get_alpha_map (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->alpha_map;
}

void
gthree_mesh_standard_material_set_alpha_map (GthreeMeshStandardMaterial *standard,
                                             GthreeTexture              *texture)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  if (texture)
    gthree_resource_use (GTHREE_RESOURCE (texture));
  if (priv->alpha_map)
    gthree_resource_unuse (GTHREE_RESOURCE (priv->alpha_map));

  if (g_set_object (&priv->alpha_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

      g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_ALPHA_MAP]);
    }
}

float
gthree_mesh_standard_material_get_env_map_intensity (GthreeMeshStandardMaterial *standard)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  return priv->env_map_intensity;
}

void
gthree_mesh_standard_material_set_env_map_intensity (GthreeMeshStandardMaterial *standard,
                                                     float                       value)
{
  GthreeMeshStandardMaterialPrivate *priv = gthree_mesh_standard_material_get_instance_private (standard);

  priv->env_map_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (standard), TRUE);

  g_object_notify_by_pspec (G_OBJECT (standard), obj_props[PROP_ENV_MAP_INTENSITY]);
}
