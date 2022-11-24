#pragma once

#include <wayland-server-core.h>

struct zgnr_seat {
  void* user_data;
  uint32_t capabilities;
};

void zgnr_seat_set_capabilities(struct zgnr_seat* self, uint32_t capabilities);

struct zgnr_seat* zgnr_seat_create(struct wl_display* display);

void zgnr_seat_destroy(struct zgnr_seat* self);
