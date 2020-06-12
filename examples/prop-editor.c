#include <graphene-gobject.h>
#include "prop-editor.h"
#include "utils.h"

typedef struct
{
  GObject *obj;
  GParamSpec *spec;
  gulong modified_id;
} ObjectProperty;

static void
free_object_property (ObjectProperty *p)
{
  g_free (p);
}

static void
connect_controller (GObject     *controller,
                    const gchar *signal,
                    GObject     *model,
                    GParamSpec  *spec,
                    GCallback    func)
{
  ObjectProperty *p;

  p = g_new (ObjectProperty, 1);
  p->obj = model;
  p->spec = spec;

  p->modified_id = g_signal_connect_data (controller, signal, func, p,
                                          (GClosureNotify)free_object_property, 0);
  g_object_set_data (controller, "object-property", p);
}

static void
block_controller (GObject *controller)
{
  ObjectProperty *p = g_object_get_data (controller, "object-property");

  if (p)
    g_signal_handler_block (controller, p->modified_id);
}

static void
unblock_controller (GObject *controller)
{
  ObjectProperty *p = g_object_get_data (controller, "object-property");

  if (p)
    g_signal_handler_unblock (controller, p->modified_id);
}

static void
get_property_value (GObject *object, GParamSpec *pspec, GValue *value)
{
  g_object_get_property (object, pspec->name, value);
}

static void
set_property_value (GObject *object, GParamSpec *pspec, GValue *value)
{
  g_object_set_property (object, pspec->name, value);
}

static void
notify_property (GObject *object, GParamSpec *pspec)
{
  g_object_notify (object, pspec->name);
}


typedef struct
{
  gpointer instance;
  GObject *alive_object;
  gulong id;
} DisconnectData;

static void
disconnect_func (gpointer data)
{
  DisconnectData *dd = data;

  g_signal_handler_disconnect (dd->instance, dd->id);
}

static void
signal_removed (gpointer  data,
                GClosure *closure)
{
  DisconnectData *dd = data;

  g_object_steal_data (dd->alive_object, "alive-object-data");
  g_free (dd);

}

static void
g_object_connect_property (GObject    *object,
                           GParamSpec *spec,
                           GCallback   func,
                           gpointer    data,
                           GObject    *alive_object)
{
  GClosure *closure;
  gchar *with_detail;
  DisconnectData *dd;

  with_detail = g_strconcat ("notify::", spec->name, NULL);

  dd = g_new (DisconnectData, 1);

  closure = g_cclosure_new (func, data, NULL);
  g_closure_add_invalidate_notifier (closure, dd, signal_removed);
  dd->id = g_signal_connect_closure (object, with_detail, closure, FALSE);
  dd->instance = object;
  dd->alive_object = alive_object;

  g_object_set_data_full (G_OBJECT (alive_object), "alive-object-data",
                          dd, disconnect_func);

  g_free (with_detail);
}

static void
float_modified (GtkAdjustment *adj, ObjectProperty *p)
{
  GValue val = G_VALUE_INIT;

  g_value_init (&val, G_TYPE_FLOAT);
  g_value_set_float (&val, (float) gtk_adjustment_get_value (adj));
  set_property_value (p->obj, p->spec, &val);
  g_value_unset (&val);
}

static void
float_changed (GObject *object, GParamSpec *pspec, gpointer data)
{
  GtkAdjustment *adj = GTK_ADJUSTMENT (data);
  GValue val = G_VALUE_INIT;

  g_value_init (&val, G_TYPE_FLOAT);
  get_property_value (object, pspec, &val);

  if (g_value_get_float (&val) != (float) gtk_adjustment_get_value (adj))
    {
      block_controller (G_OBJECT (adj));
      gtk_adjustment_set_value (adj, g_value_get_float (&val));
      unblock_controller (G_OBJECT (adj));
    }

  g_value_unset (&val);
}

static void
bool_modified (GtkSwitch *tb, GParamSpec *ignored, ObjectProperty *p)
{
  GValue val = G_VALUE_INIT;

  g_value_init (&val, G_TYPE_BOOLEAN);
  g_value_set_boolean (&val, gtk_switch_get_active (tb));
  set_property_value (p->obj, p->spec, &val);
  g_value_unset (&val);
}

static void
bool_changed (GObject *object, GParamSpec *pspec, gpointer data)
{
  GtkSwitch *tb = GTK_SWITCH (data);
  GValue val = G_VALUE_INIT;

  g_value_init (&val, G_TYPE_BOOLEAN);
  get_property_value (object, pspec, &val);

  if (g_value_get_boolean (&val) != gtk_switch_get_active (tb))
    {
      block_controller (G_OBJECT (tb));
      gtk_switch_set_active (tb, g_value_get_boolean (&val));
      unblock_controller (G_OBJECT (tb));
    }

  g_value_unset (&val);
}

