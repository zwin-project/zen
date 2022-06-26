#ifndef ZEN_BACKEND_H
#define ZEN_BACKEND_H

#include <wayland-server.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zn_backend {};
#pragma GCC diagnostic pop

struct zn_backend *zn_backend_create(struct wl_display *display);

void zn_backend_destroy(struct zn_backend *self);

#endif  //  ZEN_BACKEND_H
