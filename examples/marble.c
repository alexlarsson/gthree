#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

#include <gthree/gthree.h>
#include "utils.h"
#include "physics.h"

/* --- Platform / Level data ------------------------------------------------ */

typedef enum {
  PLATFORM_FLAT,
  PLATFORM_RAMP_X_POS,
  PLATFORM_RAMP_X_NEG,
  PLATFORM_RAMP_Z_POS,
  PLATFORM_RAMP_Z_NEG,
  PLATFORM_NARROW,
  PLATFORM_GOAL,
  PLATFORM_BOUNCE,
} PlatformType;

typedef struct {
  float x, y, z;
  float w, h, d;
  float r, g, b;
  PlatformType type;
} Platform;

typedef struct {
  const Platform *platforms;
  int n_platforms;
  float start_x, start_y, start_z;
  float time_limit;
  const char *name;
} Level;

#define RAMP_ANGLE 0.22f   /* ~12.6 degrees */

/* Level 1 -- Beginner Slopes */
static const Platform level1_platforms[] = {
  /* Start platform */
  {  0,  10,   0,    8, 1, 8,    0.30, 0.50, 0.90,  PLATFORM_FLAT },
  /* Ramp down +X */
  {  8,   8.5, 0,    8, 1, 6,    0.40, 0.60, 0.95,  PLATFORM_RAMP_X_POS },
  /* Landing */
  { 16,   6,   0,    6, 1, 6,    0.30, 0.70, 0.50,  PLATFORM_FLAT },
  /* Right turn +Z */
  { 16,   6,   6,    6, 1, 6,    0.30, 0.70, 0.50,  PLATFORM_FLAT },
  /* Narrow bridge */
  { 16,   6,  14,    2, 1, 8,    0.90, 0.30, 0.30,  PLATFORM_NARROW },
  /* After bridge */
  { 16,   6,  22,    6, 1, 4,    0.30, 0.70, 0.50,  PLATFORM_FLAT },
  /* Ramp down +Z */
  { 16,   4,  28,    6, 1, 8,    0.50, 0.50, 0.80,  PLATFORM_RAMP_Z_POS },
  /* Goal */
  { 16,   2,  36,    6, 1, 6,    1.00, 0.85, 0.00,  PLATFORM_GOAL },
  /* Guardrails on first ramp */
  {  8,   9.5, 3.5,  8, 2, 0.5,  0.20, 0.20, 0.50,  PLATFORM_FLAT },
  {  8,   9.5,-3.5,  8, 2, 0.5,  0.20, 0.20, 0.50,  PLATFORM_FLAT },
};

/* Level 2 -- Zigzag Heights */
static const Platform level2_platforms[] = {
  {  0,  16,   0,    6, 1, 6,    0.20, 0.60, 0.80,  PLATFORM_FLAT },
  {  7,  14,   0,    8, 1, 5,    0.30, 0.60, 0.90,  PLATFORM_RAMP_X_POS },
  { 14,  12,   0,    5, 1, 5,    0.20, 0.70, 0.40,  PLATFORM_FLAT },
  { 14,  12,   5.5,  2.5, 1, 6,  0.90, 0.20, 0.20,  PLATFORM_NARROW },
  { 14,  12,  11,    5, 1, 5,    0.20, 0.70, 0.40,  PLATFORM_FLAT },
  {  7,  10,  11,    8, 1, 5,    0.40, 0.50, 0.80,  PLATFORM_RAMP_X_NEG },
  {  0,   8,  11,    5, 1, 5,    0.20, 0.70, 0.40,  PLATFORM_FLAT },
  {  0,   8,  17,    2, 1, 7,    0.90, 0.50, 0.20,  PLATFORM_NARROW },
  {  0,   8,  23,    5, 1, 5,    0.20, 0.70, 0.40,  PLATFORM_FLAT },
  {  6,   6,  23,    7, 1, 4,    0.40, 0.50, 0.90,  PLATFORM_RAMP_X_POS },
  { 12,   4,  23,    4, 1, 4,    0.80, 0.20, 0.80,  PLATFORM_BOUNCE },
  { 12,   4,  30,    5, 1, 5,    1.00, 0.85, 0.00,  PLATFORM_GOAL },
  { 12,   4,  27,    4, 1, 5,    0.50, 0.50, 0.80,  PLATFORM_RAMP_Z_POS },
};