static void
rgb_changed (GObject *object, GParamSpec *pspec, gpointer data)
{
  GtkColorChooser *cb = GTK_COLOR_CHOOSER (data);
  GValue val = G_VALUE_INIT;
  graphene_vec3_t *color;
  GdkRGBA rgba = {0, 0, 0, 1};
  GdkRGBA cb_color;

  g_value_init (&val, GRAPHENE_TYPE_VEC3);
  get_property_value (object, pspec, &val);

  color = g_value_get_boxed (&val);
  if (color != NULL)
    {
      rgba.red = graphene_vec3_get_x (color);
      rgba.green = graphene_vec3_get_y (color);
      rgba.blue = graphene_vec3_get_z (color);
    }

  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (cb), &cb_color);

  if (color != NULL && !gdk_rgba_equal (&rgba, &cb_color))
    {
      block_controller (G_OBJECT (cb));
      gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (cb), &rgba);
      unblock_controller (G_OBJECT (cb));
    }
 g_value_unset (&val);
}

static void
rgb_modified (GtkColorButton *cb, GParamSpec *ignored, ObjectProperty *p)
{
  GValue val = G_VALUE_INIT;
  GValue val2 = G_VALUE_INIT;
  GdkRGBA *rgba;
  graphene_vec3_t rgb;

  g_value_init (&val, GDK_TYPE_RGBA);
  g_object_get_property (G_OBJECT (cb), "rgba", &val);

  rgba = g_value_get_boxed (&val);
  graphene_vec3_init (&rgb,
                      rgba->red,
                      rgba->green,
                      rgba->blue);
  g_value_unset (&val);


  g_value_init (&val2, GRAPHENE_TYPE_VEC3);
  g_value_set_boxed (&val2, &rgb);

  set_property_value (p->obj, p->spec, &val2);
  g_value_unset (&val2);
}

static void
texture_modified (GtkComboBox *combo, ObjectProperty *p)
{
  GValue val = G_VALUE_INIT;
  const gchar *id;
  g_autoptr(GthreeTexture) texture = NULL;

  id =  gtk_combo_box_get_active_id (combo);

  if (*id != 0)
    {
      if (g_str_has_prefix (id, "cube/"))
        {
          GdkPixbuf *pixbufs[6];
          int i;

          examples_load_cube_pixbufs ("cube/SwedishRoyalCastle", pixbufs);
          texture = (GthreeTexture *)gthree_cube_texture_new_from_array (pixbufs);
          for (i = 0; i < 6; i++)
            g_object_unref (pixbufs[i]);
        }
      else
        {
          g_autoptr(GdkPixbuf) pixbuf = examples_load_pixbuf (id);
          texture = gthree_texture_new (pixbuf);
        }
    }

  g_value_init (&val, p->spec->value_type);
  g_value_set_object (&val, texture);
  set_property_value (p->obj, p->spec, &val);
  g_value_unset (&val);

}

static void
enum_modified (GtkComboBox *combo, ObjectProperty *p)
{
  gint i;
  GEnumClass *eclass;
  GValue val = G_VALUE_INIT;

  eclass = G_ENUM_CLASS (g_type_class_peek (p->spec->value_type));
  i =  gtk_combo_box_get_active (combo);

  g_value_init (&val, p->spec->value_type);
  g_value_set_enum (&val, eclass->values[i].value);
  set_property_value (p->obj, p->spec, &val);
  g_value_unset (&val);
}

static void
enum_changed (GObject *object, GParamSpec *pspec, gpointer data)
{
  GValue val = G_VALUE_INIT;
  GEnumClass *eclass;
  gint i;

  eclass = G_ENUM_CLASS (g_type_class_peek (pspec->value_type));

  g_value_init (&val, pspec->value_type);
  get_property_value (object, pspec, &val);

  i = 0;
  while (i < eclass->n_values)
    {
      if (eclass->values[i].value == g_value_get_enum (&val))
        break;
      ++i;
    }
  g_value_unset (&val);

  block_controller (G_OBJECT (data));
  gtk_combo_box_set_active (GTK_COMBO_BOX (data), i);
  unblock_controller (G_OBJECT (data));
}

