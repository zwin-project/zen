#pragma once

#include <wayland-server-core.h>

#include "zen/renderer/system.h"

struct zna_system;

struct zna_system* zna_system_create(
    struct wl_display* display, struct znr_system* renderer);

void zna_system_destroy(struct zna_system* self);
