#ifndef ZEN_SEAT_H
#define ZEN_SEAT_H

#include <wayland-server.h>

#define ZEN_DEFAULT_SEAT "seat0"

struct zn_seat;

struct zn_seat* zn_seat_create(struct wl_display* display, const char* seat_name);

void zn_seat_destroy(struct zn_seat* self);

#endif  // ZEN_SEAT_H
