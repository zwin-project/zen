#ifndef LIBZEN_DRAG_GRAB_H
#define LIBZEN_DRAG_GRAB_H

#include <libzen-compositor/libzen-compositor.h>

struct zen_drag_grab {
  struct zen_ray_grab base;
};

struct zen_drag_grab* zen_drag_grab_create(struct zen_data_device* data_device);

#endif  // LIBZEN_DRAG_GRAB_H
