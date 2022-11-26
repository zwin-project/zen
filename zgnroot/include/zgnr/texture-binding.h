#pragma once

#include <wayland-server-core.h>
#include <zgnr/gl-texture.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_texture_binding {
  uint32_t binding;
  char *name;                       // null terminated
  struct zgnr_gl_texture *texture;  // nonnull, reference
  uint32_t target;

  //  zgnr_gl_base_technique::current.texture_binding_list or
  //  zgnr_gl_base_technique_impl::pending.texture_binding_list (internal use)
  struct wl_list link;
};

#ifdef __cplusplus
}
#endif
