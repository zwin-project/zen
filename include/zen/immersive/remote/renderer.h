#pragma once

#include "zen/immersive/remote/scene.h"
#include "zen/scene/scene.h"

struct zn_remote_immersive_renderer {
  struct zn_remote_scene* remote_scene;
};

void zn_remote_immersive_renderer_activate(
    struct zn_remote_immersive_renderer* self);

void zn_remote_immersive_renderer_deactivate(
    struct zn_remote_immersive_renderer* self);

struct zn_remote_immersive_renderer* zn_remote_immersive_renderer_create(
    struct zn_scene* scene, struct znr_remote* remote);

void zn_remote_immersive_renderer_destroy(
    struct zn_remote_immersive_renderer* self);
