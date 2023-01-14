#pragma once

#include <wayland-server-core.h>
#include <zwnr/gl-buffer.h>
#include <zwnr/gl-vertex-attrib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_gl_vertex_array {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    struct wl_list vertex_attrib_list;  // struct zwnr_gl_vertex_attrib::link
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
