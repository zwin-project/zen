#pragma once

#include <wayland-server-core.h>

struct zn_seat;

struct zn_backend {
  struct zn_seat *seat;  // @nonnull, @owning

  struct {
    struct wl_signal new_screen;  // (struct zn_screen *)
  } events;
};

bool zn_backend_start(struct zn_backend *base);

struct zn_backend *zn_backend_create(struct wl_display *display);

void zn_backend_destroy(struct zn_backend *base);
