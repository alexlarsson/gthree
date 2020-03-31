#include <math.h>
#include <epoxy/gl.h>
#include <graphene-gobject.h>

#include "gthreemeshspecglosmaterial.h"
#include "gthreecubetexture.h"
#include "gthreeprivate.h"
#include "gthreetypebuiltins.h"

typedef struct {
  graphene_vec3_t color;

  graphene_vec3_t emissive;
  float emissive_intensity;
  GthreeTexture *emissive_map;

  float glossiness;
  graphene_vec3_t specular;

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

  GthreeTexture *glossiness_map;

  GthreeTexture *specular_map;

  GthreeTexture *alpha_map;

  GthreeTexture *env_map;
  float env_map_intensity;
  float refraction_ratio;
} GthreeMeshSpecglosMaterialPrivate;


enum {
  PROP_0,

  PROP_COLOR,
  PROP_EMISSIVE_COLOR,
  PROP_EMISSIVE_INTENSITY,
  PROP_EMISSIVE_MAP,
  PROP_GLOSSINESS,
  PROP_SPECULAR_FACTOR,
  PROP_MAP,
  PROP_LIGHT_MAP,
  PROP_LIGHT_MAP_INTENSITY,
  PROP_AO_MAP,
  PROP_AO_MAP_INTENSITY,
  PROP_BUMP_MAP,
  PROP_BUMP_SCALE,
  PROP_NORMAL_MAP,
  PROP_NORMAL_MAP_TYPE,
  PROP_NORMAL_SCALE,
  PROP_DISPLACEMENT_MAP,
  PROP_DISPLACEMENT_SCALE,
  PROP_DISPLACEMENT_BIAS,
  PROP_GLOSSINESS_MAP,
  PROP_SPECULAR_MAP,
  PROP_ALPHA_MAP,
  PROP_ENV_MAP,
  PROP_ENV_MAP_INTENSITY,
  PROP_REFRACTION_RATIO,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshSpecglosMaterial, gthree_mesh_specglos_material, GTHREE_TYPE_MESH_MATERIAL);

