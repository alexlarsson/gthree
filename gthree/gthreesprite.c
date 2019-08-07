#include <math.h>
#include <epoxy/gl.h>

#include "gthreesprite.h"
#include "gthreespritematerial.h"
#include "gthreeobjectprivate.h"
#include "gthreeprivate.h"

typedef struct {
  GthreeGeometry *geometry;
  GthreeMaterial *material;
  graphene_vec2_t center;
} GthreeSpritePrivate;

enum {
  PROP_0,

  PROP_MATERIAL,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GthreeSprite, gthree_sprite, GTHREE_TYPE_OBJECT)

GthreeSprite *
gthree_sprite_new (GthreeMaterial *material)
{
  GthreeSprite *sprite;
  gboolean free_material = FALSE;

  if (material == NULL)
    {
      material = GTHREE_MATERIAL (gthree_sprite_material_new ());
      free_material = TRUE;
    }

  sprite =
    g_object_new (gthree_sprite_get_type (),
                  "material", material,
                  NULL);

  if (free_material)
    g_object_unref (material);

  return sprite;
}

static void
gthree_sprite_init (GthreeSprite *sprite)
{
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  float geometry_data[] = {
    -0.5, -0.5, 0, 0, 0,
     0.5, -0.5, 0, 1, 0,
     0.5,  0.5, 0, 1, 1,
    -0.5,  0.5, 0, 0, 1,
  };
  guint16 index_data[] = {
     0, 1, 2,
     0, 2, 3,
  };
  g_autoptr(GthreeAttributeArray) array = gthree_attribute_array_new_from_float (geometry_data, 4, 5);
  g_autoptr(GthreeAttributeArray) index_array = gthree_attribute_array_new_from_uint16 (index_data, 6, 1);
  g_autoptr(GthreeAttribute) pos =
    gthree_attribute_new_with_array_interleaved ("position",
                                                 array,
                                                 FALSE,
                                                 3, 0, 4);
  g_autoptr(GthreeAttribute) uvs =
    gthree_attribute_new_with_array_interleaved ("uv",
                                                 array,
                                                 FALSE,
                                                 2, 3, 4);
  g_autoptr(GthreeAttribute) index =
    gthree_attribute_new_with_array ("index",
                                     index_array,
                                     FALSE);

  priv->geometry = gthree_geometry_new ();
  gthree_geometry_add_attribute (priv->geometry, "position", pos);
  gthree_geometry_add_attribute (priv->geometry, "uv", uvs);

  gthree_geometry_set_index (priv->geometry, index);

  graphene_vec2_init (&priv->center, 0.5, 0.5);
}

static void
gthree_sprite_finalize (GObject *obj)
{
  GthreeSprite *sprite = GTHREE_SPRITE (obj);
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  g_clear_object (&priv->geometry);
  g_clear_object (&priv->material);

  G_OBJECT_CLASS (gthree_sprite_parent_class)->finalize (obj);
}

static void
gthree_sprite_update (GthreeObject *object)
{
  GthreeSprite *sprite = GTHREE_SPRITE (object);
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  //geometryGroup, customAttributesDirty, material;

  gthree_geometry_update (priv->geometry);

  //material.attributes && clearCustomAttributes( material );
}

static void
gthree_sprite_set_direct_uniforms  (GthreeObject *object,
                                    GthreeProgram *program,
                                    GthreeRenderer *renderer)
{
  GthreeSprite *sprite = GTHREE_SPRITE (object);
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);
  GthreeShader *shader;
  GthreeUniforms *uniforms;
  GthreeUniform *uni;

  shader = gthree_material_get_shader (priv->material);
  uniforms = gthree_shader_get_uniforms (shader);

  uni = gthree_uniforms_lookup_from_string (uniforms, "center");
  if (uni != NULL)
    {
      gthree_uniform_set_vec2 (uni, &priv->center);
      gthree_uniform_load (uni, renderer);
    }

  GTHREE_OBJECT_CLASS (gthree_sprite_parent_class)->set_direct_uniforms (object, program, renderer);
}

static void
gthree_sprite_fill_render_list (GthreeObject   *object,
                                GthreeRenderList *list)
{
  GthreeSprite *sprite = GTHREE_SPRITE (object);
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  gthree_geometry_fill_render_list (priv->geometry, list, priv->material, NULL, object);
}

static gboolean
gthree_sprite_in_frustum (GthreeObject *object,
                        const graphene_frustum_t *frustum)
{
  graphene_sphere_t sphere;
  graphene_point3d_t center;

  graphene_sphere_init (&sphere,
                        graphene_point3d_init (&center, 0, 0,0),
                        0.7071067811865476);

  graphene_matrix_transform_sphere (gthree_object_get_world_matrix (object),
                                    &sphere,
                                    &sphere);

  return graphene_frustum_intersects_sphere (frustum, &sphere);
}

static void
gthree_sprite_set_property (GObject *obj,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
  GthreeSprite *sprite = GTHREE_SPRITE (obj);

  switch (prop_id)
    {
    case PROP_MATERIAL:
      gthree_sprite_set_material (sprite, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

static void
gthree_sprite_get_property (GObject *obj,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
  GthreeSprite *sprite = GTHREE_SPRITE (obj);
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  switch (prop_id)
    {
    case PROP_MATERIAL:
      g_value_set_object (value, priv->material);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    }
}

GthreeMaterial *
gthree_sprite_get_material (GthreeSprite *sprite)
{
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  return priv->material;
}

void
gthree_sprite_set_material (GthreeSprite *sprite,
                            GthreeMaterial *material)
{
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  g_set_object (&priv->material, material);
}

const graphene_vec2_t *
gthree_sprite_get_center (GthreeSprite     *sprite)
{
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  return &priv->center;
}

void
gthree_sprite_set_center (GthreeSprite     *sprite,
                          const graphene_vec2_t *center)
{
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  priv->center = *center;
}


GthreeGeometry *
gthree_sprite_get_geometry (GthreeSprite *sprite)
{
  GthreeSpritePrivate *priv = gthree_sprite_get_instance_private (sprite);

  return priv->geometry;
}

static void
gthree_sprite_class_init (GthreeSpriteClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GthreeObjectClass *object_class = GTHREE_OBJECT_CLASS (klass);

  gobject_class->set_property = gthree_sprite_set_property;
  gobject_class->get_property = gthree_sprite_get_property;
  gobject_class->finalize = gthree_sprite_finalize;

  object_class->in_frustum = gthree_sprite_in_frustum;
  object_class->update = gthree_sprite_update;
  object_class->fill_render_list = gthree_sprite_fill_render_list;
  object_class->set_direct_uniforms = gthree_sprite_set_direct_uniforms;

  obj_props[PROP_MATERIAL] =
    g_param_spec_object ("material", "Material", "Material",
                        GTHREE_TYPE_MATERIAL,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}
