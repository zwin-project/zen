#pragma once

#include <wayland-server-core.h>

struct zgnr_seat_impl;

struct zgnr_seat_ray {
  struct zgnr_seat_impl *seat;  // nonnull

  struct wl_resource *resource;  // nonnull;

  struct wl_list link;  // zgnr_seat_impl::seat_ray_list
};

struct zgnr_seat_ray *zgnr_seat_ray_create(
    struct wl_client *client, uint32_t id, struct zgnr_seat_impl *seat);
