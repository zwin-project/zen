#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

struct zn_screen;
struct zn_ray;
struct zn_space;
struct zn_cursor;
struct zn_view;

struct zn_scene {
  struct wl_list screen_list;  // zn_screen::link
  struct wl_list board_list;   // zn_board::link
  struct wl_list view_list;    // zn_view::link

  struct zn_space *current_space;  // nullable, reference

  struct zn_cursor *cursor;  // nonnull
  struct zn_ray *ray;        // nonnull

  struct {
    struct wl_signal new_board;  // (struct zn_board*)
  } events;

  struct wl_listener current_space_destroy_listener;

  struct wlr_texture *wallpaper;  // nullable
};

void zn_scene_new_space(struct zn_scene *self, struct zn_space *space);

void zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen);

void zn_scene_new_view(struct zn_scene *self, struct zn_view *view);

struct zn_scene *zn_scene_create(void);

void zn_scene_destroy_resources(struct zn_scene *self);

void zn_scene_destroy(struct zn_scene *self);
