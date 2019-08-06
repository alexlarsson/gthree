#include <stdlib.h>
#include <gtk/gtk.h>

#include <epoxy/gl.h>

#include <gthree/gthree.h>
#include "utils.h"

GthreeScene *scene;
GthreeMeshBasicMaterial *material_simple;
GthreeMeshBasicMaterial *material_texture;
GthreeMeshBasicMaterial *material_face_color;
GthreeMeshBasicMaterial *material_vertex_color;
GthreeMeshBasicMaterial *material_wireframe;
GthreeMesh *mesh;
double rot = 0;

GList *cubes;


static const char *vertex_shader =
  "varying vec2 vUv;\n"
  "void main()\n"
  "{\n"
  "  vUv = uv;\n"
  "  vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );\n"
  "  gl_Position = projectionMatrix * mvPosition;\n"
  "}\n";


static const char *fragment_shader1 =
  "uniform vec2 resolution;\n"
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
  "  gl_FragColor=vec4(vec3(f*i/1.6,i/2.0+d/13.0,i)*d*p.x+vec3(i/1.3+d/8.0,i/2.0+d/18.0,i)*d*(1.0-p.x),1.0);\n"
  "}\n";

static const char *fragment_shader2 =
  "uniform float time;\n"
  "uniform vec2 resolution;\n"
  "uniform sampler2D texture;\n"
  "varying vec2 vUv;\n"

  "void main( void ) {\n"
  "  vec2 position = -1.0 + 2.0 * vUv;\n"

  "  float a = atan( position.y, position.x );\n"
  "  float r = sqrt( dot( position, position ) );\n"

  "  vec2 uv;\n"
  "  uv.x = cos( a ) / r;\n"
  "  uv.y = sin( a ) / r;\n"
  "  uv /= 10.0;\n"
  "  uv += time * 0.05;\n"

  "  vec3 color = texture2D( texture, uv ).rgb;\n"

  "  gl_FragColor = vec4( color * r * 1.5, 1.0 );\n"
  "}\n";


static const char *fragment_shader3 =
  "uniform float time;\n"
  "uniform vec2 resolution;\n"

  "varying vec2 vUv;\n"

  "void main( void ) {\n"
  "  vec2 position = vUv;\n"

  "  float color = 0.0;\n"
  "  color += sin( position.x * cos( time / 15.0 ) * 80.0 ) + cos( position.y * cos( time / 15.0 ) * 10.0 );\n"
  "  color += sin( position.y * sin( time / 10.0 ) * 40.0 ) + cos( position.x * sin( time / 25.0 ) * 40.0 );\n"
  "  color += sin( position.x * sin( time / 5.0 ) * 10.0 ) + sin( position.y * sin( time / 35.0 ) * 80.0 );\n"
  "  color *= sin( time / 10.0 ) * 0.5;\n"

  "  gl_FragColor = vec4( vec3( color, color * 0.5, sin( color + time / 3.0 ) * 0.75 ), 1.0 );\n"
  "}\n";

static const char *fragment_shader4 =
  "uniform float time;\n"
  "uniform vec2 resolution;\n"

  "varying vec2 vUv;\n"

  "void main( void ) {\n"
  "  vec2 position = -1.0 + 2.0 * vUv;\n"

  "  float red = abs( sin( position.x * position.y + time / 5.0 ) );\n"
  "  float green = abs( sin( position.x * position.y + time / 4.0 ) );\n"
  "  float blue = abs( sin( position.x * position.y + time / 3.0 ) );\n"
  "  gl_FragColor = vec4( red, green, blue, 1.0 );\n"
  "}\n";

static GthreeObject *
new_cube (GthreeMaterial *material)
{
  static GthreeGeometry *geometry = NULL;
  GthreeMesh *mesh;

  if (geometry == NULL)
    geometry = gthree_geometry_new_box (60, 60, 60, 1, 1, 1);

  mesh = gthree_mesh_new (geometry, material);

  return GTHREE_OBJECT (mesh);
}

float f1 = 1.0;
float v2[2] = {0, 0};

static GthreeUniformsDefinition shader1_uniforms_defs[] = {
  {"time", GTHREE_UNIFORM_TYPE_FLOAT, &f1},
  {"resolution", GTHREE_UNIFORM_TYPE_VEC2_ARRAY, NULL},
};
static GthreeUniforms *shader1_uniforms;

