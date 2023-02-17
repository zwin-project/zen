#pragma once

#include <wayland-server-core.h>

struct zn_backend;

struct zn_backend *zn_default_backend_create(struct wl_display *display);
