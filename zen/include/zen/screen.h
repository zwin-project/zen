#pragma once

#include <time.h>
#include <wayland-server-core.h>
#include <wlr/util/box.h>

struct zn_snode;

struct zn_screen_interface {
  /// @param damage_fbox : Effective coordinate system
  void (*damage)(void *impl_data, struct wlr_fbox *damage_fbox);
};

struct zn_screen {
  void *impl_data;                         // @nullable, @outlive if exists
  const struct zn_screen_interface *impl;  // @nonnull, @outlive

  struct zn_snode *snode_root;  // @nonnull, @owning

  struct {
    struct wl_signal frame;    // (struct timespec *)
    struct wl_signal destroy;  // (NULL)
  } events;
};

/// @param fbox : Effective coordinate system
void zn_screen_damage(struct zn_screen *self, struct wlr_fbox *fbox);

/// Called by the impl object
void zn_screen_notify_frame(struct zn_screen *self, struct timespec *when);

/// Called by the impl object
struct zn_screen *zn_screen_create(
    void *impl_data, const struct zn_screen_interface *implementation);

/// Called by the impl object
void zn_screen_destroy(struct zn_screen *self);
