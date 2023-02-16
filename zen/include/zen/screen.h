#pragma once

#include <time.h>
#include <wayland-server-core.h>

struct zn_snode;

struct zn_screen {
  void *impl;

  struct zn_snode *snode_root;  // @nonnull, @owning

  struct {
    struct wl_signal frame;    // (struct timespec *)
    struct wl_signal destroy;  // (NULL)
  } events;
};

/// Called by the impl object
void zn_screen_notify_frame(struct zn_screen *self, struct timespec *when);

/// Called by the impl object
struct zn_screen *zn_screen_create(void *impl);

/// Called by the impl object
void zn_screen_destroy(struct zn_screen *self);