/* Level 3 -- The Gauntlet */
static const Platform level3_platforms[] = {
  {  0,  20,   0,    5, 1, 5,    0.30, 0.30, 0.80,  PLATFORM_FLAT },
  {  7,  20,   0,    9, 1, 1.5,  0.80, 0.20, 0.20,  PLATFORM_NARROW },
  { 14,  20,   0,    3, 1, 3,    0.30, 0.60, 0.30,  PLATFORM_FLAT },
  { 14,  17,   5,    3, 1, 7,    0.50, 0.40, 0.80,  PLATFORM_RAMP_Z_POS },
  { 14,  14,  11,    3, 1, 3,    0.30, 0.60, 0.30,  PLATFORM_FLAT },
  {  8,  14,  11,    8, 1, 1.5,  0.80, 0.50, 0.20,  PLATFORM_NARROW },
  {  2,  14,  11,    3, 1, 3,    0.30, 0.60, 0.30,  PLATFORM_FLAT },
  {  2,  11,   5,    3, 1, 8,    0.50, 0.40, 0.80,  PLATFORM_RAMP_Z_NEG },
  {  2,   8,  -1,    3, 1, 3,    0.30, 0.60, 0.30,  PLATFORM_FLAT },
  {  8,   8,  -1,    7, 1, 1.5,  0.80, 0.20, 0.20,  PLATFORM_NARROW },
  { 14,   8,  -1,    4, 1, 4,    1.00, 0.85, 0.00,  PLATFORM_GOAL },
};

#define ARRAY_LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))

static const Level levels[] = {
  { level1_platforms, ARRAY_LEN (level1_platforms),  0, 12, 0,   60, "Beginner Slopes" },
  { level2_platforms, ARRAY_LEN (level2_platforms),  0, 18, 0,   75, "Zigzag Heights"  },
  { level3_platforms, ARRAY_LEN (level3_platforms),  0, 22, 0,   90, "The Gauntlet"    },
};
#define N_LEVELS ARRAY_LEN (levels)

/* --- Game state ----------------------------------------------------------- */

typedef enum {
  STATE_READY,
  STATE_PLAYING,
  STATE_WON,
  STATE_LOST_FELL,
  STATE_LOST_TIME,
} GameState;

#define MARBLE_RADIUS  1.0f
#define MARBLE_MASS    1.0f
#define MOVE_FORCE    18.0f
#define FALL_THRESHOLD -30.0f

static GthreeScene *scene;
static GthreePerspectiveCamera *camera;
static GthreeMesh *marble_mesh;
static GthreeGroup *level_group;
static GthreeMesh *goal_mesh;

static PhysicsWorld *physics;
static int marble_body_id;

static GameState game_state;
static int current_level;
static float game_timer;
static gint64 last_frame_time;

static gboolean key_up, key_down, key_left, key_right;

static GtkWidget *timer_label;
static GtkWidget *status_label;
static GtkWidget *level_label;

/* --- Helpers -------------------------------------------------------------- */

static void
reset_marble (void)
{
  const Level *lvl = &levels[current_level];

  physics_world_set_position (physics, marble_body_id,
                              lvl->start_x, lvl->start_y, lvl->start_z);
  physics_world_activate (physics, marble_body_id);

  gthree_object_set_position_xyz (GTHREE_OBJECT (marble_mesh),
                                  lvl->start_x, lvl->start_y, lvl->start_z);

  last_frame_time = 0;
}

static gboolean
marble_on_goal (void)
{
  const Level *lvl = &levels[current_level];
  float mx, my, mz;

  physics_world_get_position (physics, marble_body_id, &mx, &my, &mz);

  for (int i = 0; i < lvl->n_platforms; i++)
    {
      const Platform *p = &lvl->platforms[i];
      if (p->type != PLATFORM_GOAL)
        continue;

      float hw = p->w / 2, hd = p->d / 2;
      if (mx >= p->x - hw && mx <= p->x + hw &&
          mz >= p->z - hd && mz <= p->z + hd &&
          my <= p->y + p->h / 2 + MARBLE_RADIUS + 0.5f &&
          my >= p->y - 1.0f)
        return TRUE;
    }
  return FALSE;
}

