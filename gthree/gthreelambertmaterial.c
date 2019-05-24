#include <math.h>
#include <epoxy/gl.h>

#include "gthreelambertmaterial.h"

typedef struct {
  GdkRGBA ambient;
  GdkRGBA emissive;

  gboolean wrap_around;
  graphene_vec3_t wrap_rgb;
} GthreeLambertMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeLambertMaterial, gthree_lambert_material, GTHREE_TYPE_BASIC_MATERIAL);

GthreeLambertMaterial *
gthree_lambert_material_new ()
{
  GthreeLambertMaterial *material;

  material = g_object_new (gthree_lambert_material_get_type (),
                           NULL);

  return material;
}

static void
gthree_lambert_material_init (GthreeLambertMaterial *lambert)
{
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);

  priv->ambient.red = 1.0;
  priv->ambient.green = 1.0;
  priv->ambient.blue = 1.0;
  priv->ambient.alpha = 1.0;

  priv->emissive.red = 0.0;
  priv->emissive.green = 0.0;
  priv->emissive.blue = 0.0;
  priv->emissive.alpha = 1.0;
}

static void
gthree_lambert_material_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_lambert_material_parent_class)->finalize (obj);
}

static GthreeShader *
gthree_lambert_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("lambert");
}

static void
gthree_lambert_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GthreeLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (material);
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);

  GTHREE_MATERIAL_CLASS (gthree_lambert_material_parent_class)->set_params (material, params);

  params->wrap_around = priv->wrap_around;
}

static void
gthree_lambert_material_real_set_uniforms (GthreeMaterial *material,
                                           GthreeUniforms *uniforms,
                                           GthreeCamera   *camera)
{
  GthreeLambertMaterial *lambert = GTHREE_LAMBERT_MATERIAL (material);
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_lambert_material_parent_class)->set_uniforms (material, uniforms, camera);

#if TODO
  if ( _this.gammaInput )
    {
      uniforms.ambient.value.copyGammaToLinear( material.ambient );
      uniforms.emissive.value.copyGammaToLinear( material.emissive );
    }
  else
#endif
    {
      uni = gthree_uniforms_lookup_from_string (uniforms, "ambient");
      if (uni != NULL)
        gthree_uniform_set_color (uni, &priv->ambient);

      uni = gthree_uniforms_lookup_from_string (uniforms, "emissive");
      if (uni != NULL)
        gthree_uniform_set_color (uni, &priv->emissive);
    }

#if TODO
  if (priv->wrap_around )
    {
      uniforms.wrapRGB.value.copy( material.wrapRGB );
    }
#endif
}

static gboolean
gthree_lambert_material_needs_view_matrix (GthreeMaterial *material)
{
  return TRUE;
}

static gboolean
gthree_lambert_material_needs_lights (GthreeMaterial *material)
{
  return TRUE;
}

static void
gthree_lambert_material_class_init (GthreeLambertMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_lambert_material_finalize;
  GTHREE_MATERIAL_CLASS(klass)->get_shader = gthree_lambert_material_real_get_shader;
  GTHREE_MATERIAL_CLASS(klass)->set_params = gthree_lambert_material_real_set_params;
  GTHREE_MATERIAL_CLASS(klass)->set_uniforms = gthree_lambert_material_real_set_uniforms;
  GTHREE_MATERIAL_CLASS(klass)->needs_view_matrix = gthree_lambert_material_needs_view_matrix;
  GTHREE_MATERIAL_CLASS(klass)->needs_lights = gthree_lambert_material_needs_lights;
}

const GdkRGBA *
gthree_lambert_material_get_ambient_color (GthreeLambertMaterial *lambert)
{
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);

  return &priv->ambient;
}

void
gthree_lambert_material_set_ambient_color (GthreeLambertMaterial *lambert,
                                           const GdkRGBA *color)
{
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);

  priv->ambient = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);
}

const GdkRGBA *
gthree_lambert_material_get_emissive_color (GthreeLambertMaterial *lambert)
{
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);

  return &priv->emissive;
}

void
gthree_lambert_material_set_emissive_color (GthreeLambertMaterial *lambert,
                                            const GdkRGBA *color)
{
  GthreeLambertMaterialPrivate *priv = gthree_lambert_material_get_instance_private (lambert);

  priv->emissive = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (lambert), TRUE);
}
