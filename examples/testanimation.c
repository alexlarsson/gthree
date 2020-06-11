#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

static GthreeAnimationMixer *mixer;

GthreeScene *
init_scene (void)
{
  GthreeScene *scene;
  GthreeGeometry *geometry = NULL;
  GthreeMesh *mesh = NULL;
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;
  GthreeMeshLambertMaterial *material;
  GthreeAnimationClip *clip;
  GthreeAnimationAction *action;
  GthreeKeyframeTrack *pos_track;
  GthreeKeyframeTrack *rot_track;
  GthreeKeyframeTrack *scale_track;
  GthreeAttributeArray *times;
  GthreeAttributeArray *pos_values;
  GthreeAttributeArray *rot_values;
  GthreeAttributeArray *scale_values;
  float times_data[] = {  0, 5, 7, 10 };
  float pos_data[] = {
    0,  0,  0,
    -40, 40, 0,
    40, 40,  0,
    40, -40, 0,
  };
  float rot_data[] = {
    0,  0,  0,
    90, 0 , 0,
    0, 90,  0,
    0, 0,  90,
  };
  float scale_data[] = {
    1.0, 1.0, 1.0,
    2.0, 1.0, 1.0,
    0.5, 1.0, 1.0,
    1.0, 1.0, 0.2,
  };

  scene = gthree_scene_new ();

  ambient_light = gthree_ambient_light_new (red ());
  gthree_light_set_intensity (GTHREE_LIGHT (ambient_light), 0.2);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (ambient_light));

  directional_light = gthree_directional_light_new (blue (), 1.2);
  gthree_object_set_position_xyz (GTHREE_OBJECT (directional_light),
                                  1, 1, -1);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (directional_light));

  geometry = gthree_geometry_new_box (60, 60, 60, 1, 1, 1);
  material = gthree_mesh_lambert_material_new ();
  gthree_mesh_lambert_material_set_color (material, white ());
  mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

  clip = gthree_animation_clip_new ("testclip", 10);

  times = gthree_attribute_array_new_from_float (times_data, 4, 1);
  pos_values = gthree_attribute_array_new_from_float (pos_data, 4, 3);
  rot_values = gthree_attribute_array_new_from_float (rot_data, 4, 3);
  scale_values = gthree_attribute_array_new_from_float (scale_data, 4, 3);

  pos_track = gthree_vector_keyframe_track_new (".position", times, pos_values);
  gthree_animation_clip_add_track (clip, pos_track);

  rot_track = gthree_vector_keyframe_track_new (".rotation", times, rot_values);
  gthree_animation_clip_add_track (clip, rot_track);

  scale_track = gthree_vector_keyframe_track_new (".scale", times, scale_values);
  gthree_animation_clip_add_track (clip, scale_track);

  mixer = gthree_animation_mixer_new (GTHREE_OBJECT (mesh));
  gthree_animation_mixer_set_time_scale (mixer, 5);

  action = gthree_animation_mixer_clip_action (mixer, clip, NULL);
  gthree_animation_action_set_loop_mode (action, GTHREE_LOOP_MODE_PINGPONG, -1);

  gthree_animation_action_play (action);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 last_frame_time = 0;
  gint64 frame_time;
  float delta_time_sec;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (last_frame_time != 0)
    {
      delta_time_sec = (frame_time - last_frame_time) / (float) G_USEC_PER_SEC;
      gthree_animation_mixer_update (mixer, delta_time_sec);
    }

  last_frame_time = frame_time;

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
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  gboolean done = FALSE;

  window = examples_init ("Animations", &box, &done);

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position_xyz (GTHREE_OBJECT (camera),
                                  0, 0, 400);

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
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