/* --- Scene construction --------------------------------------------------- */

static void
build_level (int level_index)
{
  const Level *lvl = &levels[level_index];

  gthree_object_destroy_all_children (GTHREE_OBJECT (level_group));
  physics_world_clear (physics);

  marble_body_id = physics_world_add_sphere (physics, MARBLE_RADIUS, MARBLE_MASS,
                                             lvl->start_x, lvl->start_y, lvl->start_z,
                                             0.8f, 0.15f);
  physics_world_set_damping (physics, marble_body_id, 0.15f, 0.3f);

  goal_mesh = NULL;

  for (int i = 0; i < lvl->n_platforms; i++)
    {
      const Platform *p = &lvl->platforms[i];

      float rot_x = 0, rot_y = 0, rot_z = 0;
      switch (p->type)
        {
        case PLATFORM_RAMP_X_POS: rot_z = -RAMP_ANGLE; break;
        case PLATFORM_RAMP_X_NEG: rot_z =  RAMP_ANGLE; break;
        case PLATFORM_RAMP_Z_POS: rot_x =  RAMP_ANGLE; break;
        case PLATFORM_RAMP_Z_NEG: rot_x = -RAMP_ANGLE; break;
        default: break;
        }

      physics_world_add_box (physics, p->x, p->y, p->z,
                             p->w, p->h, p->d,
                             rot_x, rot_y, rot_z,
                             p->type == PLATFORM_BOUNCE ? 0.4f : 0.9f,
                             p->type == PLATFORM_BOUNCE ? 1.5f : 0.1f);

      GthreeGeometry *geo = gthree_geometry_new_box (p->w, p->h, p->d, 1, 1, 1);
      GthreeMeshPhongMaterial *mat = gthree_mesh_phong_material_new ();
      graphene_vec3_t color;

      graphene_vec3_init (&color, p->r, p->g, p->b);
      gthree_mesh_phong_material_set_color (mat, &color);
      gthree_mesh_phong_material_set_shininess (mat, 20);

      if (p->type == PLATFORM_GOAL)
        {
          graphene_vec3_t emissive;
          graphene_vec3_init (&emissive, 0.3f, 0.25f, 0.0f);
          gthree_mesh_phong_material_set_emissive_color (mat, &emissive);
        }

      GthreeMesh *mesh = gthree_mesh_new (geo, GTHREE_MATERIAL (mat));
      gthree_object_set_position_xyz (GTHREE_OBJECT (mesh), p->x, p->y, p->z);
      gthree_object_set_rotation_xyz (GTHREE_OBJECT (mesh), rot_x, rot_y, rot_z);
      gthree_object_set_cast_shadow (GTHREE_OBJECT (mesh), TRUE);
      gthree_object_set_receive_shadow (GTHREE_OBJECT (mesh), TRUE);
      gthree_object_add_child (GTHREE_OBJECT (level_group), GTHREE_OBJECT (mesh));

      if (p->type == PLATFORM_GOAL)
        goal_mesh = mesh;
    }

  char *text = g_strdup_printf ("<span size='large' color='white'>Level %d: %s</span>",
                                level_index + 1, lvl->name);
  gtk_label_set_markup (GTK_LABEL (level_label), text);
  g_free (text);
}

