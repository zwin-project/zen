#pragma once

#include <wayland-server-core.h>

struct zn_backend {
  struct {
    struct wl_signal view_mapped;  // TODO(@Aki-7): implement this
  } events;
};

struct zn_backend *zn_backend_create(struct wl_display *display);

void zn_backend_destroy(struct zn_backend *base);
