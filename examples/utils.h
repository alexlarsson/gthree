#include <gthree/gthree.h>

GdkPixbuf *examples_load_pixbuf (const char *file);
GthreeGeometry *examples_load_geometry (const char *name);
void examples_load_cube_pixbufs (const char *dir,
                                 GdkPixbuf *pixbufs[6]);

extern GdkRGBA red;
extern GdkRGBA green;
extern GdkRGBA blue;
extern GdkRGBA yellow;
extern GdkRGBA cyan;
extern GdkRGBA magenta;
extern GdkRGBA white;
extern GdkRGBA black;

extern GdkRGBA very_dark_grey;
extern GdkRGBA dark_grey;
extern GdkRGBA medium_grey;
extern GdkRGBA grey;
extern GdkRGBA light_grey;

extern GdkRGBA dark_green;
extern GdkRGBA orange;
