#ifndef ZEN_POINTER_H
#define ZEN_POINTER_H

#include <wayland-server.h>
#include <wlr/interfaces/wlr_pointer.h>

struct zn_pointer {
  struct wlr_pointer* wlr_pointer;
};

struct zn_pointer* zn_pointer_create(struct wlr_input_device* input_device);

void zn_pointer_destroy(struct zn_pointer* self);

#endif  // ZEN_POINTER_H
