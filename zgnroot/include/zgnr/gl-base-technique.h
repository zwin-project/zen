#pragma once

#include <wayland-server-core.h>
#include <zgnr/gl-vertex-array.h>
#include <zgnr/rendering-unit.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_base_technique {
  struct zgnr_rendering_unit *unit;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  bool comitted;

  struct {
    struct zgnr_gl_vertex_array *vertex_array;  // nullable
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
