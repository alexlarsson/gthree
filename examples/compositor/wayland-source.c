/*
 * Wayland GLib Integration
 *
 * Based on casilda-wayland-source by Juan Pablo Ugarte
 * Copyright (C) 2024  Juan Pablo Ugarte
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "wayland-source.h"

typedef struct
{
  GSource            source;
  struct wl_display *display;
} WaylandSource;

#define WAYLAND_SOURCE(s) ((WaylandSource *) s)

static gboolean
wayland_source_prepare (GSource *base, int *timeout)
{
  WaylandSource *source = WAYLAND_SOURCE (base);

  *timeout = -1;

  wl_display_flush_clients (source->display);

  return FALSE;
}

static gboolean
wayland_source_check (GSource *base)
{
  WaylandSource *source = WAYLAND_SOURCE (base);
  struct wl_event_loop *loop = wl_display_get_event_loop (source->display);

  wl_event_loop_dispatch_idle (loop);

  return FALSE;
}

static gboolean
wayland_source_dispatch (GSource                  *base,
                         G_GNUC_UNUSED GSourceFunc callback,
                         G_GNUC_UNUSED void       *data)
{
  WaylandSource *source = WAYLAND_SOURCE (base);
  struct wl_event_loop *loop = wl_display_get_event_loop (source->display);

  wl_event_loop_dispatch (loop, 0);

  return TRUE;
}

static GSourceFuncs wayland_source_funcs = {
  .prepare = wayland_source_prepare,
  .check = wayland_source_check,
  .dispatch = wayland_source_dispatch,
};

GSource *
wayland_source_new (struct wl_display *display)
{
  struct wl_event_loop *loop = wl_display_get_event_loop (display);
  GSource *source = g_source_new (&wayland_source_funcs,
                                  sizeof (WaylandSource));

  WAYLAND_SOURCE (source)->display = display;

  g_source_add_unix_fd (source,
                        wl_event_loop_get_fd (loop),
                        G_IO_IN | G_IO_ERR);

  return source;
}
