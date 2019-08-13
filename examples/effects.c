#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeEffectComposer *composer;
GthreeScene *scene;
GthreeMesh *mesh;
GthreePerspectiveCamera *camera;
GthreeMesh *wire;
GthreeMesh *mesh2;
GthreeScene *scene2;
GthreePerspectiveCamera *camera2;
GthreeMeshBasicMaterial *material;

GthreePass *clear_pass;
GthreePass *psycho_pass;
GthreePass *clear_depth_pass;
GthreePass *render_pass;
GthreePass *render2_pass;
GthreePass *bloom_pass;
GthreePass *greyscale_pass;

GthreeRenderTarget *render_target;

void
init_scene (void)
{
  GthreeGeometry *geometry;
  graphene_point3d_t pos;
  GdkPixbuf *crate_pixbuf;
  GthreeTexture *texture;

  geometry = gthree_geometry_new_box (80, 80, 80, 1, 1, 1);

  scene = gthree_scene_new ();

  crate_pixbuf = examples_load_pixbuf ("crate.gif");
  texture = gthree_texture_new (crate_pixbuf);

  material = gthree_mesh_basic_material_new ();
  gthree_mesh_basic_material_set_map (GTHREE_MESH_BASIC_MATERIAL (material), texture);
  mesh = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position_point3d (GTHREE_OBJECT (mesh),
                              graphene_point3d_init (&pos, 0, 20, 0));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (mesh));

  material = gthree_mesh_basic_material_new ();
  gthree_mesh_material_set_is_wireframe (GTHREE_MESH_MATERIAL (material), TRUE);
  gthree_mesh_material_set_wireframe_line_width (GTHREE_MESH_MATERIAL (material), 3.0);
  gthree_mesh_basic_material_set_color (GTHREE_MESH_BASIC_MATERIAL (material), yellow ());
  wire = gthree_mesh_new (geometry, GTHREE_MATERIAL (material));
  gthree_object_set_position_point3d (GTHREE_OBJECT (wire),
                              graphene_point3d_init (&pos, -80,20, 0));

  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (wire));

  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_set_position_point3d (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));
}

void
init_scene2 (void)
{
  GthreeGeometry *geometry;
  graphene_point3d_t pos;
  GthreeMeshPhongMaterial *material_phong;
  GthreeAmbientLight *ambient_light;
  GthreeDirectionalLight *directional_light;

  geometry = gthree_geometry_new_sphere (40, 40, 16);

  scene2 = gthree_scene_new ();

  material_phong = gthree_mesh_phong_material_new ();
  gthree_mesh_phong_material_set_color (material_phong, orange ());
  gthree_mesh_phong_material_set_specular_color (material_phong, red ());
  gthree_mesh_phong_material_set_shininess (material_phong, 30);

  mesh2 = gthree_mesh_new (geometry, GTHREE_MATERIAL (material_phong));
  gthree_object_set_scale_point3d (GTHREE_OBJECT (mesh2),
                           graphene_point3d_init (&pos, 1.0, 1.4, 1.0));
  gthree_object_set_position_point3d (GTHREE_OBJECT (mesh2),
                              graphene_point3d_init (&pos, 40, -20, 0));

  gthree_object_add_child (GTHREE_OBJECT (scene2), GTHREE_OBJECT (mesh2));

  ambient_light = gthree_ambient_light_new (dark_grey ());
  gthree_object_add_child (GTHREE_OBJECT (scene2), GTHREE_OBJECT (ambient_light));

  directional_light = gthree_directional_light_new (white (), 0.65);
  gthree_object_set_position_point3d (GTHREE_OBJECT (directional_light),
                              graphene_point3d_init (&pos,
                                                     1, 1, -1));
  gthree_object_add_child (GTHREE_OBJECT (scene2), GTHREE_OBJECT (directional_light));

  camera2 = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_set_position_point3d (GTHREE_OBJECT (camera2),
                              graphene_point3d_init (&pos, 0, 0, 400));
  gthree_object_add_child (GTHREE_OBJECT (scene2), GTHREE_OBJECT (camera2));
}


static const char *vertex_shader =
  "varying vec2 vUv;\n"
  "void main()\n"
  "{\n"
  "  vUv = uv;\n"
  "  vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );\n"
  "  gl_Position = projectionMatrix * mvPosition;\n"
  "}\n";


