#include <math.h>
#include <epoxy/gl.h>
#include <graphene-gobject.h>

#include "gthreelight.h"
#include "gthreeprivate.h"

gboolean
gthree_light_setup_hash_equal (GthreeLightSetupHash *a,
                               GthreeLightSetupHash *b)
{
  return
    a->num_directional == b->num_directional &&
    a->num_point == b->num_point;
}


typedef struct {
  graphene_vec3_t color;
  float   intensity;
} GthreeLightPrivate;

enum {
  PROP_0,
  PROP_COLOR,
  PROP_INTENSITY,
  LAST_PROP
};

static GParamSpec *obj_props[LAST_PROP] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLight, gthree_light, GTHREE_TYPE_OBJECT);

static void
gthree_light_real_setup (GthreeLight *light,
                         GthreeCamera  *camera,
                         GthreeLightSetup *setup)
{
}

static void
gthree_light_set_property (GObject *self,
                           guint prop_id,
                           const GValue *value,
                           GParamSpec *pspec)
{
  GthreeLight *light = GTHREE_LIGHT (self);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_light_set_color (light, g_value_get_boxed (value));
      break;

    case PROP_INTENSITY:
      gthree_light_set_intensity (light, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, prop_id, pspec);
    }
}

static void
gthree_light_get_property (GObject *self,
                           guint prop_id,
                           GValue *value,
                           GParamSpec *pspec)
{
  GthreeLight *light = GTHREE_LIGHT (self);
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_INTENSITY:
      g_value_set_float (value, priv->intensity);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, prop_id, pspec);
    }
}

static void
gthree_light_class_init (GthreeLightClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_light_set_property;
  gobject_class->get_property = gthree_light_get_property;

  klass->setup = gthree_light_real_setup;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Light color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_INTENSITY] =
    g_param_spec_float ("intensity", "Intensity", "Intensity",
                        0.0, G_MAXFLOAT, 1.f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, LAST_PROP, obj_props);
}

static void
gthree_light_init (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  graphene_vec3_init (&priv->color,
                      1.0, 1.0, 1.0);
  priv->intensity = 1.0;
}

GthreeLight *
gthree_light_new (void)
{
  return g_object_new (GTHREE_TYPE_LIGHT, NULL);
}

float
gthree_light_get_intensity (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->intensity;
}

void
gthree_light_set_intensity (GthreeLight *light,
                            float intensity)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  if (priv->intensity == intensity)
    return;

  priv->intensity = intensity;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_INTENSITY]);
}


const graphene_vec3_t *
gthree_light_get_color (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return &priv->color;
}

void
gthree_light_set_color (GthreeLight *light,
                        const graphene_vec3_t *color)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  if (graphene_vec3_equal (color, &priv->color))
    return;

  priv->color = *color;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_COLOR]);
}

/* This is called once per active light in the scene after projection, but before rendering.
 * It needs to set the uniforms for the light, which are later synced to the individual
 * material light uniforms as necessary.
 */
void
gthree_light_setup (GthreeLight *light,
                    GthreeCamera  *camera,
                    GthreeLightSetup *setup)
{
  GthreeLightClass *class = GTHREE_LIGHT_GET_CLASS(light);

  class->setup (light, camera, setup);
}
