#pragma once

#include <wayland-server-core.h>
#include <zwnr/gl-sampler.h>
#include <zwnr/gl-texture.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_texture_binding {
  uint32_t binding;
  char *name;                       // null terminated
  struct zwnr_gl_texture *texture;  // nonnull, reference
  struct zwnr_gl_sampler *sampler;  // nonnull, reference
  uint32_t target;

  //  zwnr_gl_base_technique::current.texture_binding_list or
  //  zwnr_gl_base_technique_impl::pending.texture_binding_list (internal use)
  struct wl_list link;
};

#ifdef __cplusplus
}
#endif
