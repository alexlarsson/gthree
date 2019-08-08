#include <math.h>

#include "gthreespritematerial.h"
#include "gthreetypebuiltins.h"
#include "gthreecubetexture.h"

typedef struct {
  GdkRGBA color;
  float rotation;
  GthreeTexture *map;
  gboolean size_attenuation;
} GthreeSpriteMaterialPrivate;

enum {
      PROP_0,

      PROP_COLOR,
      PROP_MAP,
      PROP_ROTATION,
      PROP_SIZE_ATTENUATION,

      N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSpriteMaterial, gthree_sprite_material, GTHREE_TYPE_MATERIAL)

static void
gthree_sprite_material_finalize (GObject *obj)
{
  GthreeSpriteMaterial *sprite_material = GTHREE_SPRITE_MATERIAL (obj);
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  g_clear_object (&priv->map);
  G_OBJECT_CLASS (gthree_sprite_material_parent_class)->finalize (obj);
}

static void
gthree_sprite_material_set_property (GObject *obj,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
  GthreeSpriteMaterial *sprite_material = GTHREE_SPRITE_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_sprite_material_set_color (sprite_material, g_value_get_boxed (value));
      break;

    case PROP_ROTATION:
      gthree_sprite_material_set_rotation (sprite_material, g_value_get_float (value));
      break;

    case PROP_MAP:
      gthree_sprite_material_set_map (sprite_material, g_value_get_object (value));
      break;

    case PROP_SIZE_ATTENUATION:
      gthree_sprite_material_set_size_attenuation (sprite_material, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_sprite_material_get_property (GObject *obj,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
  GthreeSpriteMaterial *sprite_material = GTHREE_SPRITE_MATERIAL (obj);
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_ROTATION:
      g_value_set_float (value, priv->rotation);
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
gthree_sprite_material_real_get_shader (GthreeMaterial *material)
{
  return gthree_clone_shader_from_library ("sprite");
}

static void
gthree_sprite_material_real_set_params (GthreeMaterial *material,
                                        GthreeProgramParameters *params)
{
  GthreeSpriteMaterial *sprite_material = GTHREE_SPRITE_MATERIAL (material);
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  GTHREE_MATERIAL_CLASS (gthree_sprite_material_parent_class)->set_params (material, params);

  params->map = priv->map != NULL;
  if (params->map)
    params->map_encoding = gthree_texture_get_encoding (priv->map);

  params->size_attenuation = priv->size_attenuation;
}

static void
gthree_sprite_material_real_set_uniforms (GthreeMaterial *material,
                                          GthreeUniforms *uniforms,
                                          GthreeCamera   *camera,
                                          GthreeRenderer *renderer)
{
  GthreeSpriteMaterial *sprite_material = GTHREE_SPRITE_MATERIAL (material);
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_sprite_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_color (uni, &priv->color);

  uni = gthree_uniforms_lookup_from_string (uniforms, "rotation");
  if (uni != NULL)
    gthree_uniform_set_float (uni, priv->rotation);

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
gthree_sprite_material_class_init (GthreeSpriteMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_sprite_material_set_property;
  gobject_class->get_property = gthree_sprite_material_get_property;
  gobject_class->finalize = gthree_sprite_material_finalize;
  material_class->get_shader = gthree_sprite_material_real_get_shader;
  material_class->set_params = gthree_sprite_material_real_set_params;
  material_class->set_uniforms = gthree_sprite_material_real_set_uniforms;

   obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MAP] =
    g_param_spec_object ("map", "Map", "Map",
                         GTHREE_TYPE_TEXTURE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_ROTATION] =
    g_param_spec_float ("rotation", "Rotation", "Rotation",
                        -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_SIZE_ATTENUATION] =
    g_param_spec_boolean ("size-attenuation", "Size attenuation", "Size attenuation",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_sprite_material_init (GthreeSpriteMaterial *sprite_material)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->rotation = 0;
  priv->size_attenuation = TRUE;

  gthree_material_set_is_transparent (GTHREE_MATERIAL (sprite_material), TRUE);
}

GthreeSpriteMaterial *
gthree_sprite_material_new (void)
{
  return g_object_new (gthree_sprite_material_get_type (),
                       NULL);
}

const GdkRGBA *
gthree_sprite_material_get_color (GthreeSpriteMaterial *sprite_material)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  return &priv->color;
}

void
gthree_sprite_material_set_color (GthreeSpriteMaterial *sprite_material,
                                  const GdkRGBA           *color)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (sprite_material), TRUE);

  g_object_notify_by_pspec (G_OBJECT (sprite_material), obj_props[PROP_COLOR]);
}

void
gthree_sprite_material_set_map (GthreeSpriteMaterial *sprite_material,
                                GthreeTexture *texture)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  if (g_set_object (&priv->map, texture))
    {
      gthree_material_set_needs_update (GTHREE_MATERIAL (sprite_material), TRUE);

      g_object_notify_by_pspec (G_OBJECT (sprite_material), obj_props[PROP_MAP]);
    }
}


GthreeTexture  *
gthree_sprite_material_get_map (GthreeSpriteMaterial *sprite_material)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  return priv->map;
}


float
gthree_sprite_material_get_rotation (GthreeSpriteMaterial *sprite_material)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  return priv->rotation;
}

void
gthree_sprite_material_set_rotation (GthreeSpriteMaterial *sprite_material,
                                     float                    rotation)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  priv->rotation = rotation;

  gthree_material_set_needs_update (GTHREE_MATERIAL (sprite_material), TRUE);
  g_object_notify_by_pspec (G_OBJECT (sprite_material), obj_props[PROP_ROTATION]);
}

gboolean
gthree_sprite_material_get_size_attenuation (GthreeSpriteMaterial *sprite_material)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  return priv->size_attenuation;
}


void
gthree_sprite_material_set_size_attenuation (GthreeSpriteMaterial *sprite_material,
                                             gboolean size_attenuation)
{
  GthreeSpriteMaterialPrivate *priv = gthree_sprite_material_get_instance_private (sprite_material);

  priv->size_attenuation = size_attenuation;

  gthree_material_set_needs_update (GTHREE_MATERIAL (sprite_material), TRUE);
  g_object_notify_by_pspec (G_OBJECT (sprite_material), obj_props[PROP_SIZE_ATTENUATION]);
}
