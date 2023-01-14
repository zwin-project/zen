#pragma once

#include <wayland-server-core.h>
#include <zwnr/gl-shader.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * bind zwnr_program and zwnr_shader
 */
struct zwnr_program_shader {
  struct zwnr_gl_shader *shader;  // nonnull

  // zwnr_gl_program::current.program_shader_list or
  // zwnr_gl_program_impl::pending.program_shader_list (internal use)
  struct wl_list link;
};

#ifdef __cplusplus
}
#endif
