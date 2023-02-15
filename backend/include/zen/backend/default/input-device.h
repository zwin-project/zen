#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>

struct zn_input_device_base {
  struct wlr_input_device *wlr_input_device;  // @nonnull, @outlive

  struct wl_list link;  // zn_seat::device_list
};
