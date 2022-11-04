#pragma once

#include <wayland-server-core.h>

struct zn_appearance;

struct zn_appearance* zn_appearance_create(struct wl_display* display);

void zn_appearance_destroy(struct zn_appearance* self);
