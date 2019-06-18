#include <math.h>

#include "gthreepropertybindingprivate.h"
#include "gthreeskinnedmesh.h"

GthreeParsedPath *
gthree_parsed_path_dup (GthreeParsedPath *path)
{
  return gthree_parsed_path_new (path->node_name,
                                 path->object_name,
                                 path->object_index,
                                 path->property_name,
                                 path->property_index);
}

GthreeParsedPath *
gthree_parsed_path_new (const char *node_name,
                        const char *object_name,
                        const char *object_index,
                        const char *property_name,
                        const char *property_index)
{
  GthreeParsedPath *path = g_new0 (GthreeParsedPath, 1);
  path->node_name = g_strdup (node_name);
  path->object_name = g_strdup (object_name);
  path->object_index = g_strdup (object_index);
  path->property_name = g_strdup (property_name);
  path->property_index = g_strdup (property_index);
  return path;
}

void
gthree_parsed_path_free (GthreeParsedPath *path)
{
  g_free (path->node_name);
  g_free (path->object_name);
  g_free (path->object_index);
  g_free (path->property_name);
  g_free (path->property_index);
  g_free (path);
}

#define RESERVED_CHARS_RE "\\[\\]\\.:\\/"
#define RESERVED_CHARS_NO_DOT_RE "\\[\\]:\\/"
#define WORD_CHAR  "[^" RESERVED_CHARS_RE "]"
#define WORD_CHAR_OR_DOT "[^" RESERVED_CHARS_NO_DOT_RE "']"

// Parent directories, delimited by '/' or ':'. Currently unused, but must
// be matched to parse the rest of the track name.
#define DIRECTORY_RE "((?:" WORD_CHAR "+[\\/:])*)"

// Target node. May contain word characters (a-zA-Z0-9_) and '.' or '-'.
#define NODE_RE "(" WORD_CHAR_OR_DOT "+)?"

// Object on target node, and accessor. May not contain reserved
// characters. Accessor may contain any character except closing bracket.
#define OBJECT_RE "(?:\\.(" WORD_CHAR "+)(?:\\[(.+)\\])?)?"

// Property and accessor. May not contain reserved characters. Accessor may
// contain any non-bracket characters.
#define PROPERTY_RE "\\.(" WORD_CHAR "+)(?:\\[(.+)\\])?"

#define PATH_RE "^" DIRECTORY_RE NODE_RE OBJECT_RE PROPERTY_RE "$"

GthreeParsedPath *
gthree_parsed_path_parse (const char *name)
{
  static GRegex *track_re = NULL;
  const char *supported_object_names[] = { "material", "materials", "bones", NULL };
  g_autoptr(GMatchInfo) match_info = NULL;
  g_autoptr(GthreeParsedPath) path = NULL;
  char *last_dot;

  if (track_re == NULL)
    {
      track_re = g_regex_new (PATH_RE, G_REGEX_OPTIMIZE, 0, NULL);
      g_assert (track_re != NULL);
    }

  if (!g_regex_match (track_re, name, 0, &match_info))
    {
      g_warning ("Can't match property path %s", name);
      return NULL;
    }

  path = g_new0 (GthreeParsedPath, 1);

  // g_match_info_fetch returns an empty string if no match, we then NULL it instead below

  //path->directory_name = g_match_info_fetch (match_info, 1); currently unused
  path->node_name = g_match_info_fetch (match_info, 2);
  if (path->node_name && path->node_name[0] == 0)
    g_clear_pointer (&path->node_name, g_free);
  path->object_name = g_match_info_fetch (match_info, 3);
  if (path->object_name && path->object_name[0] == 0)
    g_clear_pointer (&path->object_name, g_free);
  path->object_index = g_match_info_fetch (match_info, 4);
  if (path->object_index && path->object_index[0] == 0)
    g_clear_pointer (&path->object_index, g_free);
  path->property_name = g_match_info_fetch (match_info, 5); // required
  if (path->property_name && path->property_name[0] == 0)
    g_clear_pointer (&path->property_name, g_free);
  path->property_index = g_match_info_fetch (match_info, 6);
  if (path->property_index && path->property_index[0] == 0)
    g_clear_pointer (&path->property_index, g_free);

  if (path->node_name)
    last_dot = strrchr (path->node_name, '.');
  else
    last_dot = NULL;

  if (last_dot)
    {
      const char *object_name = last_dot + 1;

      // Object names must be checked against a whitelist. Otherwise, there
      // is no way to parse 'foo.bar.baz': 'baz' must be a property, but
      // 'bar' could be the objectName, or part of a nodeName (which can
      // include '.' characters).
      if (g_strv_contains (supported_object_names, object_name))
        {
          g_free (path->object_name);
          path->object_name = g_strdup (object_name);
          *last_dot = 0; // shorten node_name at last_dot
        }
    }

  if (path->property_name == NULL || path->property_name[0] == 0)
    {
      g_warning ("can not parse property name from track name: %s", name);
      return NULL;
    }

  return g_steal_pointer (&path);
}