static const char *fragment_psycho_shader =
  "uniform float time;\n"
  "varying vec2 vUv;\n"
  "void main(void)\n"
  "{\n"
  "  vec2 p = -1.0 + 2.0 * vUv;\n"
  "  float a = time*40.0;\n"
  "  float d,e,f,g=1.0/40.0,h,i,r,q;\n"
  "  e=400.0*(p.x*0.5+0.5);\n"
  "  f=400.0*(p.y*0.5+0.5);\n"
  "  i=200.0+sin(e*g+a/150.0)*20.0;\n"
  "  d=200.0+cos(f*g/2.0)*18.0+cos(e*g)*7.0;\n"
  "  r=sqrt(pow(i-e,2.0)+pow(d-f,2.0));\n"
  "  q=f/r;\n"
  "  e=(r*cos(q))-a/2.0;f=(r*sin(q))-a/2.0;\n"
  "  d=sin(e*g)*176.0+sin(e*g)*164.0+r;\n"
  "  h=((f+d)+a/2.0)*g;\n"
  "  i=cos(h+r*p.x/1.3)*(e+e+a)+cos(q*g*6.0)*(r+h/3.0);\n"
  "  h=sin(f*g)*144.0-sin(e*g)*212.0*p.x;\n"
  "  h=(h+(f-e)*q+sin(r-(a+h)/7.0)*10.0+i/4.0)*g;\n"
  "  i+=cos(h*2.3*sin(a/350.0-q))*184.0*sin(q-(r*4.3+a/12.0)*g)+tan(r*g+h)*184.0*cos(r*g+h);\n"
  "  i=mod(i/5.6,256.0)/64.0;\n"
  "  if(i<0.0) i+=4.0;\n"
  "  if(i>=2.0) i=4.0-i;\n"
  "  d=r/350.0;\n"
  "  d+=sin(d*d*8.0)*0.52;\n"
  "  f=(sin(a*g)+1.0)/2.0;\n"
  "  gl_FragColor=vec4(vec3(i/2.0+d/13.0,i,f*i/1.6)*d*p.x*0.5+vec3(i/1.2+d/18.0,i,i/1.3+d/8.0)*d*(1.0-p.x)*0.3,1.0);\n"
  "}\n";

float f1 = 1;
static GthreeUniformsDefinition psycho_shader_uniforms_defs[] = {
   {"time", GTHREE_UNIFORM_TYPE_FLOAT, &f1},
};
static GthreeUniforms *psycho_shader_uniforms;

static const char *fragment_greyscale_shader =
  "uniform sampler2D tDiffuse;\n"
  "varying vec2 vUv;\n"
  "void main(void)\n"
  "{\n"
  "vec4 cTextureScreen = texture2D( tDiffuse, vUv );\n"
  "vec3 cResult = cTextureScreen.rgb;\n"
  "cResult = vec3( cResult.r * 0.3 + cResult.g * 0.59 + cResult.b * 0.11 );\n"
  "gl_FragColor =  vec4(cResult, 1);\n"
  "}\n";

static GthreeUniformsDefinition greyscale_shader_uniforms_defs[] = {
  {"tDiffuse", GTHREE_UNIFORM_TYPE_TEXTURE, NULL },
};
static GthreeUniforms *greyscale_shader_uniforms;

