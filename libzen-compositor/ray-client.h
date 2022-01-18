#ifndef LIBZEN_COMPOSITOR_RAY_CLIENT_H
#define LIBZEN_COMPOSITOR_RAY_CLIENT_H

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

struct zen_ray_client {
  struct zen_ray *ray;
  struct wl_list link;
  struct wl_listener ray_destroy_listener;

  struct wl_client *client;

  struct wl_list resource_list;
};

int zen_ray_client_add_resource(struct zen_ray_client *ray_client, uint32_t id);

struct zen_ray_client *zen_ray_client_find(
    struct wl_client *client, struct zen_ray *ray);

struct zen_ray_client *zen_ray_client_ensure(
    struct wl_client *client, struct zen_ray *ray);

struct wl_resource *zen_ray_client_create_insert_resource(
    struct wl_client *client, uint32_t id);

#endif  //  LIBZEN_COMPOSITOR_RAY_CLIENT_H
