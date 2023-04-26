#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_texture;
struct zn_buffer;

struct zn_gl_texture_interface {
  void (*image_2d)(struct zn_gl_texture *self, uint32_t target, int32_t level,
      int32_t internal_format, uint32_t width, uint32_t height, int32_t border,
      uint32_t format, uint32_t type, struct zn_buffer *data);
  void (*generate_mipmap)(struct zn_gl_texture *self, uint32_t target);
};

struct zn_gl_texture {
  void *impl_data;                             // @nullable, @outlive
  const struct zn_gl_texture_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

UNUSED static inline void
zn_gl_texture_image_2d(struct zn_gl_texture *self, uint32_t target,
    int32_t level, int32_t internal_format, uint32_t width, uint32_t height,
    int32_t border, uint32_t format, uint32_t type, struct zn_buffer *data)
{
  self->impl->image_2d(self, target, level, internal_format, width, height,
      border, format, type, data);
}

UNUSED static inline void
zn_gl_texture_generate_mipmap(struct zn_gl_texture *self, uint32_t target)
{
  self->impl->generate_mipmap(self, target);
}

#ifdef __cplusplus
}
#endif