typedef gpointer (*GthreePropertyGetter)(gpointer object);
typedef void (*GthreePropertyGetValue) (GthreePropertyBinding *binding,
                                        gpointer property,
                                        int property_index,
                                        float *buffer,
                                        int offset);
typedef void (*GthreePropertySetter) (gpointer object,
                                      gpointer property);
typedef void (*GthreePropertySetValue) (GthreePropertyBinding *binding,
                                        float *buffer,
                                        int offset);


typedef struct {
  char *path;
  GthreeParsedPath *parsed_path;
  GthreeObject *node;
  GthreeObject *root;

  gboolean bound;
  gboolean bound_ok;

  int resolved_prop_index;
  GObject *resolved_object;
  GthreePropertyGetter get_property;
  GthreePropertyGetValue get_value;
  GthreePropertySetter set_property;
  GthreePropertySetValue set_value;

} GthreePropertyBindingPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GthreePropertyBinding, gthree_property_binding, G_TYPE_OBJECT)

static void
gthree_property_binding_init (GthreePropertyBinding *binding)
{
  //GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
}

static void
gthree_property_binding_finalize (GObject *obj)
{
  GthreePropertyBinding *binding = GTHREE_PROPERTY_BINDING (obj);
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);

  g_free (priv->path);
  gthree_parsed_path_free (priv->parsed_path);

  g_clear_object (&priv->node);
  g_clear_object (&priv->root);

  g_clear_object (&priv->resolved_object);

  G_OBJECT_CLASS (gthree_property_binding_parent_class)->finalize (obj);
}

static void
gthree_property_binding_class_init (GthreePropertyBindingClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = gthree_property_binding_finalize;
}

static GthreeObject *
search_node_subtree (GthreeObject *object, const char *node_name)
{
  GthreeObjectIter iter;
  GthreeObject *child, *found;

  gthree_object_iter_init (&iter, object);
  while (gthree_object_iter_next (&iter, &child))
    {
      if (g_strcmp0 (node_name, gthree_object_get_name (child)) ||
          g_strcmp0 (node_name, gthree_object_get_uuid (child)))
        return child;

      found = search_node_subtree (child, node_name);
      if (found)
        return found;
    }

  return NULL;
}

static GthreeObject *
ghtree_property_binding_find_node (GthreeObject *root, const char *node_name)
{
  if ( node_name == NULL ||
       node_name[0] == 0 ||
       strcmp (node_name, "root") == 0 ||
       strcmp (node_name, ".") == 0 ||
       strcmp (node_name, "-1") == 0 ||
       g_strcmp0 (node_name, gthree_object_get_name (root)) ||
       g_strcmp0 (node_name, gthree_object_get_uuid (root)))
    return root;

  // search into skeleton bones.
  if (GTHREE_IS_SKINNED_MESH (root))
    {
      GthreeSkeleton *skeleton = gthree_skinned_mesh_get_skeleton (GTHREE_SKINNED_MESH (root));
      GthreeBone *bone = gthree_skeleton_get_bone_by_name (skeleton, node_name);
      if (bone)
        return GTHREE_OBJECT (bone);
    }

  return search_node_subtree (root, node_name);
}

GthreePropertyBinding *
gthree_property_binding_new (GthreeObject *root, const char *path, GthreeParsedPath *parsed_path)
{
  GthreePropertyBinding *binding;
  GthreePropertyBindingPrivate *priv;
  GthreeObject *node;
#ifdef TODO
  if (root != NULL && GTHREE_IS_ANIMATION_OBJECT_GROUP (root))
    {
      //return new PropertyBinding.Composite( root, path, parsedPath );
    }
  else
#endif
    {
      binding = g_object_new (GTHREE_TYPE_PROPERTY_BINDING, NULL);
      priv = gthree_property_binding_get_instance_private (binding);

      priv->path = g_strdup (path);
      if (parsed_path)
        priv->parsed_path = gthree_parsed_path_dup (parsed_path);
      else
        priv->parsed_path = gthree_parsed_path_parse (path);

      node = ghtree_property_binding_find_node (root, priv->parsed_path->node_name);
      if (priv->node == NULL)
        node = root;
      priv->node = g_object_ref (node);
      priv->root  = g_object_ref (root);
    }

  return binding;
}

char *
ghtree_property_sanitize_name (const char *name)
{
  GString *s = g_string_new ("");

  while (*name)
    {
      char c = *name++;
      if (c != '[' && c != ']' &&
          c != '.' && c != ':' &&
          c != '/')
        g_string_append_c (s, c);
    }

  return g_string_free (s, FALSE);
}

