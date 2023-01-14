#pragma once

#include <wayland-server-core.h>

struct zwnr_seat_impl;

struct zwnr_seat_ray {
  struct zwnr_seat_impl *seat;  // nonnull

  struct wl_resource *resource;  // nonnull;

  struct wl_list link;  // zwnr_seat_impl::seat_ray_list
};

struct zwnr_seat_ray *zwnr_seat_ray_create(
    struct wl_client *client, uint32_t id, struct zwnr_seat_impl *seat);
