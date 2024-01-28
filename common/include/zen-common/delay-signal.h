#pragma once

#include <wayland-server.h>

struct zn_delay_signal {
  struct wl_signal signal;
  struct wl_event_source *idle;  // @nonnull, @ref
};

/// The signal handlers will be called at the next idle time.
/// No user data is allowed as it's hard to manage the ownership of the data.
/// In there is already a scheduled signal, this signal scheduling will be
/// ignored.
void zn_delay_signal_schedule(
    struct zn_delay_signal *self, struct wl_display *display);

void zn_delay_signal_add(
    struct zn_delay_signal *self, struct wl_listener *listener);

void zn_delay_signal_init(struct zn_delay_signal *self);

void zn_delay_signal_release(struct zn_delay_signal *self);