GthreeParsedPath *
gthree_property_binding_get_parsed_path (GthreePropertyBinding *binding)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
  return priv->parsed_path;
}

void
ghtree_property_binding_get_value_vec3 (GthreePropertyBinding *binding,
                                        gpointer property,
                                        int property_index,
                                        float *buffer,
                                        int offset)
{
  const graphene_vec3_t *vec = property;

  switch (property_index) {
  case -1:
    buffer[offset+0] = graphene_vec3_get_x (vec);
    buffer[offset+1] = graphene_vec3_get_y (vec);
    buffer[offset+2] = graphene_vec3_get_z (vec);
    break;
  case 0:
    buffer[offset] = graphene_vec3_get_x (vec);
    break;
  case 1:
    buffer[offset] = graphene_vec3_get_y (vec);
    break;
  case 2:
    buffer[offset] = graphene_vec3_get_z (vec);
    break;
  }
}

void
ghtree_property_binding_get_value_euler (GthreePropertyBinding *binding,
                                         gpointer property,
                                         int property_index,
                                         float *buffer,
                                         int offset)
{
  const graphene_euler_t *e = property;

  switch (property_index) {
  case -1:
    buffer[offset+0] = graphene_euler_get_x (e);
    buffer[offset+1] = graphene_euler_get_y (e);
    buffer[offset+2] = graphene_euler_get_z (e);
    break;
  case 0:
    buffer[offset] = graphene_euler_get_x (e);
    break;
  case 1:
    buffer[offset] = graphene_euler_get_y (e);
    break;
  case 2:
    buffer[offset] = graphene_euler_get_z (e);
    break;
  }
}


void
ghtree_property_binding_get_value_quaternion (GthreePropertyBinding *binding,
                                              gpointer property,
                                              int property_index,
                                              float *buffer,
                                              int offset)
{
  const graphene_quaternion_t *q = property;
  graphene_vec4_t vec;

  graphene_quaternion_to_vec4 (q, &vec);

  switch (property_index) {
  case -1:
    buffer[offset+0] = graphene_vec4_get_x (&vec);
    buffer[offset+1] = graphene_vec4_get_y (&vec);
    buffer[offset+2] = graphene_vec4_get_z (&vec);
    buffer[offset+3] = graphene_vec4_get_w (&vec);
    break;
  case 0:
    buffer[offset] = graphene_vec4_get_x (&vec);
    break;
  case 1:
    buffer[offset] = graphene_vec4_get_y (&vec);
    break;
  case 2:
    buffer[offset] = graphene_vec4_get_z (&vec);
    break;
  case 3:
    buffer[offset] = graphene_vec4_get_w (&vec);
    break;
  }
}

static void
ghtree_property_binding_set_value_point3d (GthreePropertyBinding *binding,
                                           float *buffer,
                                           int offset)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
  graphene_point3d_t point;

  if (priv->resolved_prop_index != -1)
    {
      float old[3];
      gpointer property = priv->get_property (priv->resolved_object);
      priv->get_value (binding, property, -1, old, 0);
      point.x = old[0];
      point.y = old[1];
      point.z = old[2];
    }

  switch (priv->resolved_prop_index) {
  case -1:
    point.x = buffer[0];
    point.y = buffer[1];
    point.z = buffer[2];
    break;
  case 0:
    point.x = buffer[0];
    break;
  case 1:
    point.y = buffer[0];
    break;
  case 2:
    point.z = buffer[0];
    break;
  }

  priv->set_property (priv->resolved_object, &point);
}

static void
ghtree_property_binding_set_value_euler (GthreePropertyBinding *binding,
                                         float *buffer,
                                         int offset)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
  float val[3];
  graphene_euler_t euler;

  if (priv->resolved_prop_index != -1)
    {
      gpointer property = priv->get_property (priv->resolved_object);
      priv->get_value (binding, property, -1, val, 0);
    }

  switch (priv->resolved_prop_index) {
  case -1:
    val[0] = buffer[0];
    val[1] = buffer[1];
    val[2] = buffer[2];
    break;
  case 0:
    val[0] = buffer[0];
    break;
  case 1:
    val[1] = buffer[0];
    break;
  case 2:
    val[2] = buffer[0];
    break;
  }

  graphene_euler_init (&euler, val[0], val[1], val[2]);
  priv->set_property (priv->resolved_object, &euler);
}

static void
ghtree_property_binding_set_value_quaternion (GthreePropertyBinding *binding,
                                              float *buffer,
                                              int offset)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
  float val[4];
  graphene_euler_t euler;

  if (priv->resolved_prop_index != -1)
    {
      gpointer property = priv->get_property (priv->resolved_object);
      priv->get_value (binding, property, -1, val, 0);
    }

  switch (priv->resolved_prop_index) {
  case -1:
    val[0] = buffer[0];
    val[1] = buffer[1];
    val[2] = buffer[2];
    val[3] = buffer[3];
    break;
  case 0:
    val[0] = buffer[0];
    break;
  case 1:
    val[1] = buffer[0];
    break;
  case 2:
    val[2] = buffer[0];
    break;
  case 3:
    val[3] = buffer[0];
    break;
  }

  graphene_euler_init (&euler, val[0], val[1], val[2]);
  priv->set_property (priv->resolved_object, &euler);
}


