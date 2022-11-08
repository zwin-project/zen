#pragma once

#include <wayland-server-core.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zgnr_rendering_unit_technique {
  ZGNR_TECHNIQUE_NONE,
  ZGNR_TECHNIQUE_BASE,
};

struct zgnr_rendering_unit {
  struct zgnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_list link;  // zgnr_virtual_object::current.rendering_unit_list
  bool committed;

  struct {
    struct zgnr_gl_base_technique *technique;  // nullable
    enum zgnr_rendering_unit_technique type;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
