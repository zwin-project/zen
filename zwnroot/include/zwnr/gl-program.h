#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_gl_program {
  struct {
    struct wl_signal destroy;
  } events;

  struct {
    struct wl_list program_shader_list;  // zwnr_program_shader::link

    // User may assign false to this; zwnr will only assign true to this.
    bool should_link;

    bool linked;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
