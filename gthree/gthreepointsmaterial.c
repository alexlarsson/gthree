#include <math.h>

#include "gthreepointsmaterial.h"
#include "gthreetypebuiltins.h"
#include "gthreecubetexture.h"
#include "gthreerenderer.h"
#include "gthreeprivate.h"

typedef struct {
  GdkRGBA color;
  float size;
  GthreeTexture *map;
  gboolean size_attenuation;
} GthreePointsMaterialPrivate;

enum {
      PROP_0,

      PROP_COLOR,
      PROP_MAP,
      PROP_SIZE,
      PROP_SIZE_ATTENUATION,

      N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreePointsMaterial, gthree_points_material, GTHREE_TYPE_MATERIAL)

static void
gthree_points_material_finalize (GObject *obj)
{
  GthreePointsMaterial *points_material = GTHREE_POINTS_MATERIAL (obj);
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  g_clear_object (&priv->map);
  G_OBJECT_CLASS (gthree_points_material_parent_class)->finalize (obj);
}

static void
gthree_points_material_set_property (GObject *obj,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  GthreePointsMaterial *points_material = GTHREE_POINTS_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_points_material_set_color (points_material, g_value_get_boxed (value));
      break;

    case PROP_SIZE:
      gthree_points_material_set_size (points_material, g_value_get_float (value));
      break;

    case PROP_MAP:
      gthree_points_material_set_map (points_material, g_value_get_object (value));
      break;

    case PROP_SIZE_ATTENUATION:
      gthree_points_material_set_size_attenuation (points_material, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_points_material_get_property (GObject *obj,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  GthreePointsMaterial *points_material = GTHREE_POINTS_MATERIAL (obj);
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_SIZE:
      g_value_set_float (value, priv->size);
      break;

    case PROP_MAP:
      g_value_set_object (value, priv->map);
      break;

    case PROP_SIZE_ATTENUATION:
      g_value_set_boolean (value, priv->size_attenuation);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static GthreeShader *
gthree_points_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("points");
}

static void
gthree_points_material_real_set_params (GthreeMaterial *material,
                                        GthreeProgramParameters *params)
{
  GthreePointsMaterial *points_material = GTHREE_POINTS_MATERIAL (material);
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  GTHREE_MATERIAL_CLASS (gthree_points_material_parent_class)->set_params (material, params);

  params->map = priv->map != NULL;
  if (params->map)
    params->map_encoding = gthree_texture_get_encoding (priv->map);

  params->size_attenuation = priv->size_attenuation;
}

static void
gthree_points_material_real_set_uniforms (GthreeMaterial *material,
                                          GthreeUniforms *uniforms,
                                          GthreeCamera   *camera,
                                          GthreeRenderer *renderer)
{
  GthreePointsMaterial *points_material = GTHREE_POINTS_MATERIAL (material);
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);
  GthreeUniform *uni;

  int pixel_ratio = gthree_renderer_get_pixel_ratio (renderer);
  int height = gthree_renderer_get_height (renderer);

  GTHREE_MATERIAL_CLASS (gthree_points_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "size");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->size * pixel_ratio);

  uni = gthree_uniforms_lookup_from_string (uniforms, "scale");
  if (uni != NULL)
    gthree_uniform_set_float (uni, 0.5 * height);

  uni = gthree_uniforms_lookup_from_string (uniforms, "map");
  if (uni != NULL)
    gthree_uniform_set_texture (uni, priv->map);

#ifdef TODO
  if (material.map !== null)
    {
      if (material.map.matrixAutoUpdate)
        material.map.updateMatrix();
      uniforms.uvTransform.value.copy (material.map.matrix);
    }
#endif
}


static void
gthree_points_material_class_init (GthreePointsMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_points_material_set_property;
  gobject_class->get_property = gthree_points_material_get_property;
  gobject_class->finalize = gthree_points_material_finalize;
  material_class->get_shader = gthree_points_material_real_get_shader;
  material_class->set_params = gthree_points_material_real_set_params;
  material_class->set_uniforms = gthree_points_material_real_set_uniforms;

   obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MAP] =
    g_param_spec_object ("map", "Map", "Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SIZE] =
    g_param_spec_float ("size", "Size", "Size",
                        -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_SIZE_ATTENUATION] =
    g_param_spec_boolean ("size-attenuation", "Size attenuation", "Size attenuation",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_points_material_init (GthreePointsMaterial *points_material)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->size = 1;
  priv->size_attenuation = TRUE;

  gthree_material_set_is_transparent (GTHREE_MATERIAL (points_material), TRUE);
}

GthreePointsMaterial *
gthree_points_material_new (void)
{
  return g_object_new (gthree_points_material_get_type (),
                       NULL);
}

const GdkRGBA *
gthree_points_material_get_color (GthreePointsMaterial *points_material)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  return &priv->color;
}

void
gthree_points_material_set_color (GthreePointsMaterial *points_material,
                                  const GdkRGBA           *color)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (points_material), TRUE);

  g_object_notify_by_pspec (G_OBJECT (points_material), obj_props[PROP_COLOR]);
}

void
gthree_points_material_set_map (GthreePointsMaterial *points_material,
                                GthreeTexture *texture)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (points_material), TRUE);

      g_object_notify_by_pspec (G_OBJECT (points_material), obj_props[PROP_MAP]);
    }
}


GthreeTexture  *
gthree_points_material_get_map (GthreePointsMaterial *points_material)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  return priv->map;
}


float
gthree_points_material_get_size (GthreePointsMaterial *points_material)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  return priv->size;
}

void
gthree_points_material_set_size (GthreePointsMaterial *points_material,
                                     float                    size)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  priv->size = size;

  gthree_material_set_needs_update (GTHREE_MATERIAL (points_material), TRUE);
  g_object_notify_by_pspec (G_OBJECT (points_material), obj_props[PROP_SIZE]);
}

gboolean
gthree_points_material_get_size_attenuation (GthreePointsMaterial *points_material)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  return priv->size_attenuation;
}


void
gthree_points_material_set_size_attenuation (GthreePointsMaterial *points_material,
                                             gboolean size_attenuation)
{
  GthreePointsMaterialPrivate *priv = gthree_points_material_get_instance_private (points_material);

  priv->size_attenuation = size_attenuation;

  gthree_material_set_needs_update (GTHREE_MATERIAL (points_material), TRUE);
  g_object_notify_by_pspec (G_OBJECT (points_material), obj_props[PROP_SIZE_ATTENUATION]);
}