static void
ghtree_property_binding_bind (GthreePropertyBinding *binding)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
  GthreeObject *target_object = priv->node;
  GthreeParsedPath *parsed_path = priv->parsed_path;
  const char *object_name = parsed_path->object_name;
  const char *property_name = parsed_path->property_name;
  const char *property_index = parsed_path->property_index;
  g_autoptr(GObject) resolved_object = NULL;
  GthreePropertyGetter get_property = NULL;
  GthreePropertyGetValue get_value = NULL;
  GthreePropertySetter set_property = NULL;
  GthreePropertySetValue set_value = NULL;
  int resolved_prop_index = -1;

  priv->bound = TRUE;

  if (target_object == NULL)
    {
      target_object = ghtree_property_binding_find_node (priv->root, priv->parsed_path->node_name);
      if (target_object == NULL)
        target_object = priv->root;
      priv->node = g_object_ref (target_object);
    }

  if (target_object == NULL)
    {
      g_warning ("Trying to update node for track: %s but it wasn\'t found.", priv->path);
      return;
    }

  if (object_name)
    {
      // TODO: Animate materials and bones
      g_warning ("Non-supported object_name binding for property %s\n", property_name);
      return;
    }
  else
    {
      int max_index;

      if (strcmp (property_name, "position") == 0)
        {
          resolved_object = (GObject *)g_object_ref (target_object);
          max_index = 2;

          get_property = (GthreePropertyGetter)gthree_object_get_position;
          get_value = ghtree_property_binding_get_value_vec3;
          set_property = (GthreePropertySetter)gthree_object_set_position;
          set_value = ghtree_property_binding_set_value_point3d;
        }
      else if (strcmp (property_name, "scale") == 0)
        {
          max_index = 2;
          resolved_object = (GObject *)g_object_ref (target_object);

          get_property = (GthreePropertyGetter)gthree_object_get_scale;
          get_value = ghtree_property_binding_get_value_vec3;
          set_property = (GthreePropertySetter)gthree_object_set_scale;
          set_value = ghtree_property_binding_set_value_point3d;
        }
      else if (strcmp (property_name, "rotation") == 0)
        {
          resolved_object = (GObject *)g_object_ref (target_object);
          max_index = 2;

          get_property = (GthreePropertyGetter)gthree_object_get_rotation;
          get_value = ghtree_property_binding_get_value_euler;
          set_property = (GthreePropertySetter)gthree_object_set_rotation;
          set_value = ghtree_property_binding_set_value_euler;
        }
      else if (strcmp (property_name, "quaternion") == 0)
        {
          resolved_object = (GObject *)g_object_ref (target_object);
          max_index = 3;

          get_property = (GthreePropertyGetter)gthree_object_get_quaternion;
          get_value = ghtree_property_binding_get_value_quaternion;
          set_property = (GthreePropertySetter)gthree_object_set_quaternion;
          set_value = ghtree_property_binding_set_value_quaternion;
        }
      else
        {
          g_warning ("Non-supported binding for property %s\n", property_name);
          return;
        }

      if (property_index != NULL)
        {
          int index = atoi (property_index);
          if (index < 0 || index > max_index)
            {
              g_warning ("Non-supported obect_index for property %s\n", property_name);
              return;
            }
          resolved_prop_index = index;
        }
    }

  priv->resolved_object = g_steal_pointer (&resolved_object);
  priv->resolved_prop_index = resolved_prop_index;
  priv->get_property = get_property;
  priv->get_value = get_value;
  priv->set_property = set_property;
  priv->set_value = set_value;
}

void
ghtree_property_binding_get_value (GthreePropertyBinding *binding,
                                   float *buffer,
                                   int offset)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);
  gpointer property;

  if (!priv->bound)
    ghtree_property_binding_bind (binding);

  if (priv->get_value == NULL)
    return;

  property = priv->get_property (priv->resolved_object);
  priv->get_value (binding, property, priv->resolved_prop_index, buffer, offset);
}

void
ghtree_property_binding_set_value (GthreePropertyBinding *binding,
                                   float *buffer,
                                   int offset)
{
  GthreePropertyBindingPrivate *priv = gthree_property_binding_get_instance_private (binding);

  if (!priv->bound)
    ghtree_property_binding_bind (binding);

  if (priv->get_value == NULL)
    return;

  priv->set_value (binding, buffer, offset);
}
