/* gthreeambientlight.h: Ambient light
 *
 * Copyright 2019  Alex Larsson
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <gthree/gthreelight.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_AMBIENT_LIGHT (gthree_ambient_light_get_type())

GTHREE_API
G_DECLARE_DERIVABLE_TYPE (GthreeAmbientLight, gthree_ambient_light, GTHREE, AMBIENT_LIGHT, GthreeLight)

struct _GthreeAmbientLightClass
{
  GthreeLightClass parent_class;
};

GTHREE_API
GthreeAmbientLight *gthree_ambient_light_new (const graphene_vec3_t *color);

G_END_DECLS
