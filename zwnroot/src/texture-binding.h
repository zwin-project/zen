#include "gl-sampler.h"
#include "gl-texture.h"
#include "zwnr/texture-binding.h"

struct zwnr_texture_binding_impl {
  struct zwnr_texture_binding base;

  struct wl_listener texture_destroy_listener;
  struct wl_listener sampler_destroy_listener;
};

struct zwnr_texture_binding_impl *zwnr_texture_binding_create(uint32_t binding,
    const char *name, struct zwnr_gl_texture_impl *texture, uint32_t target,
    struct zwnr_gl_sampler_impl *sampler);

void zwnr_texture_binding_destroy(struct zwnr_texture_binding_impl *self);
