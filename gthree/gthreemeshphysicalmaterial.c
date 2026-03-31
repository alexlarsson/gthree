#include <math.h>
#include <epoxy/gl.h>
#include <graphene-gobject.h>

#include "gthreemeshphysicalmaterial.h"
#include "gthreeuniforms.h"
#include "gthreeprivate.h"

typedef struct {
  float clearcoat;
  GthreeTexture *clearcoat_map;
  float clearcoat_roughness;
  GthreeTexture *clearcoat_roughness_map;
  GthreeTexture *clearcoat_normal_map;
  graphene_vec2_t clearcoat_normal_scale;

  float ior;

  float iridescence;
  GthreeTexture *iridescence_map;
  float iridescence_ior;
  float iridescence_thickness_min;
  float iridescence_thickness_max;
  GthreeTexture *iridescence_thickness_map;

  float sheen;
  graphene_vec3_t sheen_color;
  GthreeTexture *sheen_color_map;
  float sheen_roughness;
  GthreeTexture *sheen_roughness_map;

  float transmission;
  GthreeTexture *transmission_map;
  float thickness;
  GthreeTexture *thickness_map;
  float attenuation_distance;
  graphene_vec3_t attenuation_color;

  float dispersion;

  float specular_intensity;
  GthreeTexture *specular_intensity_map;
  graphene_vec3_t specular_color;
  GthreeTexture *specular_color_map;

  float anisotropy;
  float anisotropy_rotation;
  GthreeTexture *anisotropy_map;
} GthreeMeshPhysicalMaterialPrivate;


enum {
  PROP_0,

  PROP_CLEARCOAT,
  PROP_CLEARCOAT_MAP,
  PROP_CLEARCOAT_ROUGHNESS,
  PROP_CLEARCOAT_ROUGHNESS_MAP,
  PROP_CLEARCOAT_NORMAL_MAP,
  PROP_CLEARCOAT_NORMAL_SCALE,
  PROP_IOR,
  PROP_IRIDESCENCE,
  PROP_IRIDESCENCE_MAP,
  PROP_IRIDESCENCE_IOR,
  PROP_IRIDESCENCE_THICKNESS_MIN,
  PROP_IRIDESCENCE_THICKNESS_MAX,
  PROP_IRIDESCENCE_THICKNESS_MAP,
  PROP_SHEEN,
  PROP_SHEEN_COLOR,
  PROP_SHEEN_COLOR_MAP,
  PROP_SHEEN_ROUGHNESS,
  PROP_SHEEN_ROUGHNESS_MAP,
  PROP_TRANSMISSION,
  PROP_TRANSMISSION_MAP,
  PROP_THICKNESS,
  PROP_THICKNESS_MAP,
  PROP_ATTENUATION_DISTANCE,
  PROP_ATTENUATION_COLOR,
  PROP_DISPERSION,
  PROP_SPECULAR_INTENSITY,
  PROP_SPECULAR_INTENSITY_MAP,
  PROP_SPECULAR_COLOR,
  PROP_SPECULAR_COLOR_MAP,
  PROP_ANISOTROPY,
  PROP_ANISOTROPY_ROTATION,
  PROP_ANISOTROPY_MAP,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshPhysicalMaterial, gthree_mesh_physical_material, GTHREE_TYPE_MESH_STANDARD_MATERIAL);

