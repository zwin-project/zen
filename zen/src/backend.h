#pragma once

#include <wayland-server-core.h>

#include "zen/backend.h"

void zn_backend_update_capabilities(struct zn_backend *self);

bool zn_backend_start(struct zn_backend *self);

struct zn_backend *zn_backend_create(struct wl_display *display);

void zn_backend_destroy(struct zn_backend *self);
