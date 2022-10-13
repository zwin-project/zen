#pragma once

#include <wayland-server-core.h>
#include <znr/remote.h>

#include "zen/scene/scene.h"

struct zn_remote_scene {
  struct zn_scene* scene;     // non null
  struct znr_remote* remote;  // non null

  struct wl_list board_object_list;  // zn_board_remote_object::link

  struct wl_listener new_board_listener;
};

void zn_remote_scene_start_sync(struct zn_remote_scene* self);

void zn_remote_scene_stop_sync(struct zn_remote_scene* self);

struct zn_remote_scene* zn_remote_scene_create(
    struct zn_scene* scene, struct znr_remote* remote);

void zn_remote_scene_destroy(struct zn_remote_scene* self);
