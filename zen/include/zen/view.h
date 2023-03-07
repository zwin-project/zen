#pragma once

#include <cglm/types.h>
#include <stdbool.h>
#include <wayland-server-core.h>

struct zn_view_interface {
  void (*set_activated)(void *impl_data, bool activate);
};

enum zn_view_decoration_mode {
  ZN_VIEW_DECORATION_MODE_SERVER_SIDE = 0,
  ZN_VIEW_DECORATION_MODE_CLIENT_SIDE = 1,
};

struct zn_view {
  void *impl_data;                       // @nullable, @outlive if exists
  const struct zn_view_interface *impl;  // @nonnull, @outlive

  vec2 size;

  /// TODO(@Aki-7): The decoration mode is determined by the zen component.
  /// Considering Wayland apps, the user should be able to declare a preferred
  /// mode, and zen should respect the preferred mode as much as possible.
  enum zn_view_decoration_mode decoration_mode;

  struct zn_snode *snode;  // @nonnull, @owning

  struct {
    struct wl_signal resized;       // (NULL)
    struct wl_signal unmap;         // (NULL)
    struct wl_signal decoration;    // (NULL)
    struct wl_signal request_move;  // (NULL)
  } events;
};

/// Called by the impl object
void zn_view_notify_resized(struct zn_view *self, vec2 size);

/// Called by the impl object
void zn_view_notify_decoration(
    struct zn_view *self, enum zn_view_decoration_mode mode);

/// Called by the impl object
void zn_view_notify_move_request(struct zn_view *self);

/// Called by the impl object
void zn_view_notify_unmap(struct zn_view *self);

/// Called by the impl object
struct zn_view *zn_view_create(
    void *impl_data, const struct zn_view_interface *implementation);

/// Called by the impl object
void zn_view_destroy(struct zn_view *self);