GthreeMeshSpecglosMaterial *
gthree_mesh_specglos_material_new ()
{
  GthreeMeshSpecglosMaterial *material;

  material = g_object_new (gthree_mesh_specglos_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_specglos_material_init (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  graphene_vec3_init (&priv->color,
                      1.0, 1.0, 1.0);
  graphene_vec3_init (&priv->emissive,
                      0.0, 0.0, 0.0);

  priv->emissive_intensity = 1;
  priv->glossiness = 0.5;
  graphene_vec3_init (&priv->specular,
                      0.06667, 0.06667, 0.06667);

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
gthree_mesh_specglos_material_finalize (GObject *obj)
{
  GthreeMeshSpecglosMaterial *specglos = GTHREE_MESH_SPECGLOS_MATERIAL (obj);
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  g_clear_object (&priv->map);
  g_clear_object (&priv->env_map);
  g_clear_object (&priv->light_map);
  g_clear_object (&priv->ao_map);
  g_clear_object (&priv->bump_map);
  g_clear_object (&priv->normal_map);
  g_clear_object (&priv->displacement_map);
  g_clear_object (&priv->glossiness_map);
  g_clear_object (&priv->specular_map);
  g_clear_object (&priv->alpha_map);

  G_OBJECT_CLASS (gthree_mesh_specglos_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_specglos_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("specglos");
}

static void
gthree_mesh_specglos_material_real_set_params (GthreeMaterial *material,
                                               GthreeProgramParameters *params)
{
  GthreeMeshSpecglosMaterial *specglos = GTHREE_MESH_SPECGLOS_MATERIAL (material);
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  params->map = priv->map != NULL;
  if (params->map)
    params->map_encoding = gthree_texture_get_encoding (priv->map);

  params->env_map = priv->env_map != NULL;
  if (params->env_map)
    {
      params->env_map_encoding = gthree_texture_get_encoding (priv->env_map);
      params->env_map_mode = gthree_texture_get_mapping (priv->env_map);
    }

  params->light_map = priv->light_map != NULL;
  params->ao_map = priv->ao_map != NULL;

  params->emissive_map = priv->emissive_map != NULL;
  if (params->emissive_map)
    params->emissive_map_encoding = gthree_texture_get_encoding (priv->emissive_map);

  params->bump_map = priv->bump_map != NULL;
  params->normal_map = priv->normal_map != NULL;
  params->object_space_normal_map = priv->normal_map_type == GTHREE_NORMAL_MAP_TYPE_OBJECT_SPACE;
  params->displacement_map = priv->displacement_map != NULL;
  params->glossiness_map = priv->glossiness_map != NULL;
  // also set USE_ROUGHNESSMAP to enable vUv
  params->roughness_map = priv->glossiness_map != NULL;
  params->specular_map = priv->specular_map != NULL;
  params->alpha_map = priv->alpha_map != NULL;

  GTHREE_MATERIAL_CLASS (gthree_mesh_specglos_material_parent_class)->set_params (material, params);
}

static void
gthree_mesh_specglos_material_real_set_uniforms (GthreeMaterial *material,
                                                 GthreeUniforms *uniforms,
                                                 GthreeCamera   *camera,
                                                 GthreeRenderer *renderer)
{
  GthreeMeshSpecglosMaterial *specglos = GTHREE_MESH_SPECGLOS_MATERIAL (material);
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_specglos_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_vec3 (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
  if (uni != NULL)
    {
      graphene_vec3_t emissive;

      graphene_vec3_scale (&priv->emissive, priv->emissive_intensity, &emissive);
      gthree_uniform_set_vec3 (uni, &emissive);
    }

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

  uni = gthree_uniforms_lookup_from_string (uniforms, "glossiness");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->glossiness);

  uni = gthree_uniforms_lookup_from_string (uniforms, "specular");
  if (uni != NULL)
    gthree_uniform_set_vec3 (uni, &priv->specular);

  uni = gthree_uniforms_lookup_from_string (uniforms, "glossinessMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->glossiness_map);

  uni = gthree_uniforms_lookup_from_string (uniforms, "specularMap");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->specular_map);

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
gthree_mesh_specglos_material_needs_camera_pos (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_specglos_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_mesh_specglos_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}

static void
gthree_mesh_specglos_material_set_property (GObject *obj,
                                            guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec)
{
  GthreeMeshSpecglosMaterial *specglos = GTHREE_MESH_SPECGLOS_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_mesh_specglos_material_set_color (specglos, g_value_get_boxed (value));
      break;

    case PROP_EMISSIVE_COLOR:
      gthree_mesh_specglos_material_set_emissive_color (specglos, g_value_get_boxed (value));
      break;

    case PROP_EMISSIVE_INTENSITY:
      gthree_mesh_specglos_material_set_emissive_intensity (specglos, g_value_get_float (value));
      break;

    case PROP_EMISSIVE_MAP:
      gthree_mesh_specglos_material_set_emissive_map (specglos, g_value_get_object (value));
      break;

    case PROP_GLOSSINESS:
      gthree_mesh_specglos_material_set_glossiness (specglos, g_value_get_float (value));
      break;

    case PROP_SPECULAR_FACTOR:
      gthree_mesh_specglos_material_set_specular_factor (specglos, g_value_get_boxed (value));
      break;

    case PROP_MAP:
      gthree_mesh_specglos_material_set_map (specglos, g_value_get_object (value));
      break;

    case PROP_LIGHT_MAP:
      gthree_mesh_specglos_material_set_light_map (specglos, g_value_get_object (value));
      break;

    case PROP_LIGHT_MAP_INTENSITY:
      gthree_mesh_specglos_material_set_light_map_intensity (specglos, g_value_get_float (value));
      break;

    case PROP_AO_MAP:
      gthree_mesh_specglos_material_set_ao_map (specglos, g_value_get_object (value));
      break;

    case PROP_AO_MAP_INTENSITY:
      gthree_mesh_specglos_material_set_ao_map_intensity (specglos, g_value_get_float (value));
      break;

    case PROP_BUMP_MAP:
      gthree_mesh_specglos_material_set_bump_map (specglos, g_value_get_object (value));
      break;

    case PROP_BUMP_SCALE:
      gthree_mesh_specglos_material_set_bump_scale (specglos, g_value_get_float (value));
      break;

    case PROP_NORMAL_MAP:
      gthree_mesh_specglos_material_set_normal_map (specglos, g_value_get_object (value));
      break;

    case PROP_NORMAL_MAP_TYPE:
      gthree_mesh_specglos_material_set_normal_map_type (specglos, g_value_get_enum (value));
      break;

    case PROP_NORMAL_SCALE:
      gthree_mesh_specglos_material_set_normal_map_scale (specglos, g_value_get_boxed (value));
      break;

    case PROP_DISPLACEMENT_MAP:
      gthree_mesh_specglos_material_set_displacement_map (specglos, g_value_get_object (value));
      break;

    case PROP_DISPLACEMENT_SCALE:
      gthree_mesh_specglos_material_set_displacement_scale (specglos, g_value_get_float (value));
      break;

    case PROP_DISPLACEMENT_BIAS:
      gthree_mesh_specglos_material_set_displacement_bias (specglos, g_value_get_float (value));
      break;

    case PROP_GLOSSINESS_MAP:
      gthree_mesh_specglos_material_set_glossiness_map (specglos, g_value_get_object (value));
      break;

    case PROP_SPECULAR_MAP:
      gthree_mesh_specglos_material_set_specular_map (specglos, g_value_get_object (value));
      break;

    case PROP_ALPHA_MAP:
      gthree_mesh_specglos_material_set_alpha_map (specglos, g_value_get_object (value));
      break;

    case PROP_ENV_MAP:
      gthree_mesh_specglos_material_set_env_map (specglos, g_value_get_object (value));
      break;

    case PROP_ENV_MAP_INTENSITY:
      gthree_mesh_specglos_material_set_env_map_intensity (specglos, g_value_get_float (value));
      break;

    case PROP_REFRACTION_RATIO:
      gthree_mesh_specglos_material_set_refraction_ratio (specglos, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_specglos_material_get_property (GObject *obj,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
  GthreeMeshSpecglosMaterial *specglos = GTHREE_MESH_SPECGLOS_MATERIAL (obj);
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

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

    case PROP_GLOSSINESS:
      g_value_set_float (value, priv->glossiness);
      break;

    case PROP_SPECULAR_FACTOR:
      g_value_set_boxed (value, &priv->specular);
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

    case PROP_NORMAL_SCALE:
      g_value_set_boxed (value, &priv->normal_scale);
      break;

    case PROP_DISPLACEMENT_MAP:
      g_value_set_object (value, priv->displacement_map);
      break;

    case PROP_DISPLACEMENT_SCALE:
      g_value_set_float (value, priv->displacement_scale);
      break;

    case PROP_DISPLACEMENT_BIAS:
      g_value_set_float (value, priv->displacement_bias);
      break;

    case PROP_GLOSSINESS_MAP:
      g_value_set_object (value, priv->glossiness_map);
      break;

    case PROP_SPECULAR_MAP:
      g_value_set_object (value, priv->specular_map);
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
gthree_mesh_specglos_material_class_init (GthreeMeshSpecglosMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->finalize = gthree_mesh_specglos_material_finalize;
  gobject_class->set_property = gthree_mesh_specglos_material_set_property;
  gobject_class->get_property = gthree_mesh_specglos_material_get_property;

  material_class->get_shader = gthree_mesh_specglos_material_real_get_shader;
  material_class->set_params = gthree_mesh_specglos_material_real_set_params;
  material_class->set_uniforms = gthree_mesh_specglos_material_real_set_uniforms;
  material_class->needs_camera_pos = gthree_mesh_specglos_material_needs_camera_pos;
  material_class->needs_view_matrix = gthree_mesh_specglos_material_needs_view_matrix;
  material_class->needs_lights = gthree_mesh_specglos_material_needs_lights;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_COLOR] =
    g_param_spec_boxed ("emissive-color", "Emissive Color", "Emissive",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_INTENSITY] =
    g_param_spec_float ("emissive-intensity", "Emissive intensity", "Emissive intensity",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_EMISSIVE_MAP] =
    g_param_spec_object ("emissive-map", "Emissive map", "Emissive map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_GLOSSINESS] =
    g_param_spec_float ("glossiness", "Glossiness", "Glossiness",
                        0.f, 1.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_FACTOR] =
    g_param_spec_boxed ("specular-factor", "Specular facto", "Specular factor",
                        GRAPHENE_TYPE_VEC3,
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
  obj_props[PROP_NORMAL_SCALE] =
    g_param_spec_boxed ("normal-scale", "Normal scale", "Normal scale",
                        GRAPHENE_TYPE_VEC2,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_DISPLACEMENT_MAP] =
    g_param_spec_object ("displacement-map", "Displacement map", "Displacement map",
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
  obj_props[PROP_GLOSSINESS_MAP] =
    g_param_spec_object ("glossiness-map", "Glossiness map", "Glossiness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_MAP] =
    g_param_spec_object ("specular-map", "Specular map", "Specular map",
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

const graphene_vec3_t *
gthree_mesh_specglos_material_get_emissive_color (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return &priv->emissive;
}

void
gthree_mesh_specglos_material_set_emissive_color (GthreeMeshSpecglosMaterial *specglos,
                                               const graphene_vec3_t *color)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->emissive = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));
}


GthreeTexture *
gthree_mesh_specglos_material_get_emissive_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->emissive_map;
}

void
gthree_mesh_specglos_material_set_emissive_map (GthreeMeshSpecglosMaterial *specglos,
                                                GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->emissive_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_EMISSIVE_MAP]);
    }
}

float
gthree_mesh_specglos_material_get_emissive_intensity (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->emissive_intensity;
}

void
gthree_mesh_specglos_material_set_emissive_intensity (GthreeMeshSpecglosMaterial *specglos,
                                                      float                value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->emissive_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_EMISSIVE_INTENSITY]);
}

float
gthree_mesh_specglos_material_get_refraction_ratio (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->refraction_ratio;
}

void
gthree_mesh_specglos_material_set_refraction_ratio (GthreeMeshSpecglosMaterial *specglos,
                                                   float                ratio)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->refraction_ratio = ratio;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_REFRACTION_RATIO]);
}


const graphene_vec3_t *
gthree_mesh_specglos_material_get_color (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return &priv->color;
}

void
gthree_mesh_specglos_material_set_color (GthreeMeshSpecglosMaterial *specglos,
                                         const graphene_vec3_t *color)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (graphene_vec3_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_COLOR]);
}

