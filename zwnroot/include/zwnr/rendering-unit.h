#pragma once

#include <wayland-server-core.h>
#include <zwnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zwnr_rendering_unit_technique {
  ZWNR_TECHNIQUE_NONE,
  ZWNR_TECHNIQUE_BASE,
};

struct zwnr_rendering_unit {
  struct zwnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_list link;  // zwnr_virtual_object::current.rendering_unit_list
  bool committed;

  struct {
    struct zwnr_gl_base_technique *technique;  // nullable
    enum zwnr_rendering_unit_technique type;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
