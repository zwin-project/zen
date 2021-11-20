#include "virtual-object.h"

#include <libzen/libzen.h>
#include <wayland-client.h>
#include <zigen-server-protocol.h>

#include "callback.h"
#include "compositor.h"

static void zen_virtual_object_destroy(
    struct zen_virtual_object *virtual_object);

static void
zen_virtual_object_handle_destroy(struct wl_resource *resource)
{
  struct zen_virtual_object *virtual_object;

  virtual_object = wl_resource_get_user_data(resource);

  zen_virtual_object_destroy(virtual_object);
}

static void
zen_virtual_object_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_virtual_object_protocol_commit(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  struct zen_virtual_object *virtual_object;

  virtual_object = wl_resource_get_user_data(resource);

  wl_list_insert_list(&virtual_object->frame_callback_list,
      &virtual_object->pending.frame_callback_list);
  wl_list_init(&virtual_object->pending.frame_callback_list);

  wl_signal_emit(&virtual_object->commit_signal, NULL);
}

static void
zen_virtual_object_protocol_frame(struct wl_client *client,
    struct wl_resource *resource, uint32_t callback_id)
{
  struct zen_virtual_object *virtual_object;
  struct zen_callback *callback;

  virtual_object = wl_resource_get_user_data(resource);

  callback = zen_callback_create(client, callback_id);
  if (callback == NULL) {
    zen_log("virtual object: failed to create callback\n");
    wl_client_post_no_memory(client);
    return;
  }

  wl_list_insert(&virtual_object->pending.frame_callback_list, &callback->link);
}

static const struct zgn_virtual_object_interface virtual_object_interface = {
    .destroy = zen_virtual_object_protocol_destroy,
    .commit = zen_virtual_object_protocol_commit,
    .frame = zen_virtual_object_protocol_frame,
};

static void
zen_virtual_object_compositor_frame_signal_handler(
    struct wl_listener *listener, void *data)
{
  uint32_t frame_time_msec = *(uint32_t *)data;
  struct zen_virtual_object *virtual_object;
  struct zen_callback *tmp, *frame_callback;

  virtual_object = wl_container_of(
      listener, virtual_object, compositor_frame_signal_listener);

  wl_list_for_each_safe(
      frame_callback, tmp, &virtual_object->frame_callback_list, link)
  {
    zen_callback_done(frame_callback, frame_time_msec);
  }

  wl_list_init(&virtual_object->frame_callback_list);
}

WL_EXPORT struct zen_virtual_object *
zen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zen_compositor *compositor)
{
  struct zen_virtual_object *virtual_object;
  struct wl_resource *resource;

  virtual_object = zalloc(sizeof *virtual_object);
  if (virtual_object == NULL) {
    zen_log("virtual object: failed to create virtual object\n");
    wl_client_post_no_memory(client);
    goto err;
  }

  resource = wl_resource_create(client, &zgn_virtual_object_interface, 1, id);
  if (resource == NULL) {
    zen_log("virtual object: failed to create resource\n");
    wl_client_post_no_memory(client);
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &virtual_object_interface,
      virtual_object, zen_virtual_object_handle_destroy);

  virtual_object->compositor = compositor;
  wl_signal_init(&virtual_object->commit_signal);

  virtual_object->compositor_frame_signal_listener.notify =
      zen_virtual_object_compositor_frame_signal_handler;
  wl_signal_add(&compositor->frame_signal,
      &virtual_object->compositor_frame_signal_listener);

  wl_list_init(&virtual_object->frame_callback_list);
  wl_list_init(&virtual_object->pending.frame_callback_list);

  return virtual_object;

err_resource:
  free(virtual_object);

err:
  return NULL;
}

static void
zen_virtual_object_destroy(struct zen_virtual_object *virtual_object)
{
  wl_list_remove(&virtual_object->compositor_frame_signal_listener.link);
  free(virtual_object);
}
