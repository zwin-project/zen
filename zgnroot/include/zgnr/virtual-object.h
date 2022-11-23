#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zgnr_virtual_object_role {
  ZGNR_VIRTUAL_OBJECT_ROLE_NONE,     // NULL
  ZGNR_VIRTUAL_OBJECT_ROLE_BOUNDED,  // struct zgnr_bounded*
};

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

  enum zgnr_virtual_object_role role;
  void *role_object;  // See enum zgnr_virtual_object_role for the content

  void *user_data;
};

void zgnr_virtual_object_send_frame_done(
    struct zgnr_virtual_object *self, const struct timespec *when);

#ifdef __cplusplus
}
#endif
