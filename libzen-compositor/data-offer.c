#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <libzen-compositor/libzen-compositor.h>
#include <unistd.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

void
zen_data_offer_offer(struct zen_data_offer *data_offer, const char *mime_type)
{
  zgn_data_offer_send_offer(data_offer->resource, mime_type);
}

static void
zen_data_offer_protocol_accept(struct wl_client *client,
    struct wl_resource *resource, uint32_t serial, const char *mime_type)
{
  struct zen_data_offer *data_offer = wl_resource_get_user_data(resource);

  // TODO: 理解 /* Protect against untimely calls from older data offers */
  if (!data_offer->data_source ||
      data_offer != data_offer->data_source->data_offer)
    return;

  zen_data_source_target(data_offer->data_source, mime_type);
}

static void
zen_data_offer_protocol_receive(struct wl_client *client,
    struct wl_resource *resource, const char *mime_type, int32_t fd)
{
  struct zen_data_offer *data_offer = wl_resource_get_user_data(resource);

  if (data_offer->data_source &&
      data_offer == data_offer->data_source->data_offer) {
    zen_data_source_send(data_offer->data_source, mime_type, fd);
  }
  close(fd);
}

static void
zen_data_offer_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{}

static void
zen_data_offer_protocol_finish(
    struct wl_client *client, struct wl_resource *resource)
{
  struct zen_data_offer *data_offer = wl_resource_get_user_data(resource);

  if (!data_offer->data_source ||
      data_offer != data_offer->data_source->data_offer)  // TODO: 理解
    return;

  zen_data_source_dnd_finished(data_offer->data_source);
}

static void
zen_data_offer_protocol_set_actions(struct wl_client *client,
    struct wl_resource *resource, uint32_t dnd_actions,
    uint32_t preferred_action)
{}

static const struct zgn_data_offer_interface data_offer_interface = {
    .accept = zen_data_offer_protocol_accept,
    .receive = zen_data_offer_protocol_receive,
    .destroy = zen_data_offer_protocol_destroy,
    .finish = zen_data_offer_protocol_finish,
    .set_actions = zen_data_offer_protocol_set_actions,
};

static void
zen_data_offer_handle_destroy(struct wl_resource *resource)
{
  // TODO
}

WL_EXPORT struct zen_data_offer *
zen_data_offer_create(
    struct zen_data_source *data_source, struct wl_resource *target)
{
  struct wl_client *client = wl_resource_get_client(target);
  struct zen_data_offer *data_offer;

  data_offer = zalloc(sizeof *data_offer);
  if (data_offer == NULL) {
    zen_log("data offer: failed to allocate memory\n");
    wl_client_post_no_memory(client);
    goto err;
  }

  data_offer->resource =
      wl_resource_create(client, &zgn_data_offer_interface, 1, 0);
  if (data_offer->resource == NULL) {
    zen_log("data offer: failed to create a resource\n");
    wl_client_post_no_memory(client);
    goto err_resource;
  }

  wl_resource_set_implementation(data_offer->resource, &data_offer_interface,
      data_offer, zen_data_offer_handle_destroy);

  data_offer->data_source = data_source;
  data_source->data_offer = data_offer;

  return data_offer;

err_resource:
  free(data_offer);

err:
  return NULL;
}

void
zen_data_offer_destroy(struct zen_data_offer *data_offer)
{
  // TODO
  free(data_offer);
}
