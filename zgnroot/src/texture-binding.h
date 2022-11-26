#include "gl-texture.h"
#include "zgnr/texture-binding.h"

struct zgnr_texture_binding_impl {
  struct zgnr_texture_binding base;

  struct wl_listener texture_destroy_listener;
};

struct zgnr_texture_binding_impl* zgnr_texture_binding_create(uint32_t binding,
    const char* name, struct zgnr_gl_texture_impl* texture, uint32_t target);

void zgnr_texture_binding_destroy(struct zgnr_texture_binding_impl* self);
