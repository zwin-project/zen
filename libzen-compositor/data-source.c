#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <libzen-compositor/libzen-compositor.h>
#include <string.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

WL_EXPORT void
zen_data_source_target(
    struct zen_data_source *data_source, const char *mime_type)
{
  zgn_data_source_send_target(data_source->resource, mime_type);
}

WL_EXPORT void
zen_data_source_send(
    struct zen_data_source *data_source, const char *mime_type, int32_t fd)
{
  zgn_data_source_send_send(data_source->resource, mime_type, fd);
}

WL_EXPORT void
zen_data_source_cancelled(struct zen_data_source *data_source)
{
  zgn_data_source_send_cancelled(data_source->resource);
}

WL_EXPORT
void
zen_data_source_dnd_drop_performed(struct zen_data_source *data_source)
{
  zgn_data_source_send_dnd_drop_performed(data_source->resource);
}

WL_EXPORT
void
zen_data_source_dnd_finished(struct zen_data_source *data_source)
{
  zgn_data_source_send_dnd_finished(data_source->resource);

  data_source->data_offer = NULL;
}

static void
zen_data_source_protocol_offer(struct wl_client *client,
    struct wl_resource *resource, const char *mime_type)
{
  struct zen_data_source *data_source;
  char **type;

  data_source = wl_resource_get_user_data(resource);

  type = wl_array_add(&data_source->mime_type_list, sizeof *type);
  if (type) *type = strdup(mime_type);
  if (!type || !*type) {
    zen_log("data source: failed to allocate memory\n");
    wl_client_post_no_memory(client);
  }
}

static void
zen_data_source_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static void
zen_data_source_protocol_set_actions(struct wl_client *client,
    struct wl_resource *resource, uint32_t dnd_actions)
{
  // TODO
}

struct zgn_data_source_interface data_source_interface = {
    .offer = zen_data_source_protocol_offer,
    .destroy = zen_data_source_protocol_destroy,
    .set_actions = zen_data_source_protocol_set_actions,
};

static void
zen_data_source_handle_destroy(struct wl_resource *resource)
{
  struct zen_data_source *data_source = wl_resource_get_user_data(resource);
  char **type;

  wl_signal_emit(&data_source->destroy_signal, data_source);

  wl_array_for_each(type, &data_source->mime_type_list) free(*type);

  wl_array_release(&data_source->mime_type_list);

  free(data_source);
}

WL_EXPORT struct zen_data_source *
zen_data_source_create(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  struct zen_data_source *data_source;

  data_source = zalloc(sizeof *data_source);
  if (data_source == NULL) {
    zen_log("data source: failed to allocate memory\n");
    wl_client_post_no_memory(client);
    goto err;
  }

  data_source->resource =
      wl_resource_create(client, &zgn_data_source_interface, 1, id);
  if (data_source->resource == NULL) {
    zen_log("data source: failed to create a resource\n");
    wl_client_post_no_memory(client);
    goto err_resource;
  }

  wl_resource_set_implementation(data_source->resource, &data_source_interface,
      data_source, zen_data_source_handle_destroy);

  wl_array_init(&data_source->mime_type_list);

  data_source->data_offer = NULL;

  wl_signal_init(&data_source->destroy_signal);

  return data_source;

err_resource:
  free(data_source);

err:
  return NULL;
}