GthreeMeshPhysicalMaterial *
gthree_mesh_physical_material_new ()
{
  GthreeMeshPhysicalMaterial *material;

  material = g_object_new (gthree_mesh_physical_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_mesh_physical_material_init (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->clearcoat = 0;
  priv->clearcoat_roughness = 0;
  graphene_vec2_init (&priv->clearcoat_normal_scale, 1.0, 1.0);

  priv->ior = 1.5;

  priv->iridescence = 0;
  priv->iridescence_ior = 1.3;
  priv->iridescence_thickness_min = 100;
  priv->iridescence_thickness_max = 400;

  priv->sheen = 0;
  graphene_vec3_init (&priv->sheen_color, 0.0, 0.0, 0.0);
  priv->sheen_roughness = 1.0;

  priv->transmission = 0;
  priv->thickness = 0;
  priv->attenuation_distance = INFINITY;
  graphene_vec3_init (&priv->attenuation_color, 1.0, 1.0, 1.0);

  priv->specular_intensity = 1.0;
  graphene_vec3_init (&priv->specular_color, 1.0, 1.0, 1.0);

  priv->anisotropy = 0;
  priv->anisotropy_rotation = 0;
  priv->dispersion = 0;
}

static void
gthree_mesh_physical_material_finalize (GObject *obj)
{
  GthreeMeshPhysicalMaterial *physical = GTHREE_MESH_PHYSICAL_MATERIAL (obj);
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  g_clear_object (&priv->clearcoat_map);
  g_clear_object (&priv->clearcoat_roughness_map);
  g_clear_object (&priv->clearcoat_normal_map);
  g_clear_object (&priv->iridescence_map);
  g_clear_object (&priv->iridescence_thickness_map);
  g_clear_object (&priv->sheen_color_map);
  g_clear_object (&priv->sheen_roughness_map);
  g_clear_object (&priv->transmission_map);
  g_clear_object (&priv->thickness_map);
  g_clear_object (&priv->specular_intensity_map);
  g_clear_object (&priv->specular_color_map);
  g_clear_object (&priv->anisotropy_map);

  G_OBJECT_CLASS (gthree_mesh_physical_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_mesh_physical_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("physical");
}

static void
gthree_mesh_physical_material_real_set_params (GthreeMaterial *material,
                                               GthreeProgramParameters *params)
{
  GthreeMeshPhysicalMaterial *physical = GTHREE_MESH_PHYSICAL_MATERIAL (material);
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  GTHREE_MATERIAL_CLASS (gthree_mesh_physical_material_parent_class)->set_params (material, params);

  params->clearcoat = priv->clearcoat > 0;
  params->clearcoat_map = priv->clearcoat_map != NULL;
  params->clearcoat_roughness_map = priv->clearcoat_roughness_map != NULL;
  params->clearcoat_normal_map = priv->clearcoat_normal_map != NULL;

  params->anisotropy = priv->anisotropy != 0;
  params->anisotropy_map = priv->anisotropy_map != NULL;

  params->iridescence = priv->iridescence > 0;
  params->iridescence_map = priv->iridescence_map != NULL;
  params->iridescence_thickness_map = priv->iridescence_thickness_map != NULL;

  params->sheen = priv->sheen > 0;
  params->sheen_color_map = priv->sheen_color_map != NULL;
  params->sheen_roughness_map = priv->sheen_roughness_map != NULL;

  params->transmission = priv->transmission > 0;
  params->transmission_map = priv->transmission_map != NULL;
  params->thickness_map = priv->thickness_map != NULL;

  params->dispersion = priv->dispersion > 0;

  params->specular_color_map = priv->specular_color_map != NULL;
  params->specular_intensity_map = priv->specular_intensity_map != NULL;
}

static void
gthree_mesh_physical_material_real_set_uniforms (GthreeMaterial *material,
                                                 GthreeUniforms *uniforms,
                                                 GthreeCamera   *camera,
                                                 GthreeRenderer *renderer)
{
  GthreeMeshPhysicalMaterial *physical = GTHREE_MESH_PHYSICAL_MATERIAL (material);
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_mesh_physical_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

  uni = gthree_uniforms_lookup_from_string (uniforms, "ior");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->ior);

  uni = gthree_uniforms_lookup_from_string (uniforms, "specularIntensity");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->specular_intensity);

  uni = gthree_uniforms_lookup_from_string (uniforms, "specularColor");
  if (uni != NULL)
    gthree_uniform_set_vec3 (uni, &priv->specular_color);

  if (priv->specular_intensity_map)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "specularIntensityMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->specular_intensity_map);
    }

  if (priv->specular_color_map)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "specularColorMap");
      if (uni != NULL)
        gthree_uniform_set_texture (uni, priv->specular_color_map);
    }

  if (priv->clearcoat > 0)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "clearcoat");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->clearcoat);

      uni = gthree_uniforms_lookup_from_string (uniforms, "clearcoatRoughness");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->clearcoat_roughness);

      if (priv->clearcoat_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "clearcoatMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->clearcoat_map);
        }

      if (priv->clearcoat_roughness_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "clearcoatRoughnessMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->clearcoat_roughness_map);
        }

      if (priv->clearcoat_normal_map)
        {
          float sign = 1.0;
          graphene_vec2_t clearcoat_normal_scale;

          uni = gthree_uniforms_lookup_from_string (uniforms, "clearcoatNormalMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->clearcoat_normal_map);

          if (gthree_material_get_side (GTHREE_MATERIAL (material)) == GTHREE_SIDE_BACK)
            sign = -1;

          graphene_vec2_scale (&priv->clearcoat_normal_scale, sign, &clearcoat_normal_scale);

          gthree_uniforms_set_vec2 (uniforms, "clearcoatNormalScale", &clearcoat_normal_scale);
        }
    }

  if (priv->sheen > 0)
    {
      graphene_vec3_t sheen_color;

      graphene_vec3_scale (&priv->sheen_color, priv->sheen, &sheen_color);

      uni = gthree_uniforms_lookup_from_string (uniforms, "sheenColor");
      if (uni != NULL)
        gthree_uniform_set_vec3 (uni, &sheen_color);

      uni = gthree_uniforms_lookup_from_string (uniforms, "sheenRoughness");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->sheen_roughness);

      if (priv->sheen_color_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "sheenColorMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->sheen_color_map);
        }

      if (priv->sheen_roughness_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "sheenRoughnessMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->sheen_roughness_map);
        }
    }

  if (priv->iridescence > 0)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "iridescence");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->iridescence);

      uni = gthree_uniforms_lookup_from_string (uniforms, "iridescenceIOR");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->iridescence_ior);

      uni = gthree_uniforms_lookup_from_string (uniforms, "iridescenceThicknessMinimum");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->iridescence_thickness_min);

      uni = gthree_uniforms_lookup_from_string (uniforms, "iridescenceThicknessMaximum");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->iridescence_thickness_max);

      if (priv->iridescence_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "iridescenceMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->iridescence_map);
        }

      if (priv->iridescence_thickness_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "iridescenceThicknessMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->iridescence_thickness_map);
        }
    }

  if (priv->transmission > 0)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "transmission");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->transmission);

      uni = gthree_uniforms_lookup_from_string (uniforms, "thickness");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->thickness);

      uni = gthree_uniforms_lookup_from_string (uniforms, "attenuationDistance");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->attenuation_distance);

      uni = gthree_uniforms_lookup_from_string (uniforms, "attenuationColor");
      if (uni != NULL)
        gthree_uniform_set_vec3 (uni, &priv->attenuation_color);

      if (priv->transmission_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "transmissionMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->transmission_map);
        }

      if (priv->thickness_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "thicknessMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->thickness_map);
        }
    }

  if (priv->anisotropy != 0)
    {
      graphene_vec2_t anisotropy_vec;

      graphene_vec2_init (&anisotropy_vec,
                          priv->anisotropy * cosf (priv->anisotropy_rotation),
                          priv->anisotropy * sinf (priv->anisotropy_rotation));

      uni = gthree_uniforms_lookup_from_string (uniforms, "anisotropyVector");
      if (uni != NULL)
        gthree_uniform_set_vec2 (uni, &anisotropy_vec);

      if (priv->anisotropy_map)
        {
          uni = gthree_uniforms_lookup_from_string (uniforms, "anisotropyMap");
          if (uni != NULL)
            gthree_uniform_set_texture (uni, priv->anisotropy_map);
        }
    }

  if (priv->dispersion > 0)
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "dispersion");
      if (uni != NULL)
        gthree_uniform_set_float (uni, priv->dispersion);
    }
}

