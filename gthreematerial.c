#include <math.h>
#include <epoxy/gl.h>

#include "gthreematerial.h"

typedef struct {
  gboolean transparent;
} GthreeMaterialPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (GthreeMaterial, gthree_material, G_TYPE_OBJECT);

GthreeMaterial *
gthree_material_new ()
{
  GthreeMaterial *material;

  material = g_object_new (gthree_material_get_type (),
                         NULL);


  return material;
}

static void
gthree_material_init (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  material->needs_update = TRUE;
  priv->transparent = FALSE;
}

static void
gthree_material_finalize (GObject *obj)
{
  GthreeMaterial *material = GTHREE_MATERIAL (obj);
  //GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  g_clear_object (&material->program);
  g_clear_object (&material->shader);

  G_OBJECT_CLASS (gthree_material_parent_class)->finalize (obj);
}

static void
gthree_material_class_init (GthreeMaterialClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_material_finalize;
}

gboolean
gthree_material_get_is_visible (GthreeMaterial *material)
{
  return TRUE;
}

gboolean
gthree_material_get_is_wireframe (GthreeMaterial *material)
{
  return FALSE;
}

float
gthree_material_get_wireframe_line_width (GthreeMaterial *material)
{
  return 1.0;
}

gboolean
gthree_material_get_is_transparent (GthreeMaterial *material)
{
  GthreeMaterialPrivate *priv = gthree_material_get_instance_private (material);

  return priv->transparent;
}

GthreeBlendMode
gthree_material_get_blend_mode (GthreeMaterial *material,
                                GthreeBlendEquation *equation,
                                GthreeBlendSrcFactor *src_factor,
                                GthreeBlendDstFactor *dst_factor)
{
  if (equation)
    *equation = GTHREE_BLEND_EQUATION_ADD;
  if (src_factor)
    *src_factor = GTHREE_BLEND_SRC_FACTOR_SRC_COLOR;
  if (dst_factor)
    *dst_factor = GTHREE_BLEND_DST_FACTOR_COLOR;

  return GTHREE_BLEND_NO;
}

gboolean
gthree_material_get_polygon_offset (GthreeMaterial *material,
                                    float *factor, float *units)
{
  if (factor)
    *factor = 1.0;

  if (units)
    *units = 1.0;

  return FALSE;
}

gboolean
gthree_material_get_depth_test (GthreeMaterial *material)
{
  return TRUE;
}

gboolean
gthree_material_get_depth_write (GthreeMaterial *material)
{
  return FALSE;
}

GthreeSide
gthree_material_get_side (GthreeMaterial *material)
{
  return GTHREE_SIDE_DOUBLE;
}

GthreeShader *
gthree_material_get_shader (GthreeMaterial *material)
{
  if (material->shader == NULL)
    material->shader = gthree_get_shader_from_library ("basic");

  return material->shader;
}
