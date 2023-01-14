#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zwnr_virtual_object_role {
  ZWNR_VIRTUAL_OBJECT_ROLE_NONE,       // NULL
  ZWNR_VIRTUAL_OBJECT_ROLE_BOUNDED,    // struct zwnr_bounded*
  ZWNR_VIRTUAL_OBJECT_ROLE_EXPANSIVE,  // struct zwnr_expansive*
};

struct zwnr_virtual_object {
  struct {
    struct wl_signal destroy;    // (NULL)
    struct wl_signal committed;  // (NULL)
  } events;

  struct wl_resource *resource;

  bool committed;

  struct {
    // To enable rendering_unit_list, call zwnr_gles_v32_create
    struct wl_list rendering_unit_list;

    struct wl_list frame_callback_list;
  } current;

  enum zwnr_virtual_object_role role;
  void *role_object;  // See enum zwnr_virtual_object_role for the content

  void *user_data;
};

void zwnr_virtual_object_send_frame_done(
    struct zwnr_virtual_object *self, const struct timespec *when);

#ifdef __cplusplus
}
#endif