static void
init_scene (void)
{
  graphene_vec3_t bg_color;

  scene = gthree_scene_new ();

  graphene_vec3_init (&bg_color, 0.05f, 0.05f, 0.15f);
  gthree_scene_set_background_color (scene, &bg_color);

  GthreeFog *fog = gthree_fog_new_linear (&bg_color, 60.0f, 150.0f);
  gthree_scene_set_fog (scene, fog);

  camera = gthree_perspective_camera_new (45, 1, 0.5f, 200.0f);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  GthreeAmbientLight *ambient = gthree_ambient_light_new (medium_grey ());
  gthree_light_set_intensity (GTHREE_LIGHT (ambient), G_PI);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient));

  GthreeDirectionalLight *dir_light = gthree_directional_light_new (white (), 0.8f * G_PI);
  gthree_object_set_position_xyz (GTHREE_OBJECT (dir_light), 20, 30, 10);
  gthree_object_set_cast_shadow (GTHREE_OBJECT (dir_light), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (dir_light));

  GthreeLightShadow *shadow = gthree_light_get_shadow (GTHREE_LIGHT (dir_light));
  gthree_light_shadow_set_bias (shadow, -0.001f);
  gthree_light_shadow_set_normal_bias (shadow, 0.02f);
  gthree_light_shadow_set_map_size (shadow, 2048, 2048);
  GthreeCamera *shadow_cam = gthree_light_shadow_get_camera (shadow);
  gthree_orthographic_camera_set_left (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), -60);
  gthree_orthographic_camera_set_right (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), 60);
  gthree_orthographic_camera_set_top (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), 60);
  gthree_orthographic_camera_set_bottom (GTHREE_ORTHOGRAPHIC_CAMERA (shadow_cam), -60);
  gthree_camera_set_far (shadow_cam, 120);

  /* Marble */
  GthreeGeometry *marble_geo = gthree_geometry_new_sphere (MARBLE_RADIUS, 32, 16);
  GthreeMeshPhongMaterial *marble_mat = gthree_mesh_phong_material_new ();
  graphene_vec3_t ms;
  graphene_vec3_init (&ms, 0.4f, 0.4f, 0.4f);
  gthree_mesh_phong_material_set_color (marble_mat, white ());
  gthree_mesh_phong_material_set_specular_color (marble_mat, &ms);
  gthree_mesh_phong_material_set_shininess (marble_mat, 60);

  GthreeTexture *marble_tex = examples_load_texture ("marble-tile-texture.jpg");
  gthree_texture_set_wrap_s (marble_tex, GTHREE_WRAPPING_REPEAT);
  gthree_texture_set_wrap_t (marble_tex, GTHREE_WRAPPING_REPEAT);
  graphene_vec2_t repeat;
  graphene_vec2_init (&repeat, 2.0f, 1.0f);
  gthree_texture_set_repeat (marble_tex, &repeat);
  gthree_mesh_phong_material_set_map (marble_mat, marble_tex);

  marble_mesh = gthree_mesh_new (marble_geo, GTHREE_MATERIAL (marble_mat));
  gthree_object_set_cast_shadow (GTHREE_OBJECT (marble_mesh), TRUE);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (marble_mesh));

  level_group = gthree_group_new ();
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (level_group));
}

/* --- Camera --------------------------------------------------------------- */

static void
update_camera (void)
{
  float mx, my, mz;

  physics_world_get_position (physics, marble_body_id, &mx, &my, &mz);

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  mx - 15.0f, my + 18.0f, mz - 15.0f);
  gthree_object_look_at_xyz (GTHREE_OBJECT (camera), mx, my, mz);
}

/* --- Goal animation ------------------------------------------------------- */

static void
animate_goal (gint64 frame_time)
{
  if (!goal_mesh)
    return;

  float t = (frame_time % 2000000) / 2000000.0f;
  float glow = 0.15f + 0.25f * sinf (t * 2 * G_PI);
  graphene_vec3_t emissive;
  graphene_vec3_init (&emissive, glow, glow * 0.8f, 0);

  GthreeMaterial *mat = gthree_mesh_get_material (goal_mesh, 0);
  gthree_mesh_phong_material_set_emissive_color (GTHREE_MESH_PHONG_MATERIAL (mat), &emissive);
}

/* --- Game state ----------------------------------------------------------- */

