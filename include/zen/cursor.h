#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/box.h>

struct zn_board;
struct zna_cursor;
struct zn_cursor_grab;
struct zn_default_cursor_grab;

struct zn_cursor_grab_interface {
  void (*motion_relative)(
      struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec);
  /**
   * @param board is nonnull
   */
  void (*motion_absolute)(struct zn_cursor_grab *grab, struct zn_board *board,
      double x, double y, uint32_t time_msec);
  void (*button)(struct zn_cursor_grab *grab, uint32_t time_msec,
      uint32_t button, enum wlr_button_state state);
  void (*axis)(struct zn_cursor_grab *grab, uint32_t time_msec,
      enum wlr_axis_source source, enum wlr_axis_orientation orientation,
      double delta, int32_t delta_discrete);
  void (*frame)(struct zn_cursor_grab *grab);
  void (*enter)(
      struct zn_cursor_grab *grab, struct zn_board *board, double x, double y);
  void (*leave)(struct zn_cursor_grab *grab);
  void (*rebase)(struct zn_cursor_grab *grab);
  void (*cancel)(struct zn_cursor_grab *grab);
};

struct zn_cursor_grab {
  const struct zn_cursor_grab_interface *impl;
  struct zn_cursor *cursor;
};

struct zn_cursor {
  struct zn_cursor_grab *grab;                  // nonnull
  struct zn_default_cursor_grab *default_grab;  // nonnull

  double x, y;
  bool visible;
  struct zn_board *board;  // nullable

  struct wlr_surface *surface;  // nullable
  double surface_hotspot_x, surface_hotspot_y;

  char *xcursor_name;                   // nonnull
  struct wlr_texture *xcursor_texture;  // nullable
  double xcursor_hotspot_x, xcursor_hotspot_y;

  struct {
    vec2 size;
    mat4 transform;
  } geometry;

  struct wlr_xcursor_manager *xcursor_manager;  // nonnull

  struct wl_listener board_destroy_listener;
  struct wl_listener surface_commit_listener;
  struct wl_listener surface_destroy_listener;

  struct zna_cursor *appearance;
};

void zn_cursor_get_fbox(struct zn_cursor *self, struct wlr_fbox *fbox);

void zn_cursor_damage(struct zn_cursor *self);

/**
 * @returns cursor texture, nullable
 */
struct wlr_texture *zn_cursor_get_texture(struct zn_cursor *self);

void zn_cursor_set_surface(struct zn_cursor *self, struct wlr_surface *surface,
    int hotspot_x, int hotspot_y);

/**
 * @param name cannot be NULL
 */
void zn_cursor_set_xcursor(struct zn_cursor *self, const char *name);

/**
 * @param board can be NULL
 */
void zn_cursor_move(
    struct zn_cursor *self, struct zn_board *board, double x, double y);

void zn_cursor_start_grab(struct zn_cursor *self, struct zn_cursor_grab *grab);

void zn_cursor_end_grab(struct zn_cursor *self);

struct zn_cursor *zn_cursor_create(void);

void zn_cursor_destroy_resources(struct zn_cursor *self);

void zn_cursor_destroy(struct zn_cursor *self);
