#pragma once

#include <wayland-server-core.h>

#include "zen/immersive/remote/scene.h"
#include "zen/scene/board.h"

struct zn_board_remote_object {
  struct zn_board* board;  // non null;

  struct wl_list link;  // zn_remote_immersive_render::scene::board_object_list

  struct wl_listener board_destroy_listener;
};

struct zn_board_remote_object* zn_board_remote_object_create(
    struct zn_board* board, struct zn_remote_scene* remote_scene);

void zn_board_remote_object_destroy(struct zn_board_remote_object* self);
