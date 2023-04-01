#pragma once

#include <wayland-server-core.h>

struct zn_cursor_grab;
struct zn_theme;

struct zn_desktop_shell {
  struct zn_screen_layout *screen_layout;  // @nonnull, @owning

  struct zn_cursor_grab *cursor_grab;  // @nonnull, @owning

  struct zn_theme *theme;  // @nonnull, @owning

  struct wl_listener new_xr_system_listener;
  struct wl_listener new_screen_listener;
  struct wl_listener seat_capabilities_listener;
  struct wl_listener pointer_motion_listener;
  struct wl_listener pointer_button_listener;
  struct wl_listener pointer_axis_listener;
  struct wl_listener pointer_frame_listener;
  struct wl_listener view_mapped_listener;
};

/// Ownership of `grab` moves to `self`
void zn_desktop_shell_start_cursor_grab(
    struct zn_desktop_shell *self, struct zn_cursor_grab *grab);

void zn_desktop_shell_end_cursor_grab(struct zn_desktop_shell *self);

struct zn_theme *zn_desktop_shell_get_theme(void);

struct zn_desktop_shell *zn_desktop_shell_get_singleton(void);

struct zn_desktop_shell *zn_desktop_shell_create(void);

void zn_desktop_shell_destroy(struct zn_desktop_shell *self);
