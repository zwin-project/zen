#pragma once

#include <wayland-server-core.h>

/// @return true if successful, false otherwise
bool zn_shm_init(struct wl_display *display);
