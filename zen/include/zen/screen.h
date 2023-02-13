#pragma once

#include <pixman.h>
#include <wayland-server-core.h>

struct zn_screen_frame_event {
  pixman_region32_t damage;
};

struct zn_screen {
  void *impl;

  struct {
    struct wl_signal frame;    // (struct zn_screen_frame_event *)
    struct wl_signal destroy;  // (NULL)
  } events;
};

/// Called by the impl object
void zn_screen_notify_frame(
    struct zn_screen *self, struct zn_screen_frame_event *event);

/// Called by the impl object
struct zn_screen *zn_screen_create(void *impl);

/// Called by the impl object
void zn_screen_destroy(struct zn_screen *self);
