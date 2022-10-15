#pragma once

#include <wayland-server-core.h>
#include <znr/rendering-unit.h>

#include "zen/scene/ray.h"

struct zn_remote_scene;

struct zn_ray_remote_object {
  struct zn_remote_scene* remote_scene;
  struct zn_ray* ray;

  struct znr_rendering_unit* rendering_unit;

  struct wl_listener ray_destroy_listener;
};

struct zn_ray_remote_object* zn_ray_remote_object_create(
    struct zn_ray* ray, struct zn_remote_scene* remote_scene);

void zn_ray_remote_object_destroy(struct zn_ray_remote_object* self);
