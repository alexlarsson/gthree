#include <math.h>
#include <graphene-gobject.h>

#include "gthreelinebasicmaterial.h"
#include "gthreetypebuiltins.h"

typedef struct {
  graphene_vec3_t color;
  float line_width;
} GthreeLineBasicMaterialPrivate;

enum {
  PROP_0,

  PROP_LINE_WIDTH,
  PROP_COLOR,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeLineBasicMaterial, gthree_line_basic_material, GTHREE_TYPE_MATERIAL)

static void
gthree_line_basic_material_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_line_basic_material_parent_class)->finalize (obj);
}

static void
gthree_line_basic_material_real_set_params (GthreeMaterial *material,
                                       GthreeProgramParameters *params)
{
  GTHREE_MATERIAL_CLASS (gthree_line_basic_material_parent_class)->set_params (material, params);
}

static void
gthree_line_basic_material_real_set_uniforms (GthreeMaterial *material,
                                              GthreeUniforms *uniforms,
                                              GthreeCamera   *camera,
                                              GthreeRenderer *renderer)
{
  GthreeLineBasicMaterial *line_basic = GTHREE_LINE_BASIC_MATERIAL (material);
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);
  GthreeUniform *uni;

  GTHREE_MATERIAL_CLASS (gthree_line_basic_material_parent_class)->set_uniforms (material, uniforms, camera, renderer);

  uni = gthree_uniforms_lookup_from_string (uniforms, "diffuse");
  if (uni != NULL)
    gthree_uniform_set_vec3 (uni, &priv->color);
}

static gboolean
gthree_line_basic_material_needs_camera_pos (GthreeMaterial *material)
{
  return FALSE;
}

static void
gthree_line_basic_material_set_property (GObject *obj,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
  GthreeLineBasicMaterial *line_basic = GTHREE_LINE_BASIC_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_COLOR:
      gthree_line_basic_material_set_color (line_basic, g_value_get_boxed (value));
      break;

    case PROP_LINE_WIDTH:
      gthree_line_basic_material_set_line_width (line_basic, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_line_basic_material_get_property (GObject *obj,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
  GthreeLineBasicMaterial *line_basic = GTHREE_LINE_BASIC_MATERIAL (obj);
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);

  switch (prop_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &priv->color);
      break;

    case PROP_LINE_WIDTH:
      g_value_set_float (value, priv->line_width);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_line_basic_material_class_init (GthreeLineBasicMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeMaterialClass *material_class = GTHREE_MATERIAL_CLASS (klass);

  gobject_class->set_property = gthree_line_basic_material_set_property;
  gobject_class->get_property = gthree_line_basic_material_get_property;
  gobject_class->finalize = gthree_line_basic_material_finalize;

  material_class->set_params = gthree_line_basic_material_real_set_params;
  material_class->set_uniforms = gthree_line_basic_material_real_set_uniforms;
  material_class->needs_camera_pos = gthree_line_basic_material_needs_camera_pos;

  obj_props[PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_LINE_WIDTH] =
    g_param_spec_float ("line-width", "Line width", "Line width",
                        0.f, 100.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_line_basic_material_init (GthreeLineBasicMaterial *line_basic)
{
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);

  graphene_vec3_init (&priv->color,
                      1.0, 1.0, 1.0);

  priv->line_width = 1.0;
}

GthreeLineBasicMaterial *
gthree_line_basic_material_new (void)
{
  return g_object_new (gthree_line_basic_material_get_type (), NULL);
}

const graphene_vec3_t *
gthree_line_basic_material_get_color (GthreeLineBasicMaterial *line_basic)
{
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);

  return &priv->color;
}

void
gthree_line_basic_material_set_color (GthreeLineBasicMaterial *line_basic,
                                 const graphene_vec3_t *color)
{
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);

  if (gdk_rgba_equal (color, &priv->color))
    return;

  priv->color = *color;

  gthree_material_set_needs_update (GTHREE_MATERIAL (line_basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (line_basic), obj_props[PROP_COLOR]);
}

float
gthree_line_basic_material_get_line_width (GthreeLineBasicMaterial *line_basic)
{
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);

  return priv->line_width;
}

void
gthree_line_basic_material_set_line_width (GthreeLineBasicMaterial *line_basic,
                                           float line_width)
{
  GthreeLineBasicMaterialPrivate *priv = gthree_line_basic_material_get_instance_private (line_basic);

  if (priv->line_width == line_width)
    return;

  priv->line_width = line_width;

  gthree_material_set_needs_update (GTHREE_MATERIAL (line_basic), TRUE);

  g_object_notify_by_pspec (G_OBJECT (line_basic), obj_props[PROP_COLOR]);
}
