#pragma once

#include <wayland-server.h>

struct zn_xr_system;

struct zn_gl_context {
  struct wl_global *global;  // @nonnull, @owning

  struct wl_list resource_list;  // wl_resource::link of zwn_gl_context

  // must be connected state
  struct zn_xr_system *xr_system;  // @nonnull, @outlive

  struct wl_listener xr_system_session_state_changed_listener;

  bool destroying;

  struct {
    struct wl_signal destroy;
  } events;
};

struct zn_gl_context *zn_gl_context_create(
    struct wl_display *display, struct zn_xr_system *xr_system);

void zn_gl_context_destroy(struct zn_gl_context *self);
