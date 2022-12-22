#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_data_device.h>

struct zn_data_device_manager {
  // This object will be automatically destroyed when wl_display is destroyed
  struct wlr_data_device_manager *wlr_data_device_manager;
};

struct zn_data_device_manager *zn_data_device_manager_create(
    struct wl_display *display);

void zn_data_device_manager_destroy(struct zn_data_device_manager *self);
