#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

struct zn_backend;
struct zn_xr_system;

/// Within the lifetime of zn_backend, the start and stop methods can be called
/// in this order only once each.
struct zn_backend_interface {
  struct wlr_texture *(*create_wlr_texture_from_pixels)(struct zn_backend *self,
      uint32_t format, uint32_t stride, uint32_t width, uint32_t height,
      const void *data);
  /// @param xr_system is nullable
  void (*set_xr_system)(
      struct zn_backend *self, struct zn_xr_system *xr_system);
  /// @return value can be NULL
  struct zn_xr_system *(*get_xr_system)(struct zn_backend *self);
  /// Starts monitoring the connection or disconnection of input/output devices.
  bool (*start)(struct zn_backend *self);
  /// Destroy input/output devices
  void (*stop)(struct zn_backend *self);
  void (*destroy)(struct zn_backend *self);
};

struct zn_backend {
  const struct zn_backend_interface *impl;

  struct {
    struct wl_signal new_screen;      // (struct zn_screen *)
    struct wl_signal view_mapped;     // (struct zn_view *)
    struct wl_signal bounded_mapped;  // (struct zn_bounded *)
    struct wl_signal destroy;         // (NULL)
    struct wl_signal new_xr_system;   // (struct zn_xr_system *)
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

UNUSED static inline void
zn_backend_set_xr_system(
    struct zn_backend *self, struct zn_xr_system *xr_system)
{
  self->impl->set_xr_system(self, xr_system);
}

/// @return value can be NULL
UNUSED static inline struct zn_xr_system *
zn_backend_get_xr_system(struct zn_backend *self)
{
  return self->impl->get_xr_system(self);
}
