#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

struct zn_backend;
struct zn_seat;

struct zn_backend *zn_default_backend_create(
    struct wl_display *display, struct zn_seat *zn_seat);
