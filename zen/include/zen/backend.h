#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

struct zn_backend;

/// Within the lifetime of zn_backend, the start and stop methods can be called
/// in this order only once each.
struct zn_backend_interface {
  struct wlr_texture *(*create_wlr_texture_from_pixels)(struct zn_backend *self,
      uint32_t format, uint32_t stride, uint32_t width, uint32_t height,
      const void *data);
  /// Starts monitoring the connection or disconnection of input/output devices.
  bool (*start)(struct zn_backend *self);
  /// Destroy input/output devices
  void (*stop)(struct zn_backend *self);
  void (*destroy)(struct zn_backend *self);
};

struct zn_backend {
  const struct zn_backend_interface *impl;

  struct {
    struct wl_signal new_screen;  // (struct zn_screen *)
    struct wl_signal destroy;     // (NULL)
  } events;
};

UNUSED static inline bool
zn_backend_start(struct zn_backend *self)
{
  return self->impl->start(self);
}

UNUSED static inline void
zn_backend_stop(struct zn_backend *self)
{
  self->impl->stop(self);
}

UNUSED static inline void
zn_backend_destroy(struct zn_backend *self)
{
  self->impl->destroy(self);
}

UNUSED static inline struct wlr_texture *
zn_backend_create_wlr_texture_from_pixels(struct zn_backend *self,
    uint32_t format, uint32_t stride, uint32_t width, uint32_t height,
    const void *data)
{
  return self->impl->create_wlr_texture_from_pixels(
      self, format, stride, width, height, data);
}
