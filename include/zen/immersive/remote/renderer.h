#pragma once

#include "zen/immersive/remote/scene.h"
#include "zen/immersive/renderer.h"
#include "zen/scene/scene.h"

struct zn_remote_immersive_renderer {
  struct zn_immersive_renderer base;

  struct zn_remote_scene* remote_scene;
};

void zn_remote_immersive_renderer_activate(
    struct zn_remote_immersive_renderer* self);

void zn_remote_immersive_renderer_deactivate(
    struct zn_remote_immersive_renderer* self);
