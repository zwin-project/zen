#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

struct zn_screen;
struct zn_screen_layout;
struct zn_ray;
struct zn_cursor;
struct zn_view;

struct zn_scene {
  struct zn_screen_layout *screen_layout;
  struct wl_list board_list;  // zn_board::link
  struct wl_list view_list;   // zn_view::link

  struct zn_cursor *cursor;  // nonnull
  struct zn_ray *ray;        // nonnull

  struct {
    struct wl_signal new_board;  // (struct zn_board*)
  } events;

  struct wlr_texture *wallpaper;  // nullable
};

void zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen);

void zn_scene_new_view(struct zn_scene *self, struct zn_view *view);

struct zn_scene *zn_scene_create(void);

void zn_scene_destroy_resources(struct zn_scene *self);

void zn_scene_destroy(struct zn_scene *self);
