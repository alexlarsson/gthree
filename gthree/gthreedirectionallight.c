#include <math.h>
#include <epoxy/gl.h>

#include "gthreedirectionallight.h"
#include "gthreeprivate.h"

typedef struct {
  float intensity;
  GthreeObject *target;
} GthreeDirectionalLightPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeDirectionalLight, gthree_directional_light, GTHREE_TYPE_LIGHT);

GthreeDirectionalLight *
gthree_directional_light_new (const GdkRGBA *color,
			      float intensity)
{
  GthreeDirectionalLight *light;
  GthreeDirectionalLightPrivate *priv;

  light = g_object_new (gthree_directional_light_get_type (),
			NULL);
  priv = gthree_directional_light_get_instance_private (light);

  gthree_light_set_color (GTHREE_LIGHT (light), color);
  priv->intensity = intensity;

  return light;
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
gthree_directional_light_set_target (GthreeDirectionalLight *directional,
				     GthreeObject *object)
{
  GthreeDirectionalLightPrivate *priv = gthree_directional_light_get_instance_private (directional);

  if (object)
    g_object_ref (object);

  if (priv->target)
    g_object_unref (priv->target);

  priv->target = object;
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
  params->max_dir_lights++;
  
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
gthree_directional_light_class_init (GthreeDirectionalLightClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_directional_light_finalize;
  GTHREE_LIGHT_CLASS(klass)->set_params = gthree_directional_light_real_set_params;
  GTHREE_LIGHT_CLASS(klass)->setup = gthree_directional_light_real_setup;
}
