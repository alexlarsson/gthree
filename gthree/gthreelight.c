#include <math.h>
#include <epoxy/gl.h>

#include "gthreelight.h"

typedef struct {
  GdkRGBA color;
  gboolean visible;
  gboolean only_shadow;
  gboolean casts_shadow;
} GthreeLightPrivate;

enum {
  PROP_0,
  PROP_COLOR,
  PROP_IS_VISIBLE,
  PROP_IS_ONLY_SHADOW,
  PROP_CASTS_SHADOW,
  LAST_PROP
};

static GParamSpec *obj_props[LAST_PROP] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLight, gthree_light, GTHREE_TYPE_OBJECT);

static void
gthree_light_real_set_params (GthreeLight *light,
			      GthreeProgramParameters *params)
{
}

static void
gthree_light_real_setup (GthreeLight *light,
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

    case PROP_IS_VISIBLE:
      gthree_light_set_is_visible (light, g_value_get_boolean (value));
      break;

    case PROP_IS_ONLY_SHADOW:
      gthree_light_set_is_only_shadow (light, g_value_get_boolean (value));
      break;

    case PROP_CASTS_SHADOW:
      gthree_light_set_casts_shadow (light, g_value_get_boolean (value));
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

    case PROP_IS_VISIBLE:
      g_value_set_boolean (value, priv->visible);
      break;

    case PROP_IS_ONLY_SHADOW:
      g_value_set_boolean (value, priv->only_shadow);
      break;

    case PROP_CASTS_SHADOW:
      g_value_set_boolean (value, priv->casts_shadow);
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

  klass->set_params = gthree_light_real_set_params;
  klass->setup = gthree_light_real_setup;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Light color",
                        GDK_TYPE_RGBA,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_IS_VISIBLE] =
    g_param_spec_boolean ("is-visible", "Is Visible", "Is visible",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_IS_ONLY_SHADOW] =
    g_param_spec_boolean ("is-only-shadow", "Is Only Shadow", "Is only shadow",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  obj_props[PROP_CASTS_SHADOW] =
    g_param_spec_boolean ("casts-shadow", "Casts Shadow", "Casts shadow",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, LAST_PROP, obj_props);
}

static void
gthree_light_init (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  priv->color.red = 1.0;
  priv->color.green = 1.0;
  priv->color.blue = 1.0;
  priv->color.alpha = 1.0;

  priv->visible = TRUE;
  priv->only_shadow = FALSE;
  priv->casts_shadow = FALSE;
}

GthreeLight *
gthree_light_new (void)
{
  return g_object_new (GTHREE_TYPE_LIGHT, NULL);
}

gboolean
gthree_light_get_is_only_shadow (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->only_shadow;
}

void
gthree_light_set_is_only_shadow (GthreeLight *light,
                                 gboolean only_shadow)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  only_shadow = !!only_shadow;

  if (priv->only_shadow == only_shadow)
    return;

  priv->only_shadow = only_shadow;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_IS_ONLY_SHADOW]);
}

gboolean
gthree_light_get_is_visible (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->visible;
}

void
gthree_light_set_is_visible (GthreeLight *light,
			     gboolean visible)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  visible = !!visible;

  if (priv->visible == visible)
    return;

  priv->visible = visible;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_IS_VISIBLE]);
}

void
gthree_light_set_casts_shadow (GthreeLight *light,
                               gboolean casts_shadow)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  casts_shadow = !!casts_shadow;

  if (priv->casts_shadow == casts_shadow)
    return;

  priv->casts_shadow = casts_shadow;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_CASTS_SHADOW]);
}

gboolean
gthree_light_get_casts_shadow (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return priv->casts_shadow;
}

void
gthree_light_set_params (GthreeLight             *light,
			 GthreeProgramParameters *params)
{
  GthreeLightClass *class = GTHREE_LIGHT_GET_CLASS(light);

  class->set_params (light, params);
}

const GdkRGBA *
gthree_light_get_color (GthreeLight *light)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  return &priv->color;
}

void
gthree_light_set_color (GthreeLight *light,
			const GdkRGBA *color)
{
  GthreeLightPrivate *priv = gthree_light_get_instance_private (light);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  g_object_notify_by_pspec (G_OBJECT (light), obj_props[PROP_COLOR]);
}

void
gthree_light_setup (GthreeLight *light,
		    GthreeLightSetup *setup)
{
  GthreeLightClass *class = GTHREE_LIGHT_GET_CLASS(light);

  class->setup (light, setup);
}
