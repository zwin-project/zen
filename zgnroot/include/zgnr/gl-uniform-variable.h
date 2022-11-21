#pragma once

#include <wayland-server-core.h>
#include <zigen-gles-v32-protocol.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_uniform_variable {
  uint32_t location;
  char *name;  // nullable
  enum zgn_gl_base_technique_uniform_variable_type type;
  uint32_t col;
  uint32_t row;
  uint32_t count;
  bool transpose;
  bool newly_comitted;
  void *value;

  // zgnr_gl_base_technique::current.uniform_variable_list or
  // zgnr_gl_base_technique_impl::pending.uniform_variable_list (internal use)
  struct wl_list link;
};

#ifdef __cplusplus
}
#endif
