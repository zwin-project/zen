#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-shell-server-protocol.h>

struct zen_shell {
  struct zen_compositor* compositor;

  struct wl_global* global;
};

static void
zen_shell_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_shell_protocol_get_cuboid_window(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object, struct wl_array* half_size)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
  UNUSED(virtual_object);
  UNUSED(half_size);
}

static const struct zgn_shell_interface shell_interface = {
    .destroy = zen_shell_protocol_destroy,
    .get_cuboid_window = zen_shell_protocol_get_cuboid_window,
};

static void
zen_shell_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zen_shell* shell = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &zgn_shell_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("shell: failed to create a resource\n");
    return;
  }

  wl_resource_set_implementation(resource, &shell_interface, shell, NULL);
}

WL_EXPORT struct zen_shell*
zen_shell_create(struct zen_compositor* compositor)
{
  struct zen_shell* shell;
  struct wl_global* global;

  shell = zalloc(sizeof *shell);
  if (shell == NULL) {
    zen_log("shell: failed to allocate memory\n");
    goto err;
  }

  global = wl_global_create(
      compositor->display, &zgn_shell_interface, 1, shell, zen_shell_bind);
  if (global == NULL) {
    zen_log("shell: failed to create a zen_shell global\n");
    goto err_global;
  }

  shell->compositor = compositor;
  shell->global = global;

  return shell;

err_global:
  free(shell);

err:
  return NULL;
}

WL_EXPORT void
zen_shell_destroy(struct zen_shell* shell)
{
  wl_global_destroy(shell->global);
  free(shell);
}
