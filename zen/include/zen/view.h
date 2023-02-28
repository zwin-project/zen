#pragma once

#include <stdbool.h>
#include <wayland-server-core.h>

struct zn_view_interface {
  void (*set_activated)(void *impl_data, bool activate);
};

struct zn_view {
  void *impl_data;                       // @nullable, @outlive if exists
  const struct zn_view_interface *impl;  // @nonnull, @outlive

  struct zn_snode *snode;  // @nonnull, @owning

  struct {
    struct wl_signal unmap;  // (NULL)
  } events;
};

/// Called by the impl object
void zn_view_notify_unmap(struct zn_view *self);

/// Called by the impl object
struct zn_view *zn_view_create(
    void *impl_data, const struct zn_view_interface *implementation);

/// Called by the impl object
void zn_view_destroy(struct zn_view *self);
