#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_space_manager {
  struct {
    struct wl_signal new_space;  // (struct zgnr_space*)
  } events;
};

struct zgnr_space_manager *zgnr_space_manager_create(
    struct wl_display *display);

void zgnr_space_manager_destroy(struct zgnr_space_manager *self);

#ifdef __cplusplus
}
#endif
