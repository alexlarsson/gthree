#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreePerspectiveCamera *camera;
GthreeObject *group = NULL;

GthreeScene *ortho_scene;
GthreeOrthographicCamera *ortho_camera;

GthreeTexture *sprite0_texture;
GthreeTexture *sprite1_texture;
GthreeTexture *sprite2_texture;

GthreeSprite *spriteTL, *spriteTR, *spriteBL, *spriteBR, *spriteC;

static void
update_hud_sprites (int width,
                    int height)
{
  width /= 2;
  height /= 2;

  gthree_object_set_position_xyz (GTHREE_OBJECT (spriteTL), -width,  height, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (spriteTR),  width,  height, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (spriteBL), -width, -height, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (spriteBR),  width, -height, 1);
  gthree_object_set_position_xyz (GTHREE_OBJECT (spriteC),   0,      0,      1);
}

GthreeScene *
init_scene (GtkWidget *window)
{
  g_autoptr(GdkPixbuf) sprite0_pixbuf = examples_load_pixbuf ("sprite0.png");
  g_autoptr(GdkPixbuf) sprite1_pixbuf = examples_load_pixbuf ("sprite1.png");
  g_autoptr(GdkPixbuf) sprite2_pixbuf = examples_load_pixbuf ("sprite2.png");

  sprite0_texture = gthree_texture_new (sprite0_pixbuf);
  sprite1_texture = gthree_texture_new (sprite1_pixbuf);
  sprite2_texture = gthree_texture_new (sprite2_pixbuf);

  scene = gthree_scene_new ();

  int amount = 200;
  float radius = 500;

  g_autoptr(GthreeSpriteMaterial) materialB = gthree_sprite_material_new ();
  g_autoptr(GthreeSpriteMaterial) materialC = gthree_sprite_material_new ();

  gthree_sprite_material_set_color (materialB, white ());
  gthree_sprite_material_set_map (materialB, sprite1_texture);
  gthree_sprite_material_set_color (materialC, white ());
  gthree_sprite_material_set_map (materialC, sprite2_texture);

  // TODO: Set fog == TRUE for materials

  group = GTHREE_OBJECT (gthree_group_new ());

  for (int a = 0; a < amount; a++)
    {
      GthreeSprite *sprite;
      float x = g_random_double_range (-0.5, 0.5);
      float y = g_random_double_range (-0.5, 0.5);
      float z = g_random_double_range (-0.5, 0.5);
      g_autoptr(GthreeMaterial) material = NULL;
      graphene_vec3_t pos;

      if (z < 0)
        {
          material = gthree_material_clone (GTHREE_MATERIAL (materialB));
        }
      else
        {
          graphene_vec3_t color;
          graphene_vec2_t v2;

          material = gthree_material_clone (GTHREE_MATERIAL (materialC));

          rgb_init_from_hsl (&color,
                             g_random_double_range (0, 180),
                             0.75, 0.5);
          gthree_sprite_material_set_color (GTHREE_SPRITE_MATERIAL (material), &color);

          // This makes these sprites half the size (since we see only the top left quadrant, testing
          // both offset and repeat
          gthree_texture_set_offset (gthree_sprite_material_get_map (GTHREE_SPRITE_MATERIAL (material)),
                                     graphene_vec2_init (&v2, -0.5, -0.5));
          gthree_texture_set_repeat (gthree_sprite_material_get_map (GTHREE_SPRITE_MATERIAL (material)),
                                     graphene_vec2_init (&v2, 2, 2));
        }

      sprite = gthree_sprite_new (GTHREE_MATERIAL (material));

      graphene_vec3_init (&pos, x, y, z);
      graphene_vec3_normalize (&pos, &pos);
      graphene_vec3_scale (&pos, radius, &pos);

      gthree_object_set_position (GTHREE_OBJECT (sprite), &pos);

      gthree_object_add_child (group, GTHREE_OBJECT (sprite));
    }

  gthree_object_add_child (GTHREE_OBJECT (scene), group);

  /* Orthographic overlay */

  ortho_scene = gthree_scene_new ();

  g_autoptr(GthreeSpriteMaterial) material = gthree_sprite_material_new ();
  gthree_sprite_material_set_map (material, sprite0_texture);

  int width = gdk_pixbuf_get_width (sprite0_pixbuf) * gtk_widget_get_scale_factor (window);
  int height = gdk_pixbuf_get_height (sprite0_pixbuf) * gtk_widget_get_scale_factor (window);
  graphene_vec2_t v2;

  spriteTL = gthree_sprite_new (GTHREE_MATERIAL (material));
  gthree_sprite_set_center (spriteTL,
                            graphene_vec2_init (&v2, 0.0, 1.0));
  gthree_object_set_scale_xyz (GTHREE_OBJECT (spriteTL),
                               width, height, 1);
  gthree_object_add_child (GTHREE_OBJECT (ortho_scene), GTHREE_OBJECT (spriteTL));

  spriteTR = gthree_sprite_new (GTHREE_MATERIAL (material));
  gthree_sprite_set_center (spriteTR,
                            graphene_vec2_init (&v2, 1.0, 1.0));
  gthree_object_set_scale_xyz (GTHREE_OBJECT (spriteTR),
                               width, height, 1);
  gthree_object_add_child (GTHREE_OBJECT (ortho_scene), GTHREE_OBJECT (spriteTR));

  spriteBL = gthree_sprite_new (GTHREE_MATERIAL (material));
  gthree_sprite_set_center (spriteBL,
                            graphene_vec2_init (&v2, 0.0, 0.0));
  gthree_object_set_scale_xyz (GTHREE_OBJECT (spriteBL),
                               width, height, 1);
  gthree_object_add_child (GTHREE_OBJECT (ortho_scene), GTHREE_OBJECT (spriteBL));

  spriteBR = gthree_sprite_new (GTHREE_MATERIAL (material));
  gthree_sprite_set_center (spriteBR,
                            graphene_vec2_init (&v2, 1.0, 0.0));
  gthree_object_set_scale_xyz (GTHREE_OBJECT (spriteBR),
                               width, height, 1);
  gthree_object_add_child (GTHREE_OBJECT (ortho_scene), GTHREE_OBJECT (spriteBR));

  spriteC = gthree_sprite_new (GTHREE_MATERIAL (material));
  gthree_sprite_set_center (spriteC,
                            graphene_vec2_init (&v2, 0.5, 0.5));
  gthree_object_set_scale_xyz (GTHREE_OBJECT (spriteC),
                               width, height, 1);
  gthree_object_add_child (GTHREE_OBJECT (ortho_scene), GTHREE_OBJECT (spriteC));

  update_hud_sprites (100, 100);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float relative_time;
  GthreeObjectIter iter;
  GthreeObject *sprite;
  int i, len;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  gthree_object_set_rotation_xyz (group,
                                  relative_time * 0.5, relative_time * 0.75, relative_time * 1.0);


  i = 0;
  len = gthree_object_get_n_children (group);

  gthree_object_iter_init (&iter, group);
  while (gthree_object_iter_next (&iter, &sprite))
    {
      GthreeMaterial *material = gthree_sprite_get_material (GTHREE_SPRITE (sprite));
      const graphene_vec3_t *sprite_pos = gthree_object_get_position (sprite);
      float scale = sinf (relative_time / 100.0 + graphene_vec3_get_x (sprite_pos) * 0.01 ) * 0.3 + 1.0;
      GthreeTexture *map = gthree_sprite_material_get_map (GTHREE_SPRITE_MATERIAL (material));
      GdkPixbuf *pixbuf = gthree_texture_get_pixbuf (map);
      int imageWidth = gdk_pixbuf_get_width (pixbuf);
      int imageHeight = gdk_pixbuf_get_height (pixbuf);

      gthree_sprite_material_set_rotation (GTHREE_SPRITE_MATERIAL (material),
                                           gthree_sprite_material_get_rotation (GTHREE_SPRITE_MATERIAL (material)) + 0.1 * ( i / (float)len ));

      gthree_object_set_scale_xyz (sprite,
                                   imageWidth * scale, imageHeight * scale, 1.0);

      if (map != sprite2_texture)
        gthree_material_set_opacity (GTHREE_MATERIAL (material), sinf( relative_time / 100.0 + graphene_vec3_get_x (sprite_pos) * 0.01 ) * 0.4 + 0.6);

      i++;
    }

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

static void
resize_area (GthreeArea *area,
             gint width,
             gint height,
             GthreePerspectiveCamera *camera)
{
  gthree_perspective_camera_set_aspect (camera, (float)width / (float)(height));

  gthree_orthographic_camera_set_left (ortho_camera, -width / 2);
  gthree_orthographic_camera_set_right (ortho_camera, width / 2);
  gthree_orthographic_camera_set_top (ortho_camera, height / 2);
  gthree_orthographic_camera_set_bottom (ortho_camera, -height / 2);

  update_hud_sprites (width, height);
}

static gboolean
render_area (GtkGLArea    *gl_area,
             GdkGLContext *context)
{
  GthreeArea *area = GTHREE_AREA (gl_area);

  gthree_renderer_set_autoclear (gthree_area_get_renderer (area), FALSE);
  gthree_renderer_clear (gthree_area_get_renderer (area), TRUE, TRUE, TRUE);
  gthree_renderer_render (gthree_area_get_renderer (area),
                          scene, GTHREE_CAMERA (camera));
  gthree_renderer_clear_depth (gthree_area_get_renderer (area));
  gthree_renderer_render (gthree_area_get_renderer (area),
                          ortho_scene, GTHREE_CAMERA (ortho_camera));
  return TRUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area;
  GthreeScene *scene;
  gboolean done = FALSE;

  window = examples_init ("Sprites", &box, &done);

  scene = init_scene (window);
  camera = gthree_perspective_camera_new (60, 1, 1, 2100);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  0, 0, 1500);

  int width = 100;
  int height = 100;
  ortho_camera = gthree_orthographic_camera_new ( - width / 2, width / 2, height / 2, - height / 2, 1, 10);
  gthree_object_add_child (GTHREE_OBJECT (ortho_scene), GTHREE_OBJECT (ortho_camera));
  gthree_object_set_position_xyz (GTHREE_OBJECT (ortho_camera),
                                  0, 0, 10);


  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect (area, "render", G_CALLBACK (render_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (box), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  gtk_widget_show (window);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
