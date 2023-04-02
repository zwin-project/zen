#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr_system;

struct zn_xr_system_interface {
  void (*connect)(struct zn_xr_system *self);
};

enum zn_xr_system_session_status {
  ZN_XR_SYSTEM_SESSION_STATUS_NOT_CONNECTED,  // initial status
  ZN_XR_SYSTEM_SESSION_STATUS_CONNECTED,
};

struct zn_xr_system {
  void *impl_data;                            // @outlive
  const struct zn_xr_system_interface *impl;  // @nonnull, @outlive

  enum zn_xr_system_session_status status;

  struct {
    struct wl_signal session_status_changed;  // (NULL)
    struct wl_signal destroy;
  } events;
};

inline void
zn_xr_system_connect(struct zn_xr_system *self)
{
  self->impl->connect(self);
}

#ifdef __cplusplus
}
#endif