static GthreeUniformsDefinition shader2_uniforms_defs[] = {
  {"time", GTHREE_UNIFORM_TYPE_FLOAT, &f1},
  {"resolution", GTHREE_UNIFORM_TYPE_VEC2_ARRAY, NULL},
  {"texture", GTHREE_UNIFORM_TYPE_TEXTURE, NULL},
};
static GthreeUniforms *shader2_uniforms;

GthreeScene *
init_scene (void)
{
  GthreeObject *cube;
  graphene_point3d_t pos = { 0, 0, 0};
  GthreeShader *shader;
  GthreeShaderMaterial *material;
  GthreeTexture *texture;
  GdkPixbuf *disturb;

  disturb = examples_load_pixbuf ("disturb.jpg");

  scene = gthree_scene_new ();

  pos.x = -100;
  pos.y = 40;

  shader1_uniforms = gthree_uniforms_new_from_definitions (shader1_uniforms_defs, G_N_ELEMENTS (shader1_uniforms_defs));
  shader = gthree_shader_new (NULL, shader1_uniforms,
                              vertex_shader,
                              fragment_shader1);

  material = gthree_shader_material_new (shader);

  cube = new_cube (GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 70;
  pos.y -= 80;

  texture = gthree_texture_new (disturb);
  shader2_uniforms = gthree_uniforms_new_from_definitions (shader2_uniforms_defs, G_N_ELEMENTS (shader2_uniforms_defs));
  gthree_uniforms_set_texture (shader2_uniforms, "texture", texture);

  shader = gthree_shader_new (NULL, shader2_uniforms,
                              vertex_shader,
                              fragment_shader2);

  material = gthree_shader_material_new (shader);

  cube = new_cube (GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 70;
  pos.y += 80;

  shader = gthree_shader_new (NULL, shader1_uniforms,
                              vertex_shader,
                              fragment_shader3);

  material = gthree_shader_material_new (shader);

  cube = new_cube (GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  pos.x += 70;
  pos.y -= 80;

  shader = gthree_shader_new (NULL, shader1_uniforms,
                              vertex_shader,
                              fragment_shader4);

  material = gthree_shader_material_new (shader);

  cube = new_cube (GTHREE_MATERIAL (material));
  gthree_object_add_child (GTHREE_OBJECT (scene), cube);
  gthree_object_set_position (GTHREE_OBJECT (cube), &pos);
  cubes = g_list_prepend (cubes, cube);

  return scene;
}

static gboolean
tick (GtkWidget     *widget,
      GdkFrameClock *frame_clock,
      gpointer       user_data)
{
  static graphene_point3d_t rot = { 0, 0, 0};
  static gint64 first_frame_time = -1;
  double time;
  gint64 frame_time;

  GList *l;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);
  if (first_frame_time == -1)
    first_frame_time = frame_time;
  frame_time -= first_frame_time;

  time = frame_time / 400000.0;
  gthree_uniforms_set_float (shader1_uniforms, "time", time);
  gthree_uniforms_set_float (shader2_uniforms, "time", time / 5.0);

  rot.y += 2.0;
  rot.z += 1.0;

  for (l = cubes; l != NULL; l = l->next)
    {
      GthreeObject *cube = l->data;
      graphene_euler_t euler;

      rot.y = -rot.y;

      gthree_object_set_rotation (cube,
                                  graphene_euler_init (&euler, rot.x, rot.y, rot.z));
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
}

int
main (int argc, char *argv[])
{
  GtkWidget *window, *box, *hbox, *button, *area;
  GthreeScene *scene;
  GthreePerspectiveCamera *camera;
  graphene_point3d_t pos;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Shader");
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

  scene = init_scene ();
  camera = gthree_perspective_camera_new (30, 1, 1, 10000);
  gthree_object_add_child (GTHREE_OBJECT (scene), GTHREE_OBJECT (camera));

  gthree_object_set_position (GTHREE_OBJECT (camera),
                              graphene_point3d_init (&pos, 0, 0, 400));

  area = gthree_area_new (scene, GTHREE_CAMERA (camera));
  g_signal_connect (area, "resize", G_CALLBACK (resize_area), camera);
  gtk_widget_set_hexpand (area, TRUE);
  gtk_widget_set_vexpand (area, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox), area);
  gtk_widget_show (area);

  gtk_widget_add_tick_callback (GTK_WIDGET (area), tick, area, NULL);

  button = gtk_button_new_with_label ("Quit");
  gtk_widget_set_hexpand (button, TRUE);
  gtk_container_add (GTK_CONTAINER (box), button);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_widget_show (button);

  gtk_widget_show (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
