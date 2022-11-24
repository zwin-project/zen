#pragma once

#include <wayland-server-core.h>

struct zgnr_seat {
  void* user_data;
};

struct zgnr_seat* zgnr_seat_create(struct wl_display* display);

void zgnr_seat_destroy(struct zgnr_seat* self);
