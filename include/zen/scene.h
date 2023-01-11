#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

struct zn_board;
struct zn_screen;
struct zn_screen_layout;
struct zn_ray;
struct zn_cursor;
struct zn_view;

struct zn_scene {
  struct zn_screen_layout *screen_layout;
  struct wl_list board_list;  // zn_board::link
  struct wl_list view_list;   // zn_view::link

  struct zn_cursor *cursor;      // nonnull
  struct zn_ray *ray;            // nonnull
  struct zn_view *focused_view;  // nullable

  struct wl_listener focused_view_destroy_listener;
  struct wl_listener display_system_changed_listener;

  struct {
    struct wl_signal new_board;               // (struct zn_board*)
    struct wl_signal board_mapped_to_screen;  // (struct zn_board*)
  } events;

  struct wlr_texture *wallpaper;  // nullable
};

void zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen);

void zn_scene_new_view(struct zn_scene *self, struct zn_view *view);

void zn_scene_set_focused_view(struct zn_scene *self, struct zn_view *view);

void zn_scene_initialize_boards(
    struct zn_scene *self, int64_t board_initial_count);

void zn_scene_setup_keybindings(struct zn_scene *self);

struct zn_scene *zn_scene_create(void);

void zn_scene_destroy_resources(struct zn_scene *self);

void zn_scene_destroy(struct zn_scene *self);