void
gthree_mesh_specglos_material_set_map (GthreeMeshSpecglosMaterial *specglos,
                                       GthreeTexture *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_MAP]);
    }
}

GthreeTexture *
gthree_mesh_specglos_material_get_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->map;
}

void
gthree_mesh_specglos_material_set_env_map (GthreeMeshSpecglosMaterial *specglos,
                                           GthreeTexture *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->env_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_ENV_MAP]);
    }
}

GthreeTexture *
gthree_mesh_specglos_material_get_env_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->env_map;
}

float
gthree_mesh_specglos_material_get_glossiness  (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->glossiness;
}

void
gthree_mesh_specglos_material_set_glossiness (GthreeMeshSpecglosMaterial *specglos,
                                             float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->glossiness = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_GLOSSINESS]);
}

const graphene_vec3_t *
gthree_mesh_specglos_material_get_specular_factor  (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return &priv->specular;
}

void
gthree_mesh_specglos_material_set_specular_factor (GthreeMeshSpecglosMaterial *specglos,
                                                   const graphene_vec3_t      *factor)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);;

  priv->specular = *factor;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_SPECULAR_FACTOR]);
}

GthreeTexture *
gthree_mesh_specglos_material_get_light_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->light_map;
}

void
gthree_mesh_specglos_material_set_light_map (GthreeMeshSpecglosMaterial *specglos,
                                             GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->light_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_LIGHT_MAP]);
    }
}

