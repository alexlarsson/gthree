/* gthreearea.h: Gthree widget
 *
 * Copyright 2019  Alex Larsson
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gthree/gthreescene.h>
#include <gthree/gthreecamera.h>
#include <gthree/gthreerenderer.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_AREA (gthree_area_get_type())

GTHREE_API
G_DECLARE_DERIVABLE_TYPE (GthreeArea, gthree_area, GTHREE, AREA, GtkGLArea)

struct _GthreeAreaClass
{
  GtkGLAreaClass parent_class;
};

GTHREE_API
GtkWidget *     gthree_area_new          (GthreeScene  *scene,
                                          GthreeCamera *camera);

GTHREE_API
GthreeScene *   gthree_area_get_scene    (GthreeArea   *area);
GTHREE_API
void            gthree_area_set_scene    (GthreeArea   *area,
                                          GthreeScene  *scene);
GTHREE_API
GthreeCamera *  gthree_area_get_camera   (GthreeArea   *area);
GTHREE_API
void            gthree_area_set_camera   (GthreeArea   *area,
                                          GthreeCamera *camera);
GTHREE_API
GthreeRenderer *gthree_area_get_renderer (GthreeArea   *area);

G_END_DECLS