static void
gthree_mesh_physical_material_set_property (GObject *obj,
                                            guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec)
{
  GthreeMeshPhysicalMaterial *physical = GTHREE_MESH_PHYSICAL_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_CLEARCOAT:
      gthree_mesh_physical_material_set_clearcoat (physical, g_value_get_float (value));
      break;

    case PROP_CLEARCOAT_MAP:
      gthree_mesh_physical_material_set_clearcoat_map (physical, g_value_get_object (value));
      break;

    case PROP_CLEARCOAT_ROUGHNESS:
      gthree_mesh_physical_material_set_clearcoat_roughness (physical, g_value_get_float (value));
      break;

    case PROP_CLEARCOAT_ROUGHNESS_MAP:
      gthree_mesh_physical_material_set_clearcoat_roughness_map (physical, g_value_get_object (value));
      break;

    case PROP_CLEARCOAT_NORMAL_MAP:
      gthree_mesh_physical_material_set_clearcoat_normal_map (physical, g_value_get_object (value));
      break;

    case PROP_CLEARCOAT_NORMAL_SCALE:
      gthree_mesh_physical_material_set_clearcoat_normal_scale (physical, g_value_get_boxed (value));
      break;

    case PROP_IOR:
      gthree_mesh_physical_material_set_ior (physical, g_value_get_float (value));
      break;

    case PROP_IRIDESCENCE:
      gthree_mesh_physical_material_set_iridescence (physical, g_value_get_float (value));
      break;

    case PROP_IRIDESCENCE_MAP:
      gthree_mesh_physical_material_set_iridescence_map (physical, g_value_get_object (value));
      break;

    case PROP_IRIDESCENCE_IOR:
      gthree_mesh_physical_material_set_iridescence_ior (physical, g_value_get_float (value));
      break;

    case PROP_IRIDESCENCE_THICKNESS_MIN:
      gthree_mesh_physical_material_set_iridescence_thickness_min (physical, g_value_get_float (value));
      break;

    case PROP_IRIDESCENCE_THICKNESS_MAX:
      gthree_mesh_physical_material_set_iridescence_thickness_max (physical, g_value_get_float (value));
      break;

    case PROP_IRIDESCENCE_THICKNESS_MAP:
      gthree_mesh_physical_material_set_iridescence_thickness_map (physical, g_value_get_object (value));
      break;

    case PROP_SHEEN:
      gthree_mesh_physical_material_set_sheen (physical, g_value_get_float (value));
      break;

    case PROP_SHEEN_COLOR:
      gthree_mesh_physical_material_set_sheen_color (physical, g_value_get_boxed (value));
      break;

    case PROP_SHEEN_COLOR_MAP:
      gthree_mesh_physical_material_set_sheen_color_map (physical, g_value_get_object (value));
      break;

    case PROP_SHEEN_ROUGHNESS:
      gthree_mesh_physical_material_set_sheen_roughness (physical, g_value_get_float (value));
      break;

    case PROP_SHEEN_ROUGHNESS_MAP:
      gthree_mesh_physical_material_set_sheen_roughness_map (physical, g_value_get_object (value));
      break;

    case PROP_TRANSMISSION:
      gthree_mesh_physical_material_set_transmission (physical, g_value_get_float (value));
      break;

    case PROP_TRANSMISSION_MAP:
      gthree_mesh_physical_material_set_transmission_map (physical, g_value_get_object (value));
      break;

    case PROP_THICKNESS:
      gthree_mesh_physical_material_set_thickness (physical, g_value_get_float (value));
      break;

    case PROP_THICKNESS_MAP:
      gthree_mesh_physical_material_set_thickness_map (physical, g_value_get_object (value));
      break;

    case PROP_ATTENUATION_DISTANCE:
      gthree_mesh_physical_material_set_attenuation_distance (physical, g_value_get_float (value));
      break;

    case PROP_ATTENUATION_COLOR:
      gthree_mesh_physical_material_set_attenuation_color (physical, g_value_get_boxed (value));
      break;

    case PROP_DISPERSION:
      gthree_mesh_physical_material_set_dispersion (physical, g_value_get_float (value));
      break;

    case PROP_SPECULAR_INTENSITY:
      gthree_mesh_physical_material_set_specular_intensity (physical, g_value_get_float (value));
      break;

    case PROP_SPECULAR_INTENSITY_MAP:
      gthree_mesh_physical_material_set_specular_intensity_map (physical, g_value_get_object (value));
      break;

    case PROP_SPECULAR_COLOR:
      gthree_mesh_physical_material_set_specular_color (physical, g_value_get_boxed (value));
      break;

    case PROP_SPECULAR_COLOR_MAP:
      gthree_mesh_physical_material_set_specular_color_map (physical, g_value_get_object (value));
      break;

    case PROP_ANISOTROPY:
      gthree_mesh_physical_material_set_anisotropy (physical, g_value_get_float (value));
      break;

    case PROP_ANISOTROPY_ROTATION:
      gthree_mesh_physical_material_set_anisotropy_rotation (physical, g_value_get_float (value));
      break;

    case PROP_ANISOTROPY_MAP:
      gthree_mesh_physical_material_set_anisotropy_map (physical, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_physical_material_get_property (GObject *obj,
                                            guint prop_id,
                                            GValue *value,
                                            GParamSpec *pspec)
{
  GthreeMeshPhysicalMaterial *physical = GTHREE_MESH_PHYSICAL_MATERIAL (obj);
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  switch (prop_id)
    {
    case PROP_CLEARCOAT:
      g_value_set_float (value, priv->clearcoat);
      break;

    case PROP_CLEARCOAT_MAP:
      g_value_set_object (value, priv->clearcoat_map);
      break;

    case PROP_CLEARCOAT_ROUGHNESS:
      g_value_set_float (value, priv->clearcoat_roughness);
      break;

    case PROP_CLEARCOAT_ROUGHNESS_MAP:
      g_value_set_object (value, priv->clearcoat_roughness_map);
      break;

    case PROP_CLEARCOAT_NORMAL_MAP:
      g_value_set_object (value, priv->clearcoat_normal_map);
      break;

    case PROP_CLEARCOAT_NORMAL_SCALE:
      g_value_set_boxed (value, &priv->clearcoat_normal_scale);
      break;

    case PROP_IOR:
      g_value_set_float (value, priv->ior);
      break;

    case PROP_IRIDESCENCE:
      g_value_set_float (value, priv->iridescence);
      break;

    case PROP_IRIDESCENCE_MAP:
      g_value_set_object (value, priv->iridescence_map);
      break;

    case PROP_IRIDESCENCE_IOR:
      g_value_set_float (value, priv->iridescence_ior);
      break;

    case PROP_IRIDESCENCE_THICKNESS_MIN:
      g_value_set_float (value, priv->iridescence_thickness_min);
      break;

    case PROP_IRIDESCENCE_THICKNESS_MAX:
      g_value_set_float (value, priv->iridescence_thickness_max);
      break;

    case PROP_IRIDESCENCE_THICKNESS_MAP:
      g_value_set_object (value, priv->iridescence_thickness_map);
      break;

    case PROP_SHEEN:
      g_value_set_float (value, priv->sheen);
      break;

    case PROP_SHEEN_COLOR:
      g_value_set_boxed (value, &priv->sheen_color);
      break;

    case PROP_SHEEN_COLOR_MAP:
      g_value_set_object (value, priv->sheen_color_map);
      break;

    case PROP_SHEEN_ROUGHNESS:
      g_value_set_float (value, priv->sheen_roughness);
      break;

    case PROP_SHEEN_ROUGHNESS_MAP:
      g_value_set_object (value, priv->sheen_roughness_map);
      break;

    case PROP_TRANSMISSION:
      g_value_set_float (value, priv->transmission);
      break;

    case PROP_TRANSMISSION_MAP:
      g_value_set_object (value, priv->transmission_map);
      break;

    case PROP_THICKNESS:
      g_value_set_float (value, priv->thickness);
      break;

    case PROP_THICKNESS_MAP:
      g_value_set_object (value, priv->thickness_map);
      break;

    case PROP_ATTENUATION_DISTANCE:
      g_value_set_float (value, priv->attenuation_distance);
      break;

    case PROP_ATTENUATION_COLOR:
      g_value_set_boxed (value, &priv->attenuation_color);
      break;

    case PROP_DISPERSION:
      g_value_set_float (value, priv->dispersion);
      break;

    case PROP_SPECULAR_INTENSITY:
      g_value_set_float (value, priv->specular_intensity);
      break;

    case PROP_SPECULAR_INTENSITY_MAP:
      g_value_set_object (value, priv->specular_intensity_map);
      break;

    case PROP_SPECULAR_COLOR:
      g_value_set_boxed (value, &priv->specular_color);
      break;

    case PROP_SPECULAR_COLOR_MAP:
      g_value_set_object (value, priv->specular_color_map);
      break;

    case PROP_ANISOTROPY:
      g_value_set_float (value, priv->anisotropy);
      break;

    case PROP_ANISOTROPY_ROTATION:
      g_value_set_float (value, priv->anisotropy_rotation);
      break;

    case PROP_ANISOTROPY_MAP:
      g_value_set_object (value, priv->anisotropy_map);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}


static void
gthree_mesh_physical_material_class_init (GthreeMeshPhysicalMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->finalize = gthree_mesh_physical_material_finalize;
  gobject_class->set_property = gthree_mesh_physical_material_set_property;
  gobject_class->get_property = gthree_mesh_physical_material_get_property;

  material_class->get_shader = gthree_mesh_physical_material_real_get_shader;
  material_class->set_params = gthree_mesh_physical_material_real_set_params;
  material_class->set_uniforms = gthree_mesh_physical_material_real_set_uniforms;

  obj_props[PROP_CLEARCOAT] =
    g_param_spec_float ("clearcoat", "Clearcoat", "Clearcoat",
                        0.f, 1.f, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_CLEARCOAT_MAP] =
    g_param_spec_object ("clearcoat-map", "Clearcoat map", "Clearcoat map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_CLEARCOAT_ROUGHNESS] =
    g_param_spec_float ("clearcoat-roughness", "Clearcoat roughness", "Clearcoat roughness",
                        0.f, 1.f, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_CLEARCOAT_ROUGHNESS_MAP] =
    g_param_spec_object ("clearcoat-roughness-map", "Clearcoat roughness map", "Clearcoat roughness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_CLEARCOAT_NORMAL_MAP] =
    g_param_spec_object ("clearcoat-normal-map", "Clearcoat normal map", "Clearcoat normal map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_CLEARCOAT_NORMAL_SCALE] =
    g_param_spec_boxed ("clearcoat-normal-scale", "Clearcoat normal scale", "Clearcoat normal scale",
                        GRAPHENE_TYPE_VEC2,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IOR] =
    g_param_spec_float ("ior", "IOR", "IOR",
                        0.f, 10.f, 1.5f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IRIDESCENCE] =
    g_param_spec_float ("iridescence", "Iridescence", "Iridescence",
                        0.f, 1.f, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IRIDESCENCE_MAP] =
    g_param_spec_object ("iridescence-map", "Iridescence map", "Iridescence map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IRIDESCENCE_IOR] =
    g_param_spec_float ("iridescence-ior", "Iridescence IOR", "Iridescence IOR",
                        0.f, 10.f, 1.3f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IRIDESCENCE_THICKNESS_MIN] =
    g_param_spec_float ("iridescence-thickness-min", "Iridescence thickness min", "Iridescence thickness min",
                        0.f, 10000.f, 100.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IRIDESCENCE_THICKNESS_MAX] =
    g_param_spec_float ("iridescence-thickness-max", "Iridescence thickness max", "Iridescence thickness max",
                        0.f, 10000.f, 400.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_IRIDESCENCE_THICKNESS_MAP] =
    g_param_spec_object ("iridescence-thickness-map", "Iridescence thickness map", "Iridescence thickness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHEEN] =
    g_param_spec_float ("sheen", "Sheen", "Sheen",
                        0.f, 1.f, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHEEN_COLOR] =
    g_param_spec_boxed ("sheen-color", "Sheen color", "Sheen color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHEEN_COLOR_MAP] =
    g_param_spec_object ("sheen-color-map", "Sheen color map", "Sheen color map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHEEN_ROUGHNESS] =
    g_param_spec_float ("sheen-roughness", "Sheen roughness", "Sheen roughness",
                        0.f, 1.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SHEEN_ROUGHNESS_MAP] =
    g_param_spec_object ("sheen-roughness-map", "Sheen roughness map", "Sheen roughness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_TRANSMISSION] =
    g_param_spec_float ("transmission", "Transmission", "Transmission",
                        0.f, 1.f, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_TRANSMISSION_MAP] =
    g_param_spec_object ("transmission-map", "Transmission map", "Transmission map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_THICKNESS] =
    g_param_spec_float ("thickness", "Thickness", "Thickness",
                        0.f, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_THICKNESS_MAP] =
    g_param_spec_object ("thickness-map", "Thickness map", "Thickness map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ATTENUATION_DISTANCE] =
    g_param_spec_float ("attenuation-distance", "Attenuation distance", "Attenuation distance",
                        0.f, G_MAXFLOAT, G_MAXFLOAT,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ATTENUATION_COLOR] =
    g_param_spec_boxed ("attenuation-color", "Attenuation color", "Attenuation color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_DISPERSION] =
    g_param_spec_float ("dispersion", "Dispersion", "Dispersion",
                        0.f, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_INTENSITY] =
    g_param_spec_float ("specular-intensity", "Specular intensity", "Specular intensity",
                        0.f, 10.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_INTENSITY_MAP] =
    g_param_spec_object ("specular-intensity-map", "Specular intensity map", "Specular intensity map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_COLOR] =
    g_param_spec_boxed ("specular-color", "Specular color", "Specular color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SPECULAR_COLOR_MAP] =
    g_param_spec_object ("specular-color-map", "Specular color map", "Specular color map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ANISOTROPY] =
    g_param_spec_float ("anisotropy", "Anisotropy", "Anisotropy",
                        -1.f, 1.f, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ANISOTROPY_ROTATION] =
    g_param_spec_float ("anisotropy-rotation", "Anisotropy rotation", "Anisotropy rotation",
                        -G_MAXFLOAT, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ANISOTROPY_MAP] =
    g_param_spec_object ("anisotropy-map", "Anisotropy map", "Anisotropy map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

float
gthree_mesh_physical_material_get_clearcoat (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->clearcoat;
}

void
gthree_mesh_physical_material_set_clearcoat (GthreeMeshPhysicalMaterial *physical,
                                             float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->clearcoat = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_CLEARCOAT]);
}

/**
 * gthree_mesh_physical_material_get_clearcoat_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_clearcoat_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->clearcoat_map;
}

void
gthree_mesh_physical_material_set_clearcoat_map (GthreeMeshPhysicalMaterial *physical,
                                                 GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->clearcoat_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_CLEARCOAT_MAP]);
    }
}

float
gthree_mesh_physical_material_get_clearcoat_roughness (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->clearcoat_roughness;
}

void
gthree_mesh_physical_material_set_clearcoat_roughness (GthreeMeshPhysicalMaterial *physical,
                                                       float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->clearcoat_roughness = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_CLEARCOAT_ROUGHNESS]);
}

/**
 * gthree_mesh_physical_material_get_clearcoat_roughness_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_clearcoat_roughness_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->clearcoat_roughness_map;
}

void
gthree_mesh_physical_material_set_clearcoat_roughness_map (GthreeMeshPhysicalMaterial *physical,
                                                           GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->clearcoat_roughness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_CLEARCOAT_ROUGHNESS_MAP]);
    }
}

/**
 * gthree_mesh_physical_material_get_clearcoat_normal_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_clearcoat_normal_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->clearcoat_normal_map;
}

void
gthree_mesh_physical_material_set_clearcoat_normal_map (GthreeMeshPhysicalMaterial *physical,
                                                        GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->clearcoat_normal_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_CLEARCOAT_NORMAL_MAP]);
    }
}

const graphene_vec2_t *
gthree_mesh_physical_material_get_clearcoat_normal_scale (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return &priv->clearcoat_normal_scale;
}

void
gthree_mesh_physical_material_set_clearcoat_normal_scale (GthreeMeshPhysicalMaterial *physical,
                                                          graphene_vec2_t            *scale)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->clearcoat_normal_scale = *scale;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_CLEARCOAT_NORMAL_SCALE]);
}

float
gthree_mesh_physical_material_get_ior (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->ior;
}

void
gthree_mesh_physical_material_set_ior (GthreeMeshPhysicalMaterial *physical,
                                       float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->ior = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IOR]);
}

float
gthree_mesh_physical_material_get_iridescence (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->iridescence;
}

void
gthree_mesh_physical_material_set_iridescence (GthreeMeshPhysicalMaterial *physical,
                                               float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->iridescence = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IRIDESCENCE]);
}

/**
 * gthree_mesh_physical_material_get_iridescence_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_iridescence_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->iridescence_map;
}

void
gthree_mesh_physical_material_set_iridescence_map (GthreeMeshPhysicalMaterial *physical,
                                                   GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->iridescence_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IRIDESCENCE_MAP]);
    }
}

float
gthree_mesh_physical_material_get_iridescence_ior (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->iridescence_ior;
}

void
gthree_mesh_physical_material_set_iridescence_ior (GthreeMeshPhysicalMaterial *physical,
                                                   float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->iridescence_ior = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IRIDESCENCE_IOR]);
}

float
gthree_mesh_physical_material_get_iridescence_thickness_min (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->iridescence_thickness_min;
}

void
gthree_mesh_physical_material_set_iridescence_thickness_min (GthreeMeshPhysicalMaterial *physical,
                                                             float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->iridescence_thickness_min = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IRIDESCENCE_THICKNESS_MIN]);
}

float
gthree_mesh_physical_material_get_iridescence_thickness_max (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->iridescence_thickness_max;
}

void
gthree_mesh_physical_material_set_iridescence_thickness_max (GthreeMeshPhysicalMaterial *physical,
                                                             float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->iridescence_thickness_max = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IRIDESCENCE_THICKNESS_MAX]);
}

/**
 * gthree_mesh_physical_material_get_iridescence_thickness_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_iridescence_thickness_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->iridescence_thickness_map;
}

void
gthree_mesh_physical_material_set_iridescence_thickness_map (GthreeMeshPhysicalMaterial *physical,
                                                             GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->iridescence_thickness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_IRIDESCENCE_THICKNESS_MAP]);
    }
}

float
gthree_mesh_physical_material_get_sheen (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->sheen;
}

void
gthree_mesh_physical_material_set_sheen (GthreeMeshPhysicalMaterial *physical,
                                         float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->sheen = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SHEEN]);
}

const graphene_vec3_t *
gthree_mesh_physical_material_get_sheen_color (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return &priv->sheen_color;
}

void
gthree_mesh_physical_material_set_sheen_color (GthreeMeshPhysicalMaterial *physical,
                                               const graphene_vec3_t      *color)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->sheen_color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SHEEN_COLOR]);
}

/**
 * gthree_mesh_physical_material_get_sheen_color_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_sheen_color_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->sheen_color_map;
}

void
gthree_mesh_physical_material_set_sheen_color_map (GthreeMeshPhysicalMaterial *physical,
                                                   GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->sheen_color_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SHEEN_COLOR_MAP]);
    }
}

float
gthree_mesh_physical_material_get_sheen_roughness (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->sheen_roughness;
}

void
gthree_mesh_physical_material_set_sheen_roughness (GthreeMeshPhysicalMaterial *physical,
                                                   float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->sheen_roughness = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SHEEN_ROUGHNESS]);
}

/**
 * gthree_mesh_physical_material_get_sheen_roughness_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_sheen_roughness_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->sheen_roughness_map;
}

void
gthree_mesh_physical_material_set_sheen_roughness_map (GthreeMeshPhysicalMaterial *physical,
                                                       GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->sheen_roughness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SHEEN_ROUGHNESS_MAP]);
    }
}

float
gthree_mesh_physical_material_get_transmission (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->transmission;
}

void
gthree_mesh_physical_material_set_transmission (GthreeMeshPhysicalMaterial *physical,
                                                float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->transmission = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_TRANSMISSION]);
}

/**
 * gthree_mesh_physical_material_get_transmission_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_transmission_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->transmission_map;
}

void
gthree_mesh_physical_material_set_transmission_map (GthreeMeshPhysicalMaterial *physical,
                                                    GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->transmission_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_TRANSMISSION_MAP]);
    }
}

float
gthree_mesh_physical_material_get_thickness (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->thickness;
}

void
gthree_mesh_physical_material_set_thickness (GthreeMeshPhysicalMaterial *physical,
                                             float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->thickness = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_THICKNESS]);
}

/**
 * gthree_mesh_physical_material_get_thickness_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_thickness_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->thickness_map;
}

void
gthree_mesh_physical_material_set_thickness_map (GthreeMeshPhysicalMaterial *physical,
                                                 GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->thickness_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_THICKNESS_MAP]);
    }
}

float
gthree_mesh_physical_material_get_attenuation_distance (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->attenuation_distance;
}

void
gthree_mesh_physical_material_set_attenuation_distance (GthreeMeshPhysicalMaterial *physical,
                                                        float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->attenuation_distance = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_ATTENUATION_DISTANCE]);
}

const graphene_vec3_t *
gthree_mesh_physical_material_get_attenuation_color (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return &priv->attenuation_color;
}

void
gthree_mesh_physical_material_set_attenuation_color (GthreeMeshPhysicalMaterial *physical,
                                                     const graphene_vec3_t      *color)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->attenuation_color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_ATTENUATION_COLOR]);
}

float
gthree_mesh_physical_material_get_dispersion (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->dispersion;
}

void
gthree_mesh_physical_material_set_dispersion (GthreeMeshPhysicalMaterial *physical,
                                              float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->dispersion = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_DISPERSION]);
}

float
gthree_mesh_physical_material_get_specular_intensity (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->specular_intensity;
}

void
gthree_mesh_physical_material_set_specular_intensity (GthreeMeshPhysicalMaterial *physical,
                                                      float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->specular_intensity = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SPECULAR_INTENSITY]);
}

/**
 * gthree_mesh_physical_material_get_specular_intensity_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_specular_intensity_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->specular_intensity_map;
}

void
gthree_mesh_physical_material_set_specular_intensity_map (GthreeMeshPhysicalMaterial *physical,
                                                          GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->specular_intensity_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SPECULAR_INTENSITY_MAP]);
    }
}

const graphene_vec3_t *
gthree_mesh_physical_material_get_specular_color (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return &priv->specular_color;
}

void
gthree_mesh_physical_material_set_specular_color (GthreeMeshPhysicalMaterial *physical,
                                                  const graphene_vec3_t      *color)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->specular_color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SPECULAR_COLOR]);
}

/**
 * gthree_mesh_physical_material_get_specular_color_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_specular_color_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->specular_color_map;
}

void
gthree_mesh_physical_material_set_specular_color_map (GthreeMeshPhysicalMaterial *physical,
                                                      GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->specular_color_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_SPECULAR_COLOR_MAP]);
    }
}

float
gthree_mesh_physical_material_get_anisotropy (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->anisotropy;
}

void
gthree_mesh_physical_material_set_anisotropy (GthreeMeshPhysicalMaterial *physical,
                                              float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->anisotropy = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_ANISOTROPY]);
}

float
gthree_mesh_physical_material_get_anisotropy_rotation (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->anisotropy_rotation;
}

void
gthree_mesh_physical_material_set_anisotropy_rotation (GthreeMeshPhysicalMaterial *physical,
                                                       float                       value)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  priv->anisotropy_rotation = value;

  gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

  g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_ANISOTROPY_ROTATION]);
}

/**
 * gthree_mesh_physical_material_get_anisotropy_map:
 *
 * Returns: (transfer none):
 */
GthreeTexture *
gthree_mesh_physical_material_get_anisotropy_map (GthreeMeshPhysicalMaterial *physical)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  return priv->anisotropy_map;
}

void
gthree_mesh_physical_material_set_anisotropy_map (GthreeMeshPhysicalMaterial *physical,
                                                  GthreeTexture              *texture)
{
  GthreeMeshPhysicalMaterialPrivate *priv = gthree_mesh_physical_material_get_instance_private (physical);

  if (g_set_object (&priv->anisotropy_map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (physical));

      g_object_notify_by_pspec (G_OBJECT (physical), obj_props[PROP_ANISOTROPY_MAP]);
    }
}
