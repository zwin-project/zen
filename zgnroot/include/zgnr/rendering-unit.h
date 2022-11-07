#pragma once

#include <wayland-server-core.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_rendering_unit {
  struct zgnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_list link;  // zgnr_virtual_object::current.rendering_unit_list
  bool committed;

  struct {
    struct wl_list gl_base_technique_list;  // zgnr_gl_base_technique::link
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
