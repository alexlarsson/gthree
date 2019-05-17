#include <math.h>
#include <epoxy/gl.h>

#include "gthreepointlight.h"
#include "gthreeprivate.h"

typedef struct {
  float intensity;
  float distance;
} GthreePointLightPrivate;

enum {
  PROP_0,

  PROP_INTENSITY,
  PROP_DISTANCE,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreePointLight, gthree_point_light, GTHREE_TYPE_LIGHT)

GthreePointLight *
gthree_point_light_new (const GdkRGBA *color,
                        float intensity,
                        float distance)
{
  return g_object_new (gthree_point_light_get_type (),
                       "color", color,
                       "intensity", intensity,
                       "distance", distance,
                       NULL);
}

static void
gthree_point_light_init (GthreePointLight *point)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);

  priv->intensity = 1;
  priv->distance = 0;
}

static void
gthree_point_light_real_set_params (GthreeLight *light,
                                    GthreeProgramParameters *params)
{
  params->max_point_lights++;

  GTHREE_LIGHT_CLASS (gthree_point_light_parent_class)->set_params (light, params);
}

static void
gthree_point_light_real_setup (GthreeLight *light,
                               GthreeLightSetup *setup)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (light);
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);
  const GdkRGBA *color = gthree_light_get_color (light);

  setup->point_count += 1;

  if (gthree_light_get_is_visible (light))
    {
      int pointOffset = setup->point_len * 3;
      graphene_vec4_t pos;

      g_array_set_size (setup->point_colors, pointOffset + 3);

#if TODO
      if (this.gammaInput)
        setColorGamma(pointColors, pointOffset, color, intensity * intensity);
      else
#endif
        {
          g_array_index (setup->point_colors, float, pointOffset) =  color->red * priv->intensity;
          g_array_index (setup->point_colors, float, pointOffset+1) = color->green * priv->intensity;
          g_array_index (setup->point_colors, float, pointOffset+2) = color->blue * priv->intensity;
        }

      graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)),
                               3, &pos);

      g_array_set_size (setup->point_positions, pointOffset + 3);
      g_array_index (setup->point_positions, float, pointOffset) = graphene_vec4_get_x (&pos);
      g_array_index (setup->point_positions, float, pointOffset+1) = graphene_vec4_get_y (&pos);
      g_array_index (setup->point_positions, float, pointOffset+2) = graphene_vec4_get_z (&pos);

      g_array_set_size (setup->point_distances, setup->point_len + 1);
      g_array_index (setup->point_distances, float, setup->point_len) = priv->distance;

      setup->point_len += 1;
    }

  GTHREE_LIGHT_CLASS (gthree_point_light_parent_class)->setup (light, setup);
}

static void
gthree_point_light_set_property (GObject *obj,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (obj);

  switch (prop_id)
    {
    case PROP_INTENSITY:
      gthree_point_light_set_intensity (point, g_value_get_float (value));
      break;

    case PROP_DISTANCE:
      gthree_point_light_set_distance (point, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_point_light_get_property (GObject *obj,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  GthreePointLight *point = GTHREE_POINT_LIGHT (obj);
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);

  switch (prop_id)
    {
    case PROP_INTENSITY:
      g_value_set_float (value, priv->intensity);
      break;

    case PROP_DISTANCE:
      g_value_set_float (value, priv->distance);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_point_light_class_init (GthreePointLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeLightClass *light_class = GTHREE_LIGHT_CLASS (klass);

  gobject_class->set_property = gthree_point_light_set_property;
  gobject_class->get_property = gthree_point_light_get_property;

  light_class->set_params = gthree_point_light_real_set_params;
  light_class->setup = gthree_point_light_real_setup;

  obj_props[PROP_INTENSITY] =
    g_param_spec_float ("intensity", "Intensity", "Intensity",
                        -G_MAXFLOAT, G_MAXFLOAT, 1.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_DISTANCE] =
    g_param_spec_float ("distance", "Distance", "Distance",
                        -G_MAXFLOAT, G_MAXFLOAT, 0.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

void
gthree_point_light_set_intensity (GthreePointLight *light,
                                  float intensity)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  priv->intensity = intensity;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_INTENSITY]);
}

float
gthree_point_light_get_intensity (GthreePointLight *light)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  return priv->intensity;
}

void
gthree_point_light_set_distance (GthreePointLight *light,
                                 float distance)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  priv->distance = distance;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_DISTANCE]);
}

float
gthree_point_light_get_distance (GthreePointLight *light)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (light);

  return priv->distance;
}
