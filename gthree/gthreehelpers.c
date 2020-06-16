#include <graphene-gobject.h>

#include "gthreehelpers.h"
#include "gthreeattribute.h"
#include "gthreegeometry.h"
#include "gthreeline.h"
#include "gthreemesh.h"
#include "gthreelinebasicmaterial.h"
#include "gthreemeshbasicmaterial.h"

struct _GthreePlaneHelperClass {
  GthreeObjectClass parent_class;
} ;


struct _GthreePlaneHelper {
  GthreeObject parent;

  graphene_plane_t plane;
  float size;
  graphene_vec3_t color;
  GthreeLineBasicMaterial *line_material;
  GthreeMeshBasicMaterial *mesh_material;
};

G_DEFINE_TYPE (GthreePlaneHelper, gthree_plane_helper, GTHREE_TYPE_OBJECT)

enum {
  PLANE_PROP_0,

  PLANE_PROP_PLANE,
  PLANE_PROP_SIZE,
  PLANE_PROP_COLOR,

  PLANE_N_PROPS
};

static GParamSpec *plane_props[PLANE_N_PROPS] = { NULL, };

static void
gthree_plane_helper_set_property (GObject *obj,
                                  guint prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
  GthreePlaneHelper *helper = GTHREE_PLANE_HELPER (obj);

  switch (prop_id)
    {
    case PLANE_PROP_COLOR:
      gthree_plane_helper_set_color (helper, g_value_get_boxed (value));
      break;

    case PLANE_PROP_PLANE:
      gthree_plane_helper_set_plane (helper, g_value_get_boxed (value));
      break;

    case PLANE_PROP_SIZE:
      gthree_plane_helper_set_size (helper, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_plane_helper_get_property (GObject *obj,
                                  guint prop_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
  GthreePlaneHelper *helper = GTHREE_PLANE_HELPER (obj);

  switch (prop_id)
    {
    case PLANE_PROP_COLOR:
      g_value_set_boxed (value, &helper->color);
      break;

    case PLANE_PROP_PLANE:
      g_value_set_boxed (value, &helper->plane);
      break;

    case PLANE_PROP_SIZE:
      g_value_set_float (value, helper->size);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_plane_helper_finalize (GObject *obj)
{
  GthreePlaneHelper *helper = GTHREE_PLANE_HELPER (obj);

  g_object_unref (helper->line_material);
  g_object_unref (helper->mesh_material);

  G_OBJECT_CLASS (gthree_plane_helper_parent_class)->finalize (obj);
}

static gboolean
gthree_plane_helper_update_matrix_world (GthreeObject *object,
                                         gboolean force)
{
  GthreePlaneHelper *helper = GTHREE_PLANE_HELPER (object);
  graphene_vec3_t normal;
  float scale;

  scale = graphene_plane_get_constant (&helper->plane);
  graphene_plane_get_normal (&helper->plane, &normal);

  if (fabs(scale) < 1e-8)
    scale = 1e-8; // sign does not matter

  gthree_object_set_scale_xyz (object, 0.5 * helper->size, 0.5 * helper->size, scale);

  // TODO: For some reason three.js instead does
  // ( scale < 0 ) ? BackSide : FrontSide
  // And that gets the same results as the below (in e.g. the clipping example. I have no idea why...
  // Three.js has this comment: "renderer flips side when determinant < 0; flipping not wanted here"
  gthree_material_set_side (GTHREE_MATERIAL (helper->mesh_material), GTHREE_SIDE_BACK);

  gthree_object_look_at (object, &normal);

  return GTHREE_OBJECT_CLASS (gthree_plane_helper_parent_class)->update_matrix_world (object, force);
}

static void
gthree_plane_helper_class_init (GthreePlaneHelperClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->finalize = gthree_plane_helper_finalize;
  gobject_class->set_property = gthree_plane_helper_set_property;
  gobject_class->get_property = gthree_plane_helper_get_property;

  object_class->update_matrix_world = gthree_plane_helper_update_matrix_world;

  plane_props[PLANE_PROP_COLOR] =
    g_param_spec_boxed ("color", "Color", "Color",
                        GRAPHENE_TYPE_VEC3,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  plane_props[PLANE_PROP_PLANE] =
    g_param_spec_boxed ("plane", "Plane", "Plane",
                        GRAPHENE_TYPE_PLANE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  plane_props[PLANE_PROP_SIZE] =
    g_param_spec_float ("size", "Size", "Size",
                        0.f, 100000000.f, 1.0f,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, PLANE_N_PROPS, plane_props);
}

static void
gthree_plane_helper_init (GthreePlaneHelper *helper)
{
  g_autoptr(GthreeAttribute) line_position = NULL;
  g_autoptr(GthreeAttribute) mesh_position = NULL;
  g_autoptr(GthreeGeometry) line_geometry = NULL;
  g_autoptr(GthreeGeometry) mesh_geometry = NULL;
  g_autoptr(GthreeLine) line = NULL;
  g_autoptr(GthreeMesh) mesh = NULL;
  static float line_data[] = { 1, - 1, 1, - 1, 1, 1, - 1, - 1, 1, 1, 1, 1, - 1, 1, 1, - 1, - 1, 1, 1, - 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 };
  static float mesh_data[] = { 1, 1, 1, - 1, 1, 1, - 1, - 1, 1, 1, 1, 1, - 1, - 1, 1, 1, - 1, 1 };

  graphene_plane_init (&helper->plane, graphene_vec3_x_axis (), 0.0);

  line_geometry = gthree_geometry_new ();

  line_position = gthree_attribute_new_from_float ("position", line_data, G_N_ELEMENTS (line_data) / 3, 3);
  gthree_geometry_add_attribute (line_geometry, "position", line_position);

  helper->line_material = gthree_line_basic_material_new ();

 line = gthree_line_new (line_geometry, GTHREE_MATERIAL (helper->line_material));
 gthree_object_add_child (GTHREE_OBJECT (helper), GTHREE_OBJECT (line));

  mesh_geometry = gthree_geometry_new ();

  mesh_position = gthree_attribute_new_from_float ("position", mesh_data, G_N_ELEMENTS (mesh_data) / 3, 3);
  gthree_geometry_add_attribute (mesh_geometry, "position", mesh_position);

  helper->mesh_material = gthree_mesh_basic_material_new ();
  gthree_material_set_opacity (GTHREE_MATERIAL (helper->mesh_material), 0.2);
  gthree_material_set_is_transparent (GTHREE_MATERIAL (helper->mesh_material), TRUE);

  gthree_material_set_depth_write  (GTHREE_MATERIAL (helper->mesh_material), TRUE); // TODO: Should be FALSE

  mesh = gthree_mesh_new (mesh_geometry, GTHREE_MATERIAL (helper->mesh_material));
  gthree_object_add_child (GTHREE_OBJECT (helper), GTHREE_OBJECT (mesh));
}

GthreePlaneHelper *
gthree_plane_helper_new (const graphene_plane_t *plane,
                         float size,
                         const graphene_vec3_t *color)
{
  return g_object_new (GTHREE_TYPE_PLANE_HELPER,
                       "plane", plane,
                       "size", size,
                       "color", color,
                       NULL);
}

void
gthree_plane_helper_set_plane (GthreePlaneHelper *helper,
                               const graphene_plane_t *plane)
{
  helper->plane = *plane;
}

const graphene_plane_t *
gthree_plane_helper_get_plane (GthreePlaneHelper *helper)
{
  return &helper->plane;
}

void
gthree_plane_helper_set_color (GthreePlaneHelper *helper,
                               const graphene_vec3_t *color)
{
  helper->color = *color;

  gthree_line_basic_material_set_color (helper->line_material, &helper->color);
  gthree_mesh_basic_material_set_color (helper->mesh_material, &helper->color);
}


const graphene_vec3_t *
gthree_plane_helper_get_color (GthreePlaneHelper *helper)
{
  return &helper->color;
}

void
gthree_plane_helper_set_size (GthreePlaneHelper *helper,
                              float size)
{
  helper->size = size;
}

float
gthree_plane_helper_get_size (GthreePlaneHelper *helper)
{
  return helper->size;
}
