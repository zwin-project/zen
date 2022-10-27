#pragma once

#include <cglm/cglm.h>
#include <wayland-server-core.h>
#include <znr/gl-buffer.h>
#include <znr/rendering-unit.h>
#include <znr/virtual-object.h>

#include "zen/immersive/remote/mem-buffer.h"
#include "zen/scene/ray.h"

struct zn_remote_scene;

struct zn_ray_remote_object {
  struct zn_remote_scene* remote_scene;
  struct zn_ray* ray;

  struct znr_virtual_object* virtual_object;
  struct znr_rendering_unit* rendering_unit;
  struct znr_gl_buffer* gl_buffer;
  struct zn_remote_mem_buffer* vertex_buffer;

  struct wl_listener ray_destroy_listener;
  struct wl_listener ray_motion_listener;
};

struct zn_ray_remote_object* zn_ray_remote_object_create(
    struct zn_ray* ray, struct zn_remote_scene* remote_scene);

void zn_ray_remote_object_destroy(struct zn_ray_remote_object* self);
