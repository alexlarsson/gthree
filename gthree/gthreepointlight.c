#include <math.h>
#include <epoxy/gl.h>

#include "gthreepointlight.h"
#include "gthreeprivate.h"

typedef struct {
  float intensity;
  float distance;
} GthreePointLightPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreePointLight, gthree_point_light, GTHREE_TYPE_LIGHT);

GthreePointLight *
gthree_point_light_new (const GdkRGBA *color,
			float intensity,
			float distance)
{
  GthreePointLight *light;
  GthreePointLightPrivate *priv;

  light = g_object_new (gthree_point_light_get_type (),
                           NULL);
  priv = gthree_point_light_get_instance_private (light);

  gthree_light_set_color (GTHREE_LIGHT (light), color);
  priv->intensity = intensity;
  priv->distance = distance;

  return light;
}

static void
gthree_point_light_init (GthreePointLight *point)
{
  GthreePointLightPrivate *priv = gthree_point_light_get_instance_private (point);

  priv->intensity = 1;
  priv->distance = 0;
}

static void
gthree_point_light_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_point_light_parent_class)->finalize (obj);
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
gthree_point_light_class_init (GthreePointLightClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_point_light_finalize;
  GTHREE_LIGHT_CLASS(klass)->set_params = gthree_point_light_real_set_params;
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_point_light_real_setup;
}
