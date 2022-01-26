#include "ray-client.h"

#include <libzen-compositor/libzen-compositor.h>
#include <zigen-server-protocol.h>

static void zen_ray_client_destroy(struct zen_ray_client *ray_client);

static void
zen_ray_client_handle_destroy(struct wl_resource *resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zen_ray_client_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zgn_ray_interface ray_interface = {
    .release = zen_ray_client_protocol_release,
};

static void
zen_ray_client_ray_destroy_handler(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_ray_client *ray_client;

  ray_client = wl_container_of(listener, ray_client, ray_destroy_listener);

  zen_ray_client_destroy(ray_client);
}

static void
zen_ray_client_client_destroy_handler(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_ray_client *ray_client;

  ray_client = wl_container_of(listener, ray_client, client_destroy_listener);

  zen_ray_client_destroy(ray_client);
}

static struct zen_ray_client *
zen_ray_client_create(struct wl_client *client, struct zen_ray *ray)
{
  struct zen_ray_client *ray_client;

  ray_client = zalloc(sizeof *ray_client);
  if (ray_client == NULL) {
    wl_client_post_no_memory(client);
    zen_log("ray client: failed to allocate memory\n");
    goto err;
  }

  ray_client->ray = ray;
  wl_list_insert(&ray->ray_client_list, &ray_client->link);

  ray_client->ray_destroy_listener.notify = zen_ray_client_ray_destroy_handler;
  wl_signal_add(&ray->destroy_signal, &ray_client->ray_destroy_listener);

  ray_client->client_destroy_listener.notify =
      zen_ray_client_client_destroy_handler;
  wl_client_add_destroy_listener(client, &ray_client->client_destroy_listener);

  ray_client->client = client;
  wl_list_init(&ray_client->resource_list);

  return ray_client;

err:
  return NULL;
}

static void
zen_ray_client_destroy(struct zen_ray_client *ray_client)
{
  struct wl_resource *resource, *tmp;
  wl_resource_for_each_safe(resource, tmp, &ray_client->resource_list)
  {
    wl_resource_set_user_data(resource, NULL);
    wl_resource_set_destructor(resource, NULL);
    wl_list_init(wl_resource_get_link(resource));
  }

  wl_list_remove(&ray_client->client_destroy_listener.link);
  wl_list_remove(&ray_client->ray_destroy_listener.link);
  wl_list_remove(&ray_client->link);
  free(ray_client);
}

WL_EXPORT int
zen_ray_client_add_resource(struct zen_ray_client *ray_client, uint32_t id)
{
  struct wl_resource *resource;

  resource = wl_resource_create(ray_client->client, &zgn_ray_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(ray_client->client);
    zen_log("ray client: failed to create a resource\n");
    goto err;
  }

  wl_resource_set_implementation(
      resource, &ray_interface, ray_client, zen_ray_client_handle_destroy);

  wl_list_insert(&ray_client->resource_list, wl_resource_get_link(resource));

  // FIXME: send enter event if already focused

  return 0;

err:
  return -1;
}

WL_EXPORT struct zen_ray_client *
zen_ray_client_find(struct wl_client *client, struct zen_ray *ray)
{
  struct zen_ray_client *ray_client;

  wl_list_for_each(ray_client, &ray->ray_client_list, link)
  {
    if (ray_client->client == client) return ray_client;
  }

  return NULL;
}

WL_EXPORT struct zen_ray_client *
zen_ray_client_ensure(struct wl_client *client, struct zen_ray *ray)
{
  struct zen_ray_client *ray_client;

  ray_client = zen_ray_client_find(client, ray);

  if (ray_client) {
    return ray_client;
  } else {
    return zen_ray_client_create(client, ray);
  }
}

WL_EXPORT struct wl_resource *
zen_ray_client_create_inert_resource(struct wl_client *client, uint32_t id)
{
  struct wl_resource *resource;

  resource = wl_resource_create(client, &zgn_ray_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("ray client: failed to create a resource\n");
    goto err;
  }

  wl_resource_set_implementation(resource, &ray_interface, NULL, NULL);

  return resource;

err:
  return NULL;
}
