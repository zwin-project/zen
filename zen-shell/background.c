#include "background.h"

#include <string.h>
#include <zigen-shell-server-protocol.h>

char *zen_background_role = "zen_background";

static void
zen_background_handle_destroy(struct wl_resource *resource)
{
  struct zen_background *background;

  background = wl_resource_get_user_data(resource);

  zen_background_destroy(background);
}

static void
zen_background_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_background_virtual_object_destroy_handler(
    struct wl_listener *listener, void *data)
{
  // Clients must not destroy a bound virtual object before a background. But
  // if it does so, compositor will just destroy the background as well.
  // Sending implementation error to clients brings error when terminating the
  // client

  UNUSED(data);
  struct zen_background *background;

  background =
      wl_container_of(listener, background, virtual_object_destroy_listener);

  wl_resource_destroy(background->resource);
}

static const struct zgn_background_interface background_interface = {
    .destroy = zen_background_protocol_destroy,
};

WL_EXPORT struct zen_background *
zen_background_create(struct wl_client *client, uint32_t id,
    struct wl_resource *shell_resource,
    struct zen_virtual_object *virtual_object)
{
  struct zen_background *background;
  struct wl_resource *resource;

  if (virtual_object->role_object != NULL) {
    wl_resource_post_error(shell_resource, ZGN_SHELL_ERROR_ROLE,
        "given virtual object has another role");
    zen_log("background: given virtual object has another role\n");
    goto err;
  }

  if (strcmp(virtual_object->role, "") != 0 &&
      strcmp(virtual_object->role, zen_background_role) != 0) {
    wl_resource_post_error(shell_resource, ZGN_SHELL_ERROR_ROLE,
        "given virtual object has been attached another type of role");
    zen_log(
        "background: given virtual object has been attached another type of "
        "role\n");
    goto err;
  }

  background = zalloc(sizeof *background);
  if (background == NULL) {
    wl_client_post_no_memory(client);
    zen_log("background: failed to allocate memory\n");
    goto err;
  }

  resource = wl_resource_create(client, &zgn_background_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("background: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &background_interface, background,
      zen_background_handle_destroy);

  free(virtual_object->role);
  virtual_object->role = strdup(zen_background_role);
  virtual_object->role_object = background;

  background->resource = resource;
  background->virtual_object = virtual_object;

  background->virtual_object_destroy_listener.notify =
      zen_background_virtual_object_destroy_handler;
  wl_signal_add(&virtual_object->destroy_signal,
      &background->virtual_object_destroy_listener);

  return background;

err_resource:
  free(background);

err:
  return NULL;
}

WL_EXPORT void
zen_background_destroy(struct zen_background *background)
{
  background->virtual_object->role_object = NULL;
  wl_list_remove(&background->virtual_object_destroy_listener.link);
  free(background);
}
