#ifndef ZEN_INPUT_MANAGER_H
#define ZEN_INPUT_MANAGER_H

#include <wayland-server.h>

struct zn_input_manager;

struct zn_input_manager* zn_input_manager_create(struct wl_display* display);

void zn_input_manager_destroy(struct zn_input_manager* self);

#endif  // ZEN_INPUT_MANAGER_H
