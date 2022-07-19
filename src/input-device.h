#ifndef ZEN_INPUT_DEVICE_H
#define ZEN_INPUT_DEVICE_H

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

struct zn_input_device;

enum wlr_input_device_type zn_input_device_get_type(
    struct zn_input_device* self);

struct zn_input_device* zn_input_device_create(
    struct wlr_input_device* wlr_input, struct wl_list* devices);

void zn_input_device_destroy(struct zn_input_device* self);

#endif  // ZEN_INPUT_DEVICE_H
