/*
 * Wayland GLib Integration
 *
 * Based on casilda-wayland-source by Juan Pablo Ugarte
 * Copyright (C) 2024  Juan Pablo Ugarte
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

#include <glib.h>
#include <wayland-server-core.h>

GSource *wayland_source_new (struct wl_display *display);