void
init_composer (void)
{
  GthreeShader *psycho_shader, *shader2;

  psycho_shader_uniforms = gthree_uniforms_new_from_definitions (psycho_shader_uniforms_defs,
                                                           G_N_ELEMENTS (psycho_shader_uniforms_defs));
  greyscale_shader_uniforms = gthree_uniforms_new_from_definitions (greyscale_shader_uniforms_defs,
                                                           G_N_ELEMENTS (greyscale_shader_uniforms_defs));
  psycho_shader = gthree_shader_new (NULL, psycho_shader_uniforms,
                                     vertex_shader,
                                     fragment_psycho_shader);
  shader2 = gthree_shader_new (NULL, greyscale_shader_uniforms,
                               vertex_shader,
                               fragment_greyscale_shader);

  clear_pass = gthree_clear_pass_new (black ());
  psycho_pass = gthree_shader_pass_new (psycho_shader, NULL);
  gthree_pass_set_clear (psycho_pass, FALSE);
  bloom_pass = gthree_bloom_pass_new (2, 4.0, 256);
  gthree_pass_set_enabled (bloom_pass, FALSE);
  greyscale_pass = gthree_shader_pass_new (shader2, NULL);
  gthree_pass_set_enabled (greyscale_pass, FALSE);

  clear_depth_pass = gthree_clear_pass_new (NULL);
  gthree_pass_set_clear (clear_depth_pass, FALSE);
  gthree_clear_pass_set_clear_depth (GTHREE_CLEAR_PASS (clear_depth_pass), TRUE);

  render_pass = gthree_render_pass_new (scene, GTHREE_CAMERA (camera), NULL);
  gthree_pass_set_clear (render_pass, FALSE);

  render2_pass = gthree_render_pass_new (scene2, GTHREE_CAMERA (camera2), NULL);
  gthree_pass_set_clear (render2_pass, FALSE);
  gthree_render_pass_set_clear_depth (GTHREE_RENDER_PASS (render2_pass), TRUE);

  composer = gthree_effect_composer_new  ();

  gthree_effect_composer_add_pass  (composer, clear_pass);
  gthree_effect_composer_add_pass  (composer, psycho_pass);
  gthree_effect_composer_add_pass  (composer, clear_depth_pass);
  gthree_effect_composer_add_pass  (composer, render_pass);
  gthree_effect_composer_add_pass  (composer, bloom_pass);
  gthree_effect_composer_add_pass  (composer, render2_pass);
  gthree_effect_composer_add_pass  (composer, greyscale_pass);
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static gint64 first_frame_time = 0;
  gint64 frame_time;
  float relative_time;
  graphene_euler_t euler;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == 0)
    first_frame_time = frame_time;

  /* This converts to a (float) count of ideal 60hz frames, just so we
     can use some nice numbers when defining animation speed below */
  relative_time = (frame_time - first_frame_time) * 60 / (float) G_USEC_PER_SEC;

  gthree_object_set_rotation (GTHREE_OBJECT (mesh),
                              graphene_euler_init (&euler,
                                                   0.0 * relative_time,
                                                   2.0 * relative_time,
                                                   1.0 * relative_time
                                                   ));

  gthree_object_set_rotation (GTHREE_OBJECT (wire),
                              graphene_euler_init (&euler,
                                                   2.0 * relative_time,
                                                   1.0 * relative_time,
                                                   1.0 * relative_time
                                                   ));

  gthree_object_set_rotation (GTHREE_OBJECT (mesh2),
                              graphene_euler_init (&euler,
                                                   2.0 * relative_time,
                                                   1.0 * relative_time,
                                                   3.0 * relative_time
                                                   ));

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

static gboolean
render_area (GtkGLArea    *gl_area,
             GdkGLContext *context)
{
  gthree_effect_composer_render (composer, gthree_area_get_renderer (GTHREE_AREA(gl_area)),
                                 0.1);
  return TRUE;
}

static void
pass_toggled (GtkToggleButton *toggle_button, GthreePass *pass)
{
  gboolean enabled = gtk_toggle_button_get_active (toggle_button);

  gthree_pass_set_enabled (pass, enabled);
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area, *check;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Effects");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (box), 6);
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show (box);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_container_add (GTK_CONTAINER (box), hbox);
  gtk_widget_show (hbox);

  init_scene ();
  init_scene2 ();
  init_composer ();

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  g_signal_connect (area, "render", G_CALLBACK (render_area), NULL);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE);
  gtk_box_set_spacing (GTK_BOX (hbox), 6);
  gtk_container_add (GTK_CONTAINER (box), hbox);
  gtk_widget_show (hbox);

  check = gtk_check_button_new_with_label ("Fancy effect");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (pass_toggled), psycho_pass);

  check = gtk_check_button_new_with_label ("Scene 1");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (pass_toggled), render_pass);

  check = gtk_check_button_new_with_label ("Bloom");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (pass_toggled), bloom_pass);

  check = gtk_check_button_new_with_label ("Scene 2");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (pass_toggled), render2_pass);

  check = gtk_check_button_new_with_label ("Greyscale");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), FALSE);
  gtk_container_add (GTK_CONTAINER (hbox), check);
  gtk_widget_show (check);
  g_signal_connect (check, "toggled", G_CALLBACK (pass_toggled), greyscale_pass);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