static void
set_game_state (GameState new_state)
{
  game_state = new_state;

  switch (new_state)
    {
    case STATE_READY:
      gtk_label_set_markup (GTK_LABEL (status_label),
                            "<span size='xx-large' weight='bold' color='white'>"
                            "Press SPACE to start</span>");
      gtk_widget_set_visible (status_label, TRUE);
      reset_marble ();
      break;

    case STATE_PLAYING:
      gtk_widget_set_visible (status_label, FALSE);
      game_timer = levels[current_level].time_limit;
      break;

    case STATE_WON:
      if (current_level < N_LEVELS - 1)
        gtk_label_set_markup (GTK_LABEL (status_label),
                              "<span size='xx-large' weight='bold' color='#44ff44'>"
                              "Level Complete!\n"
                              "Press SPACE for next level</span>");
      else
        gtk_label_set_markup (GTK_LABEL (status_label),
                              "<span size='xx-large' weight='bold' color='#44ff44'>"
                              "Congratulations!\n"
                              "All levels complete!\n"
                              "Press SPACE to play again</span>");
      gtk_widget_set_visible (status_label, TRUE);
      break;

    case STATE_LOST_FELL:
      gtk_label_set_markup (GTK_LABEL (status_label),
                            "<span size='xx-large' weight='bold' color='#ff4444'>"
                            "Fell off!\n"
                            "Press SPACE to retry</span>");
      gtk_widget_set_visible (status_label, TRUE);
      break;

    case STATE_LOST_TIME:
      gtk_label_set_markup (GTK_LABEL (status_label),
                            "<span size='xx-large' weight='bold' color='#ff4444'>"
                            "Time's up!\n"
                            "Press SPACE to retry</span>");
      gtk_widget_set_visible (status_label, TRUE);
      break;
    }
}

static void
handle_space (void)
{
  switch (game_state)
    {
    case STATE_READY:
      set_game_state (STATE_PLAYING);
      break;

    case STATE_WON:
      if (current_level < N_LEVELS - 1)
        {
          current_level++;
          build_level (current_level);
        }
      else
        {
          current_level = 0;
          build_level (current_level);
        }
      set_game_state (STATE_READY);
      break;

    case STATE_LOST_FELL:
    case STATE_LOST_TIME:
      build_level (current_level);
      set_game_state (STATE_READY);
      break;

    case STATE_PLAYING:
      break;
    }
}

/* --- Input ---------------------------------------------------------------- */

static gboolean
on_key_pressed (GtkEventControllerKey *controller,
                guint keyval, guint keycode,
                GdkModifierType mods,
                gpointer user_data)
{
  switch (keyval)
    {
    case GDK_KEY_Up:    case GDK_KEY_w: key_up    = TRUE; return TRUE;
    case GDK_KEY_Down:  case GDK_KEY_s: key_down  = TRUE; return TRUE;
    case GDK_KEY_Left:  case GDK_KEY_a: key_left  = TRUE; return TRUE;
    case GDK_KEY_Right: case GDK_KEY_d: key_right = TRUE; return TRUE;
    case GDK_KEY_space:
      handle_space ();
      return TRUE;
    }
  return FALSE;
}

static void
on_key_released (GtkEventControllerKey *controller,
                 guint keyval, guint keycode,
                 GdkModifierType mods,
                 gpointer user_data)
{
  switch (keyval)
    {
    case GDK_KEY_Up:    case GDK_KEY_w: key_up    = FALSE; break;
    case GDK_KEY_Down:  case GDK_KEY_s: key_down  = FALSE; break;
    case GDK_KEY_Left:  case GDK_KEY_a: key_left  = FALSE; break;
    case GDK_KEY_Right: case GDK_KEY_d: key_right = FALSE; break;
    }
}

