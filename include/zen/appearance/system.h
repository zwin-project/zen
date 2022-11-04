#pragma once

#include <wayland-server-core.h>

struct zna_system;

struct zna_system* zna_system_create(struct wl_display* display);

void zna_system_destroy(struct zna_system* self);
