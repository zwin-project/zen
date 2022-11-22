#pragma once

#include <wayland-server-core.h>

struct zn_shell;

struct zn_shell* zn_shell_create(struct wl_display* display);

void zn_shell_destroy(struct zn_shell* self);
