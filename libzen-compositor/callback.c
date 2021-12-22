#include "callback.h"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

static void zen_callback_destroy(struct zen_callback *callback);

static void
zen_callback_handle_destroy(struct wl_resource *resource)
{
  struct zen_callback *callback;

  callback = wl_resource_get_user_data(resource);

  zen_callback_destroy(callback);
}

WL_EXPORT void
zen_callback_done(struct zen_callback *callback, uint32_t callback_data)
{
  wl_callback_send_done(callback->resource, callback_data);
  wl_resource_destroy(callback->resource);
}

WL_EXPORT struct zen_callback *
zen_callback_create(struct wl_client *client, uint32_t id)
{
  struct zen_callback *callback;
  struct wl_resource *resource;

  callback = zalloc(sizeof *callback);
  if (callback == NULL) {
    wl_client_post_no_memory(client);
    zen_log("callback: failed to allocate memory\n");
    goto err;
  }

  resource = wl_resource_create(client, &wl_callback_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("callback: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(
      resource, NULL, callback, zen_callback_handle_destroy);

  callback->resource = resource;
  wl_list_init(&callback->link);

  return callback;

err_resource:
  free(callback);

err:
  return NULL;
}

static void
zen_callback_destroy(struct zen_callback *callback)
{
  wl_list_remove(&callback->link);
  free(callback);
}
