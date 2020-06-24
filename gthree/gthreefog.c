#include <math.h>
#include <epoxy/gl.h>

#include "gthreefog.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeFogStyle style;
  float density;
  float near;
  float far;
  graphene_vec3_t color;
} GthreeFogPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreeFog, gthree_fog, G_TYPE_OBJECT);

static void
gthree_fog_init (GthreeFog *fog)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->style = GTHREE_FOG_STYLE_LINEAR;
  priv->near = 1;
  priv->far = 1000;
  priv->density = 0.00025;
}

static void
gthree_fog_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_fog_parent_class)->finalize (obj);
}

static void
gthree_fog_class_init (GthreeFogClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_fog_finalize;
}

GthreeFog *
gthree_fog_new_linear (const graphene_vec3_t *color,
                       float near,
                       float far)
{
  GthreeFog *fog = g_object_new (gthree_fog_get_type (), NULL);
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->style = GTHREE_FOG_STYLE_LINEAR;
  priv->color = *color;
  priv->near = near;
  priv->far = far;

  return fog;
}

GthreeFog *
gthree_fog_new_exp2 (const graphene_vec3_t *color,
                     float density)
{
  GthreeFog *fog = g_object_new (gthree_fog_get_type (), NULL);
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->style = GTHREE_FOG_STYLE_EXP2;
  priv->color = *color;
  priv->density = density;

  return fog;
}


GthreeFogStyle
gthree_fog_get_style (GthreeFog *fog)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  return priv->style;
}

void
gthree_fog_set_style (GthreeFog *fog,
                      GthreeFogStyle style)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->style = style;
}

const graphene_vec3_t *
gthree_fog_get_color (GthreeFog             *fog)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  return &priv->color;
}

void
gthree_fog_set_color (GthreeFog             *fog,
                      const graphene_vec3_t *color)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->color = *color;
}


float
gthree_fog_get_near (GthreeFog *fog)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  return priv->near;
}

void
gthree_fog_set_near (GthreeFog *fog,
                     float near)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->near = near;
}

float
gthree_fog_get_far (GthreeFog *fog)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  return priv->far;
}

void
gthree_fog_set_far (GthreeFog *fog,
                    float far)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->far = far;
}

float
gthree_fog_get_density (GthreeFog *fog)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  return priv->density;
}

void
gthree_fog_set_density (GthreeFog *fog,
                        float density)
{
  GthreeFogPrivate *priv = gthree_fog_get_instance_private (fog);

  priv->density = density;
}