/* --- Tick ----------------------------------------------------------------- */

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock);

  if (last_frame_time == 0)
    last_frame_time = frame_time;

  float dt = (frame_time - last_frame_time) / (float) G_USEC_PER_SEC;
  last_frame_time = frame_time;

  if (dt > 0.05f)
    dt = 0.05f;

  if (game_state == STATE_PLAYING)
    {
      /* Camera looks from (-15, +18, -15), so forward on screen is (+1,0,+1)/sqrt(2) */
      float fx = 0, fz = 0;
      if (key_up)    { fx += MOVE_FORCE * 0.7071f; fz += MOVE_FORCE * 0.7071f; }
      if (key_down)  { fx -= MOVE_FORCE * 0.7071f; fz -= MOVE_FORCE * 0.7071f; }
      if (key_left)  { fx += MOVE_FORCE * 0.7071f; fz -= MOVE_FORCE * 0.7071f; }
      if (key_right) { fx -= MOVE_FORCE * 0.7071f; fz += MOVE_FORCE * 0.7071f; }

      physics_world_apply_central_force (physics, marble_body_id, fx, 0, fz);
      physics_world_step (physics, dt);

      float mx, my, mz;
      physics_world_get_position (physics, marble_body_id, &mx, &my, &mz);
      gthree_object_set_position_xyz (GTHREE_OBJECT (marble_mesh), mx, my, mz);

      float qx, qy, qz, qw;
      physics_world_get_rotation (physics, marble_body_id, &qx, &qy, &qz, &qw);
      graphene_quaternion_t q;
      graphene_quaternion_init (&q, qx, qy, qz, qw);
      gthree_object_set_quaternion (GTHREE_OBJECT (marble_mesh), &q);

      if (my < FALL_THRESHOLD)
        set_game_state (STATE_LOST_FELL);
      else if (marble_on_goal ())
        set_game_state (STATE_WON);

      game_timer -= dt;
      if (game_timer <= 0)
        {
          game_timer = 0;
          set_game_state (STATE_LOST_TIME);
        }

      char *text = g_strdup_printf (
          "<span size='xx-large' weight='bold' color='%s'>%.1f</span>",
          game_timer < 10.0f ? "#ff4444" : "white",
          game_timer);
      gtk_label_set_markup (GTK_LABEL (timer_label), text);
      g_free (text);
    }

  animate_goal (frame_time);
  update_camera ();

  gtk_widget_queue_draw (widget);

  return G_SOURCE_CONTINUE;
}

/* --- GTK callbacks -------------------------------------------------------- */

static void
resize_area (GthreeArea *area, gint width, gint height)
{
  gthree_perspective_camera_set_aspect (camera, (float) width / (float) height);
}

static void
realize_area (GthreeArea *area)
{
  GthreeRenderer *renderer = gthree_area_get_renderer (area);
  gthree_renderer_set_shadow_map_enabled (renderer, TRUE);
}

static void
on_destroy (GtkWidget *widget, gpointer data)
{
  gboolean *done = data;
  *done = TRUE;
  g_main_context_wakeup (NULL);
}

/* --- Main ----------------------------------------------------------------- */

int
main (int argc, char *argv[])
{
  GtkWidget *window, *overlay, *area;
  gboolean done = FALSE;

  gtk_init ();

  window = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Marble Madness");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  g_signal_connect (window, "destroy", G_CALLBACK (on_destroy), &done);

  physics = physics_world_new (-30.0f);

  init_scene ();

  overlay = gtk_overlay_new ();
  gtk_widget_set_hexpand (overlay, TRUE);
  gtk_widget_set_vexpand (overlay, TRUE);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), NULL);
  g_signal_connect_after (area, "realize", G_CALLBACK (realize_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);

  gtk_overlay_set_child (GTK_OVERLAY (overlay), area);

  /* HUD labels */
  timer_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (timer_label),
                        "<span size='xx-large' weight='bold' color='white'>60.0</span>");
  gtk_widget_set_halign (timer_label, GTK_ALIGN_END);
  gtk_widget_set_valign (timer_label, GTK_ALIGN_START);
  gtk_widget_set_margin_top (timer_label, 10);
  gtk_widget_set_margin_end (timer_label, 15);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), timer_label);

  level_label = gtk_label_new (NULL);
  gtk_widget_set_halign (level_label, GTK_ALIGN_START);
  gtk_widget_set_valign (level_label, GTK_ALIGN_START);
  gtk_widget_set_margin_top (level_label, 10);
  gtk_widget_set_margin_start (level_label, 15);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), level_label);

  status_label = gtk_label_new (NULL);
  gtk_widget_set_halign (status_label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (status_label, GTK_ALIGN_CENTER);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), status_label);

  gtk_window_set_child (GTK_WINDOW (window), overlay);

  current_level = 0;
  build_level (current_level);

  /* Keyboard input */
  {
    GtkEventController *key = gtk_event_controller_key_new ();
    g_signal_connect (key, "key-pressed", G_CALLBACK (on_key_pressed), NULL);
    g_signal_connect (key, "key-released", G_CALLBACK (on_key_released), NULL);
    gtk_widget_add_controller (window, key);
  }

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  set_game_state (STATE_READY);

  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  physics_world_free (physics);

  return EXIT_SUCCESS;
}
