#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr_system {
  void *impl_data;

  uint64_t handle;

  struct {
    struct wl_signal destroy;
  } events;
};

#ifdef __cplusplus
}
#endif
