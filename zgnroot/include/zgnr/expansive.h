#pragma once

#include <wayland-server-core.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_expansive {
  struct zgnr_virtual_object *virtual_object;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
