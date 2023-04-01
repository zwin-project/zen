#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr_system {
  struct {
    struct wl_signal destroy;
  } events;
};

#ifdef __cplusplus
}
#endif
