#ifndef LIBZEN_DRAG_GRAB_H
#define LIBZEN_DRAG_GRAB_H

#include <libzen-compositor/libzen-compositor.h>

struct zen_drag_grab {
  struct wl_listener data_source_destroy_listener;
  struct zen_ray_grab base;
};

struct zen_drag_grab* zen_drag_grab_create(
    struct zen_data_device* data_device, struct zen_data_source* data_source);

#endif  // LIBZEN_DRAG_GRAB_H
