#ifndef ZEN_DRAG_GRAB_H
#define ZEN_DRAG_GRAB_H

#include <libzen-compositor/libzen-compositor.h>

int zen_ray_start_drag(struct zen_ray* ray, struct zen_data_source* data_source,
    struct zen_virtual_object* icon, struct wl_client* client);

#endif  // ZEN_DRAG_GRAB_H