static GtkWidget *
property_editor (GObject     *object,
                 GParamSpec  *spec)
{
  GtkWidget *prop_edit;
  GtkAdjustment *adj;
  g_autofree gchar *msg = NULL;
  GType type = G_PARAM_SPEC_TYPE (spec);

  if (type == G_TYPE_PARAM_FLOAT)
    {
      adj = gtk_adjustment_new (G_PARAM_SPEC_FLOAT (spec)->default_value,
                                G_PARAM_SPEC_FLOAT (spec)->minimum,
                                G_PARAM_SPEC_FLOAT (spec)->maximum,
                                0.1,
                                MAX ((G_PARAM_SPEC_FLOAT (spec)->maximum - G_PARAM_SPEC_FLOAT (spec)->minimum) / 10, 0.1),
                                0.0);

      prop_edit = gtk_spin_button_new (adj, 0.1, 2);

      g_object_connect_property (object, spec,
                                 G_CALLBACK (float_changed),
                                 adj, G_OBJECT (adj));

      connect_controller (G_OBJECT (adj), "value_changed",
                          object, spec, G_CALLBACK (float_modified));
    }
  else if (type == G_TYPE_PARAM_BOOLEAN)
    {
      prop_edit = gtk_switch_new ();

      g_object_connect_property (object, spec,
                                 G_CALLBACK (bool_changed),
                                 prop_edit, G_OBJECT (prop_edit));

      connect_controller (G_OBJECT (prop_edit), "notify::active",
                          object, spec, G_CALLBACK (bool_modified));
    }
  else if (type == G_TYPE_PARAM_ENUM)
    {
      {
        GEnumClass *eclass;
        gint j;

        prop_edit = gtk_combo_box_text_new ();

        eclass = G_ENUM_CLASS (g_type_class_ref (spec->value_type));

        j = 0;
        while (j < eclass->n_values)
          {
            const char *text = eclass->values[j].value_name;
            if (g_str_has_prefix (text, "GTHREE_"))
              text += strlen("GTHREE_");
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (prop_edit), text);
            ++j;
          }
        connect_controller (G_OBJECT (prop_edit), "changed",
                            object, spec, G_CALLBACK (enum_modified));

        g_type_class_unref (eclass);

        g_object_connect_property (object, spec,
                                   G_CALLBACK (enum_changed),
                                   prop_edit, G_OBJECT (prop_edit));
      }
    }
  else if (type == G_TYPE_PARAM_BOXED &&
           G_PARAM_SPEC_VALUE_TYPE (spec) == GRAPHENE_TYPE_VEC3 &&
           (strstr (spec->name, "color") != NULL ||
            strstr (spec->name, "Color") != NULL))
    {
      prop_edit = gtk_color_button_new ();
      gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (prop_edit), FALSE);

      g_object_connect_property (object, spec,
                                 G_CALLBACK (rgb_changed),
                                 prop_edit, G_OBJECT (prop_edit));

      connect_controller (G_OBJECT (prop_edit), "notify::rgba",
                          object, spec, G_CALLBACK (rgb_modified));
    }
  else if (type == G_TYPE_PARAM_OBJECT &&
           G_PARAM_SPEC_VALUE_TYPE (spec) == GTHREE_TYPE_TEXTURE)
    {
      prop_edit = gtk_combo_box_text_new ();
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "", "None");
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "crate.gif", "Crate");
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "disturb.jpg", "Disturb");
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "brick_bump.jpg", "Brick bump");
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "brick_diffuse.jpg", "Brick diffuse");
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "brick_roughness.jpg", "Brick roughness");
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (prop_edit), "cube/SwedishRoyalCastle", "CastleCube");

      gtk_combo_box_set_active (GTK_COMBO_BOX (prop_edit), 0);

      connect_controller (G_OBJECT (prop_edit), "changed",
                          object, spec, G_CALLBACK (texture_modified));
   }
  else
    {
      msg = g_strdup_printf ("Uneditable property type: %s",
                             g_type_name (G_PARAM_SPEC_TYPE (spec)));
      prop_edit = gtk_label_new (msg);
      gtk_widget_set_halign (prop_edit, GTK_ALIGN_START);
      gtk_widget_set_valign (prop_edit, GTK_ALIGN_CENTER);
    }

  if (g_param_spec_get_blurb (spec))
    gtk_widget_set_tooltip_text (prop_edit, g_param_spec_get_blurb (spec));

  notify_property (object, spec);

  return prop_edit;
}

GtkWidget *
property_editor_widget_new (GObject  *object, const char *name)
{
  GtkWidget *vbox, *hbox, *label, *editor;
  guint i, num_properties;
  GParamSpec **props;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  props = g_object_class_list_properties (G_OBJECT_GET_CLASS (object), &num_properties);

  label = gtk_label_new (name);
  gtk_box_append (GTK_BOX (vbox), label);

  for (i = 0; i < num_properties; i++)
    {
      GParamSpec *prop = props[i];

      if ((prop->flags & G_PARAM_READWRITE) != G_PARAM_READWRITE)
        continue;

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      label = gtk_label_new (g_param_spec_get_nick (prop));
      gtk_box_append (GTK_BOX (hbox), label);

      editor = property_editor (object, prop);
      gtk_box_append (GTK_BOX (hbox), editor);

      gtk_box_append (GTK_BOX (vbox), hbox);

    }

#ifdef USE_GTK3
  gtk_widget_show_all (vbox);
#endif

  return vbox;
}
