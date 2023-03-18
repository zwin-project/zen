#pragma once

#include <cglm/types.h>
#include <stdbool.h>
#include <wayland-server-core.h>

#include "zen-common/util.h"

struct zn_view_interface {
  void (*set_focus)(void *impl_data, bool focused);
  void (*configure_size)(void *impl_data, vec2 size);
  void (*close)(void *impl_data);
};

enum zn_view_decoration_mode {
  ZN_VIEW_DECORATION_MODE_SERVER_SIDE = 0,
  ZN_VIEW_DECORATION_MODE_CLIENT_SIDE = 1,
};

struct zn_view_resize_event {
  uint32_t edges;  // a bitfield of enum wlr_edges
};

struct zn_view_resized_event {
  vec2 previous_size;
};

struct zn_view {
  void *impl_data;                       // @nullable, @outlive if exists
  const struct zn_view_interface *impl;  // @nonnull, @outlive

  vec2 size;

  bool has_focus;

  /// TODO(@Aki-7): The decoration mode is determined by the zen component.
  /// Considering Wayland apps, the user should be able to declare a preferred
  /// mode, and zen should respect the preferred mode as much as possible.
  enum zn_view_decoration_mode decoration_mode;

  struct zn_snode *snode;  // @nonnull, @owning

  struct {
    struct wl_signal resized;         // (struct zn_view_resized_event *)
    struct wl_signal unmap;           // (NULL)
    struct wl_signal decoration;      // (NULL)
    struct wl_signal move_request;    // (NULL)
    struct wl_signal resize_request;  // (struct zn_view_resize_event *)
  } events;
};

void zn_view_set_focus(struct zn_view *self, bool focused);

void zn_view_configure_size(struct zn_view *self, vec2 size);

void zn_view_close(struct zn_view *self);

/// Called by the impl object
void zn_view_notify_resized(struct zn_view *self, vec2 size);

/// Called by the impl object
void zn_view_notify_decoration(
    struct zn_view *self, enum zn_view_decoration_mode mode);

/// Called by the impl object
void zn_view_notify_move_request(struct zn_view *self);

/// Called by the impl object
void zn_view_notify_resize_request(
    struct zn_view *self, struct zn_view_resize_event *event);

/// Called by the impl object
void zn_view_notify_unmap(struct zn_view *self);

/// Called by the impl object
struct zn_view *zn_view_create(
    void *impl_data, const struct zn_view_interface *implementation);

/// Called by the impl object
void zn_view_destroy(struct zn_view *self);