float
gthree_mesh_specglos_material_get_light_map_intensity (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->light_map_intensity;
}

void
gthree_mesh_specglos_material_set_light_map_intensity (GthreeMeshSpecglosMaterial *specglos,
                                                       float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->light_map_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_LIGHT_MAP_INTENSITY]);
}

GthreeTexture *
gthree_mesh_specglos_material_get_ao_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->ao_map;
}

void
gthree_mesh_specglos_material_set_ao_map (GthreeMeshSpecglosMaterial *specglos,
                                          GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->ao_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_AO_MAP]);
    }
}

float
gthree_mesh_specglos_material_get_ao_map_intensity (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->ao_map_intensity;
}

void
gthree_mesh_specglos_material_set_ao_map_intensity (GthreeMeshSpecglosMaterial *specglos,
                                                    float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->ao_map_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_AO_MAP_INTENSITY]);

}

GthreeTexture *
gthree_mesh_specglos_material_get_bump_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->bump_map;
}

void
gthree_mesh_specglos_material_set_bump_map (GthreeMeshSpecglosMaterial *specglos,
                                            GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->bump_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_BUMP_MAP]);
    }
}

float
gthree_mesh_specglos_material_get_bump_scale (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->bump_scale;
}

void
gthree_mesh_specglos_material_set_bump_scale (GthreeMeshSpecglosMaterial *specglos,
                                              float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->bump_scale = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_BUMP_SCALE]);
}

