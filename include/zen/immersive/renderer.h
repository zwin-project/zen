#pragma once

#include <wayland-server-core.h>

#include "zen/scene/scene.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zn_immersive_renderer {};
#pragma GCC diagnostic pop

struct zn_immersive_renderer* zn_immersive_renderer_create(
    struct zn_scene* scene);

void zn_immersive_renderer_destroy(struct zn_immersive_renderer* self);
