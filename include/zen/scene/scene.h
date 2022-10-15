#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

#include "zen/config.h"
#include "zen/scene/ray.h"
#include "zen/scene/screen-layout.h"

struct zn_scene {
  struct zn_screen_layout* screen_layout;

  struct zn_view* focused_view;  // nullable

  struct wl_list board_list;  // zn_board::link, non empty

  // create or get zn_scene::ray via zn_scene_ensure_ray
  struct zn_ray* ray;  // null if no pointing device exists

  struct wl_listener unmap_focused_view_listener;

  struct wlr_texture* bg_texture;  // nullable

  struct {
    struct wl_signal new_board;  // (struct zn_board*)
    struct wl_signal new_ray;    // (NULL)
  } events;
};

struct zn_ray* zn_scene_ensure_ray(struct zn_scene* self);

void zn_scene_destroy_ray(struct zn_scene* self);

void zn_scene_set_focused_view(struct zn_scene* self, struct zn_view* view);

struct zn_board* zn_scene_get_focus_board(struct zn_scene* self);

void zn_scene_reassign_boards(struct zn_scene* self);

void zn_scene_setup_bindings(struct zn_scene* self);

struct zn_scene* zn_scene_create(struct zn_config* config);

void zn_scene_destroy_resources(struct zn_scene* self);

void zn_scene_destroy(struct zn_scene* self);
