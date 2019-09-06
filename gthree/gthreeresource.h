/* gthreeresource.h: Resources
 *
 * Copyright 2019  Alexander Larsson
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if !defined (__GTHREE_H_INSIDE__) && !defined (GTHREE_COMPILATION)
#error "Only <gthree/gthree.h> can be included directly."
#endif

#include <glib-object.h>
#include <gdk/gdk.h>
#include <gthree/gthreetypes.h>

G_BEGIN_DECLS

#define GTHREE_TYPE_RESOURCE (gthree_resource_get_type ())

GTHREE_API
G_DECLARE_DERIVABLE_TYPE (GthreeResource, gthree_resource, GTHREE, RESOURCE, GObject)

struct _GthreeResourceClass {
  GObjectClass parent_class;

  void (*set_used) (GthreeResource *resource,
                    gboolean        used);
  void (*unrealize) (GthreeResource *resource);

  gpointer padding[8];
};

GTHREE_API
void gthree_resources_flush_deletes        (GdkGLContext *context);
GTHREE_API
void gthree_resources_unrealize_all_for    (GdkGLContext *context);
GTHREE_API
void gthree_resources_set_all_unused_for   (GdkGLContext *context);
GTHREE_API
void gthree_resources_unrealize_unused_for (GdkGLContext *context);

GTHREE_API
void     gthree_resource_set_realized_for (GthreeResource *resource,
                                           GdkGLContext   *context);
GTHREE_API
gboolean gthree_resource_is_realized      (GthreeResource *resource);
GTHREE_API
void     gthree_resource_unrealize        (GthreeResource *resource);
GTHREE_API
gboolean gthree_resource_get_used        (GthreeResource *resource);
GTHREE_API
void     gthree_resource_set_used         (GthreeResource *resource,
                                           gboolean        used);

G_END_DECLS
