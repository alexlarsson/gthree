#include <math.h>
#include <epoxy/gl.h>

#include "gthreedirectionallight.h"
#include "gthreeprivate.h"

typedef struct {
  float intensity;
  GthreeObject *target;
} GthreeDirectionalLightPrivate;

enum {
  PROP_0,

  PROP_INTENSITY,
  PROP_TARGET,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeDirectionalLight, gthree_directional_light, GTHREE_TYPE_LIGHT)

GthreeDirectionalLight *
gthree_directional_light_new (const GdkRGBA *color,
			      float intensity)
{
  return g_object_new (gthree_directional_light_get_type (),
                       "color", color,
                       "intensity", intensity,
                       NULL);
}

static void
gthree_directional_light_init (GthreeDirectionalLight *directional)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);
  graphene_point3d_t pos = {0, 1, 0};
  
  priv->intensity = 1;

  priv->target = g_object_ref_sink (gthree_object_new ());
  
  gthree_object_set_position (GTHREE_OBJECT (directional), &pos);
}

void
gthree_directional_light_set_intensity (GthreeDirectionalLight *directional,
                                        float intensity)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  priv->intensity = intensity;

  g_object_notify_by_pspec (G_OBJECT (directional), obj_props[PROP_INTENSITY]);
}

float
gthree_directional_light_get_intensity (GthreeDirectionalLight *directional)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  return priv->intensity;
}

void
gthree_directional_light_set_target (GthreeDirectionalLight *directional,
				     GthreeObject *object)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  if (object == NULL)
    object = g_object_ref_sink (gthree_object_new ());

  if (g_set_object (&priv->target, object))
    g_object_notify_by_pspec (G_OBJECT (directional), obj_props[PROP_TARGET]);
}

GthreeObject *
gthree_directional_light_get_target (GthreeDirectionalLight *directional)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  return priv->target;
}

static void
gthree_directional_light_finalize (GObject *obj)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (obj);
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  g_clear_object (&priv->target);
  
  G_OBJECT_CLASS (gthree_directional_light_parent_class)->finalize (obj);
}

static void
gthree_directional_light_real_set_params (GthreeLight *light,
					  GthreeProgramParameters *params)
{
  params->num_dir_lights++;
  
  GTHREE_LIGHT_CLASS (gthree_directional_light_parent_class)->set_params (light, params);
}

static void
gthree_directional_light_real_setup (GthreeLight *light,
				     GthreeLightSetup *setup)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (light);
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);
  const GdkRGBA *color = gthree_light_get_color (light);

  setup->dir_count += 1;
  
  if (gthree_light_get_is_visible (light))
    {
      int directionalOffset = setup->dir_len * 3;
      graphene_vec4_t light_pos, target_pos;

      g_array_set_size (setup->dir_colors, directionalOffset + 3);

#if TODO
      if (this.gammaInput)
	setColorGamma(directionalColors, directionalOffset, color, intensity * intensity);
      else
#endif
	{
          g_array_index (setup->dir_colors, float, directionalOffset) =  color->red * priv->intensity;
	  g_array_index (setup->dir_colors, float, directionalOffset+1) = color->green * priv->intensity;
	  g_array_index (setup->dir_colors, float, directionalOffset+2) = color->blue * priv->intensity;
	}

      graphene_matrix_get_row (gthree_object_get_world_matrix (GTHREE_OBJECT (light)), 3, &light_pos);
      graphene_matrix_get_row (gthree_object_get_world_matrix (priv->target), 3, &target_pos);

      graphene_vec4_subtract (&light_pos, &target_pos, &light_pos);
      graphene_vec4_normalize (&light_pos, &light_pos);

      g_array_set_size (setup->dir_positions, directionalOffset + 3);
      g_array_index (setup->dir_positions, float, directionalOffset) = graphene_vec4_get_x (&light_pos);
      g_array_index (setup->dir_positions, float, directionalOffset+1) = graphene_vec4_get_y (&light_pos);
      g_array_index (setup->dir_positions, float, directionalOffset+2) = graphene_vec4_get_z (&light_pos);

      setup->dir_len += 1;
    }
  
  GTHREE_LIGHT_CLASS (gthree_directional_light_parent_class)->setup (light, setup);
}

static void
gthree_directional_light_set_property (GObject *obj,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (obj);

  switch (prop_id)
    {
    case PROP_INTENSITY:
      gthree_directional_light_set_intensity (directional, g_value_get_float (value));
      break;

    case PROP_TARGET:
      gthree_directional_light_set_target (directional, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_directional_light_get_property (GObject *obj,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
  GthreeDirectionalLight *directional = GTHREE_DIRECTIONAL_LIGHT (obj);
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  switch (prop_id)
    {
    case PROP_INTENSITY:
      g_value_set_float (value, priv->intensity);
      break;

    case PROP_TARGET:
      g_value_set_object (value, priv->target);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_directional_light_class_init (GthreeDirectionalLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeLightClass *light_class = GTHREE_LIGHT_CLASS (klass);

  gobject_class->set_property = gthree_directional_light_set_property;
  gobject_class->get_property = gthree_directional_light_get_property;
  gobject_class->finalize = gthree_directional_light_finalize;

  light_class->set_params = gthree_directional_light_real_set_params;
  light_class->setup = gthree_directional_light_real_setup;

  obj_props[PROP_INTENSITY] =
    g_param_spec_float ("intensity", "Intensity", "Intensity",
                        -G_MAXFLOAT, G_MAXFLOAT, 1.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_TARGET] =
    g_param_spec_object ("target", "Target", "Target",
                         GTHREE_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
