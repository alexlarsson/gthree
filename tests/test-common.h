#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include <gthree/gthree.h>

typedef void (*TestSetupFunc) (GthreeScene **scene, GthreeCamera **camera);
typedef void (*TestRendererFunc) (GthreeRenderer *renderer);
typedef void (*TestPreRenderFunc) (GthreeRenderer *renderer);

typedef struct {
  const char *name;
  TestSetupFunc setup;
  TestRendererFunc renderer_setup;
  TestPreRenderFunc pre_render;
} TestCase;

void register_test (const char *name, TestSetupFunc setup);
void register_test_full (const char *name, TestSetupFunc setup, TestRendererFunc renderer_setup);
void register_test_with_pre_render (const char *name, TestSetupFunc setup,
                                    TestRendererFunc renderer_setup,
                                    TestPreRenderFunc pre_render);

int test_main (int argc, char *argv[]);

/* Helpers */
GthreeTexture *test_load_texture (const char *filename);
GthreeGeometry *test_geometry_box (void);
GthreeGeometry *test_geometry_sphere (void);
GthreeGeometry *test_geometry_plane (void);
GthreeGeometry *test_geometry_torus_knot (void);
GthreeCubeTexture *test_cube_texture_colored (void);

#endif
