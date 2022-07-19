#ifndef ZEN_SEAT_DEVICE_H
#define ZEN_SEAT_DEVICE_H

#include <stdbool.h>

#include "input-device.h"
#include "seat.h"

struct zn_seat_device {
  struct zn_seat* seat;
  struct zn_input_device* input_device;
  struct wl_list link;  // zn_seat::devices
};

struct zn_seat_device* zn_seat_device_create(struct zn_seat* seat,
    struct zn_input_device* input_device, struct wl_list* devices);

void zn_seat_device_destroy(struct zn_seat_device* self);

#endif  // ZEN_SEAT_DEVICE_H
