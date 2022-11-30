#pragma once

#include <wayland-server-core.h>
#include <zgnr/gl-shader.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * bind zgnr_program and zgnr_shader
 */
struct zgnr_program_shader {
  struct zgnr_gl_shader *shader;  // nonnull

  // zgnr_gl_program::current.program_shader_list or
  // zgnr_gl_program_impl::pending.program_shader_list (internal use)
  struct wl_list link;
};

#ifdef __cplusplus
}
#endif
