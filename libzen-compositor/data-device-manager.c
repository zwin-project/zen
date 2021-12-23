#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

static void
zgn_data_device_manager_protocol_create_data_source(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  struct zen_data_device_manager *data_device_manager =
      wl_resource_get_user_data(resource);
  struct zen_data_source *data_source;

  data_source = zen_data_source_create(client, resource, id);
  if (data_source == NULL) {
    wl_client_post_no_memory(client);
    zen_log("data device manager: failed to create a data source\n");
  }

  if (data_device_manager->data_device)
    data_device_manager->data_device->data_source = data_source;
}

static void
zgn_data_device_manager_protocol_get_data_device(struct wl_client *client,
    struct wl_resource *manager_resource, uint32_t id,
    struct wl_resource *seat_resource)
{
  struct zen_data_device_manager *data_device_manager =
      wl_resource_get_user_data(manager_resource);
  struct zen_seat *seat = wl_resource_get_user_data(seat_resource);
  struct zen_data_device *data_device;

  if (seat) {
    data_device = zen_data_device_ensure(client, seat);
    zen_data_device_add_resource(data_device, client, id);
    data_device_manager->data_device = data_device;
  } else {
    // TODO: seatがない場合へ対処
    zen_data_device_create_insert_resource(client, id);
  }
}

struct zgn_data_device_manager_interface data_device_manager_interface = {
    .create_data_source = zgn_data_device_manager_protocol_create_data_source,
    .get_data_device = zgn_data_device_manager_protocol_get_data_device,
};

static void
zen_data_device_manager_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zen_data_device_manager *data_device_manager = data;
  struct wl_resource *resource;

  resource = wl_resource_create(
      client, &zgn_data_device_manager_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("data device manager: failed to create a resource\n");
    return;
  }

  wl_resource_set_implementation(
      resource, &data_device_manager_interface, data_device_manager, NULL);
  // TODO: zgn_data_device_managerでresourceのリストは保持してないので,
  // resourceのdestroy handlerはいらない?
}

WL_EXPORT struct zen_data_device_manager *
zen_data_device_manager_create(struct wl_display *display)
{
  struct zen_data_device_manager *data_device_manager;
  struct wl_global *global;

  data_device_manager = zalloc(sizeof *data_device_manager);
  if (data_device_manager == NULL) {
    zen_log("data device manager: failed to allocate memory\n");
    goto err;
  }

  global = wl_global_create(display, &zgn_data_device_manager_interface, 1,
      data_device_manager, zen_data_device_manager_bind);
  if (global == NULL) {
    zen_log(
        "data device manager: failed to create a data_device_manager global\n");
    goto err_global;
  }

  data_device_manager->global = global;

  return data_device_manager;

err_global:
  free(data_device_manager);

err:
  return NULL;
}

WL_EXPORT
void
zen_data_device_manager_destroy(
    struct zen_data_device_manager *data_device_manager)
{
  wl_global_destroy(data_device_manager->global);
  free(data_device_manager);
}
