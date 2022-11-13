#pragma once

#include <wayland-server-core.h>
#include <zgnr/gl-buffer.h>
#include <zgnr/gl-vertex-attrib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_vertex_array {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    struct wl_list vertex_attrib_list;  // struct zgnr_gl_vertex_attrib::link
  } current;

  void* user_data;
};

#ifdef __cplusplus
}
#endif
