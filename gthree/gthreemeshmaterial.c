#include <math.h>

#include "gthreemeshmaterial.h"
#include "gthreetypebuiltins.h"
#include "gthreecubetexture.h"

typedef struct {
  gboolean wireframe;
  float wireframe_line_width;

  gboolean skinning;
  gboolean morph_targets;
  gboolean morph_normals;
} GthreeMeshMaterialPrivate;

enum {
      PROP_0,
      PROP_WIREFRAME,
      PROP_WIREFRAME_LINE_WIDTH,
      PROP_SKINNING,
      PROP_MORPH_TARGETS,
      PROP_MORPH_NORMALS,
      N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeMeshMaterial, gthree_mesh_material, GTHREE_TYPE_MATERIAL)

  static void
  gthree_mesh_material_finalize (GObject *obj)
{
  G_OBJECT_CLASS (gthree_mesh_material_parent_class)->finalize (obj);
}

static void
gthree_mesh_material_set_property (GObject *obj,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  GthreeMeshMaterial *mesh = GTHREE_MESH_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_WIREFRAME:
      gthree_mesh_material_set_is_wireframe (mesh, g_value_get_boolean (value));
      break;

    case PROP_WIREFRAME_LINE_WIDTH:
      gthree_mesh_material_set_wireframe_line_width (mesh, g_value_get_float (value));
      break;

    case PROP_SKINNING:
      gthree_mesh_material_set_skinning (mesh, g_value_get_boolean (value));
      break;

    case PROP_MORPH_TARGETS:
      gthree_mesh_material_set_morph_targets (mesh, g_value_get_boolean (value));
      break;

    case PROP_MORPH_NORMALS:
      gthree_mesh_material_set_morph_normals (mesh, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_material_get_property (GObject *obj,
                                    guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  GthreeMeshMaterial *mesh = GTHREE_MESH_MATERIAL (obj);

  switch (prop_id)
    {
    case PROP_WIREFRAME:
      g_value_set_boolean (value, gthree_mesh_material_get_is_wireframe (mesh));
      break;

    case PROP_WIREFRAME_LINE_WIDTH:
      g_value_set_float (value, gthree_mesh_material_get_wireframe_line_width (mesh));
      break;

    case PROP_SKINNING:
      g_value_set_boolean (value, gthree_mesh_material_get_skinning (mesh));
      break;

    case PROP_MORPH_TARGETS:
      g_value_set_boolean (value, gthree_mesh_material_get_morph_targets (mesh));
      break;

    case PROP_MORPH_NORMALS:
      g_value_set_boolean (value, gthree_mesh_material_get_morph_normals (mesh));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_mesh_material_class_init (GthreeMeshMaterialClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_mesh_material_set_property;
  gobject_class->get_property = gthree_mesh_material_get_property;
  gobject_class->finalize = gthree_mesh_material_finalize;

  obj_props[PROP_WIREFRAME] =
    g_param_spec_boolean ("wireframe", "Wireframe", "Wireframe",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_WIREFRAME_LINE_WIDTH] =
    g_param_spec_float ("wireframe-line-width", "Wireframe line width", "Wireframe line width",
                        0.f, 100.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_SKINNING] =
    g_param_spec_boolean ("skinning", "Skinning", "Skinning",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MORPH_TARGETS] =
    g_param_spec_boolean ("morph-targets", "Morph targets", "Morph targets",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_props[PROP_MORPH_NORMALS] =
    g_param_spec_boolean ("morph-normals", "Morph targets", "Morph targets",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
gthree_mesh_material_init (GthreeMeshMaterial *mesh)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (mesh);

  priv->wireframe = FALSE;
  priv->wireframe_line_width = 1;

  priv->skinning = FALSE;
  priv->morph_targets = FALSE;
  priv->morph_normals = FALSE;

}

gboolean
gthree_mesh_material_get_is_wireframe (GthreeMeshMaterial *material)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  return priv->wireframe;
}

void
gthree_mesh_material_set_is_wireframe (GthreeMeshMaterial *material,
                                       gboolean wireframe)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  wireframe = !!wireframe;

  if (wireframe != priv->wireframe)
    {
      priv->wireframe = wireframe;
      gthree_material_set_needs_update (GTHREE_MATERIAL (material), TRUE);
    }
}

float
gthree_mesh_material_get_wireframe_line_width (GthreeMeshMaterial *material)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  return priv->wireframe_line_width;
}

void
gthree_mesh_material_set_wireframe_line_width (GthreeMeshMaterial *material,
                                               float line_width)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  priv->wireframe_line_width = line_width;
  gthree_material_set_needs_update (GTHREE_MATERIAL (material), TRUE);
}

gboolean
gthree_mesh_material_get_skinning (GthreeMeshMaterial          *material)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  return priv->skinning;
}

void
gthree_mesh_material_set_skinning (GthreeMeshMaterial          *material,
                                   gboolean                     value)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  value = !!value;

  if (value != priv->skinning)
    {
      priv->skinning = value;
      gthree_material_set_needs_update (GTHREE_MATERIAL (material), TRUE);
    }
}

gboolean
gthree_mesh_material_get_morph_targets (GthreeMeshMaterial          *material)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  return priv->morph_targets;
}

void
gthree_mesh_material_set_morph_targets (GthreeMeshMaterial          *material,
                                        gboolean                     value)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  value = !!value;

  if (value != priv->morph_targets)
    {
      priv->morph_targets = value;
      gthree_material_set_needs_update (GTHREE_MATERIAL (material), TRUE);
    }
}

gboolean
gthree_mesh_material_get_morph_normals (GthreeMeshMaterial          *material)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  return priv->morph_normals;
}

void
gthree_mesh_material_set_morph_normals (GthreeMeshMaterial          *material,
                                        gboolean                     value)
{
  GthreeMeshMaterialPrivate *priv = gthree_mesh_material_get_instance_private (material);

  value = !!value;

  if (value != priv->morph_normals)
    {
      priv->morph_normals = value;
      gthree_material_set_needs_update (GTHREE_MATERIAL (material), TRUE);
    }
}
