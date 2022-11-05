#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_virtual_object {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
