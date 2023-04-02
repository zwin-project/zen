#pragma once

#include <wayland-server-core.h>

struct zn_xr_system;
struct zn_desktop_server_xr_system;

struct zn_desktop_xr_system {
  struct zn_xr_system *zn_xr_system;  // @nonnull, @outlive

  struct wl_global *global;      // @nonnull, @owning
  struct wl_list resource_list;  // wl_resource::link of zen_xr_system

  struct wl_listener zn_xr_system_destroy_listener;
  struct wl_listener session_status_changed_listener;
};

struct zn_desktop_xr_system *zn_desktop_xr_system_create(
    struct zn_xr_system *zn_xr_system, struct wl_display *display);