GthreeTexture *
gthree_mesh_specglos_material_get_normal_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->normal_map;
}

void
gthree_mesh_specglos_material_set_normal_map (GthreeMeshSpecglosMaterial *specglos,
                                              GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->normal_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_NORMAL_MAP]);
    }
}

GthreeNormalMapType
gthree_mesh_specglos_material_get_normal_map_type (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->normal_map_type;
}

void
gthree_mesh_specglos_material_set_normal_map_type (GthreeMeshSpecglosMaterial *specglos,
                                                   GthreeNormalMapType         type)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->normal_map_type = type;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_NORMAL_MAP_TYPE]);
}

const graphene_vec2_t *
gthree_mesh_specglos_material_get_normal_map_scale (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return &priv->normal_scale;
}

void
gthree_mesh_specglos_material_set_normal_map_scale (GthreeMeshSpecglosMaterial *specglos,
                                                    graphene_vec2_t            *scale)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->normal_scale = *scale;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_NORMAL_SCALE]);
}

GthreeTexture *
gthree_mesh_specglos_material_get_displacement_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->displacement_map;
}

void
gthree_mesh_specglos_material_set_displacement_map (GthreeMeshSpecglosMaterial *specglos,
                                                    GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->displacement_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_DISPLACEMENT_MAP]);
    }
}

float
gthree_mesh_specglos_material_get_displacement_scale (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->displacement_scale;
}

void
gthree_mesh_specglos_material_set_displacement_scale  (GthreeMeshSpecglosMaterial *specglos,
                                                       float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->displacement_scale = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_DISPLACEMENT_SCALE]);
}

float
gthree_mesh_specglos_material_get_displacement_bias (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->displacement_bias;
}

void
gthree_mesh_specglos_material_set_displacement_bias (GthreeMeshSpecglosMaterial *specglos,
                                                     float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);


  priv->displacement_bias = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_DISPLACEMENT_BIAS]);
}

GthreeTexture *
gthree_mesh_specglos_material_get_glossiness_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->glossiness_map;
}

void
gthree_mesh_specglos_material_set_glossiness_map (GthreeMeshSpecglosMaterial *specglos,
                                                 GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->glossiness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_GLOSSINESS_MAP]);
    }
}

GthreeTexture *
gthree_mesh_specglos_material_get_specular_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->specular_map;
}

void
gthree_mesh_specglos_material_set_specular_map (GthreeMeshSpecglosMaterial *specglos,
                                                 GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->specular_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_SPECULAR_MAP]);
    }
}

GthreeTexture *
gthree_mesh_specglos_material_get_alpha_map (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->alpha_map;
}

void
gthree_mesh_specglos_material_set_alpha_map (GthreeMeshSpecglosMaterial *specglos,
                                             GthreeTexture              *texture)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  if (g_set_object (&priv->alpha_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

      g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_ALPHA_MAP]);
    }
}

float
gthree_mesh_specglos_material_get_env_map_intensity (GthreeMeshSpecglosMaterial *specglos)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  return priv->env_map_intensity;
}

void
gthree_mesh_specglos_material_set_env_map_intensity (GthreeMeshSpecglosMaterial *specglos,
                                                     float                       value)
{
  GthreeMeshSpecglosMaterialPrivate *priv = gthree_mesh_specglos_material_get_instance_private (specglos);

  priv->env_map_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (specglos));

  g_object_notify_by_pspec (G_OBJECT (specglos), obj_props[PROP_ENV_MAP_INTENSITY]);
}
