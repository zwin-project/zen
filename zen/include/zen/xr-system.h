#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr_system;
struct zn_xr_dispatcher;

struct zn_xr_system_interface {
  void (*connect)(struct zn_xr_system *self);
};

///
///   create
///     ↓
///  available --→ dead → destroy
///     ↓↑          ↑
/// synchronized ---+
///     ↓↑
///   visible
///     ↓↑
///    focus
///
/// connected = synchronized | visible | focus
///
enum zn_xr_system_session_state {
  ZN_XR_SYSTEM_SESSION_STATE_AVAILABLE,
  ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED,
  ZN_XR_SYSTEM_SESSION_STATE_VISIBLE,
  ZN_XR_SYSTEM_SESSION_STATE_FOCUS,
  ZN_XR_SYSTEM_SESSION_STATE_DEAD,
};

struct zn_xr_system {
  void *impl_data;                            // @outlive
  const struct zn_xr_system_interface *impl;  // @nonnull, @outlive

  enum zn_xr_system_session_state state;

  /// These dispatchers are null when connected, not null otherwise.
  struct zn_xr_dispatcher *high_priority_dispatcher;  // @owning
  struct zn_xr_dispatcher *default_dispatcher;        // @owning

  struct {
    struct wl_signal session_state_changed;  // (NULL)
    struct wl_signal destroy;
  } events;
};

UNUSED static inline void
zn_xr_system_connect(struct zn_xr_system *self)
{
  self->impl->connect(self);
}

UNUSED static inline bool
zn_xr_system_is_connected(struct zn_xr_system *self)
{
  uint32_t connected = ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED |
                       ZN_XR_SYSTEM_SESSION_STATE_VISIBLE |
                       ZN_XR_SYSTEM_SESSION_STATE_FOCUS;
  return (self->state & connected) != 0;
}

#ifdef __cplusplus
}
#endif
