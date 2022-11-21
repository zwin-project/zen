#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_virtual_object {
  struct {
    struct wl_signal destroy;    // (NULL)
    struct wl_signal committed;  // (NULL)
  } events;

  bool committed;

  struct {
    // To enable rendering_unit_list, call zgnr_gles_v32_create
    struct wl_list rendering_unit_list;

    struct wl_list frame_callback_list;
  } current;

  void *user_data;
};

void zgnr_virtual_object_send_frame_done(
    struct zgnr_virtual_object *self, const struct timespec *when);

#ifdef __cplusplus
}
#endif
