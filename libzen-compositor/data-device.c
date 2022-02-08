#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "drag-grab.h"

static void
zen_data_device_start_drag(struct zen_data_device *data_device,
    struct zen_data_source *data_source, struct zen_virtual_object *icon)
{
  UNUSED(icon);
  struct zen_drag_grab *drag_grab;

  // FIXME: handle the case data source already exists
  data_device->data_source = data_source;

  if (data_device->seat->ray == NULL) return;

  drag_grab = zen_drag_grab_create(data_device, data_source);
  if (drag_grab == NULL) {
    zen_log("data device: failed to a drag grab");
    return;
  }

  zen_ray_clear_focus(data_device->seat->ray);
  zen_ray_grab_start(data_device->seat->ray, &drag_grab->base);
}

static void
zgn_data_device_protocol_start_drag(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *source_resource,
    struct wl_resource *origin_resource, struct wl_resource *icon_resource,
    uint32_t serial)
{
  UNUSED(client);
  UNUSED(icon_resource);
  struct zen_data_device *data_device;
  struct zen_seat *seat;
  struct zen_ray *ray;
  struct zen_data_source *data_source = NULL;
  struct zen_virtual_object *origin =
      wl_resource_get_user_data(origin_resource);
  struct zen_virtual_object *current_focus;

  data_device = wl_resource_get_user_data(resource);
  if (data_device == NULL) goto err;

  seat = data_device->seat;
  if (seat == NULL) goto err;

  ray = seat->ray;
  if (!ray || ray->button_count != 1 || ray->grab_serial != serial) goto err;

  current_focus = zen_weak_link_get_user_data(&ray->focus_virtual_object_link);
  if (!current_focus || current_focus != origin) goto err;

  if (source_resource) data_source = wl_resource_get_user_data(source_resource);
  zen_data_device_start_drag(data_device, data_source, NULL);

  return;

err:
  zgn_data_source_send_cancelled(source_resource);
}

static void
zgn_data_device_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zgn_data_device_interface data_device_interface = {
    .start_drag = zgn_data_device_protocol_start_drag,
    .release = zgn_data_device_protocol_release,
};

static void
zen_data_device_handle_destroy(struct wl_resource *resource)
{
  struct zen_data_device *data_device;

  data_device = wl_resource_get_user_data(resource);

  wl_list_remove(wl_resource_get_link(resource));

  if (data_device->focus_resource == resource) {
    data_device->focus_resource = NULL;
    zen_weak_link_unset(&data_device->focus_virtual_object_link);
  }
}

WL_EXPORT void
zen_data_device_end_drag(struct zen_data_device *data_device)
{
  zen_data_device_clear_focus(data_device);
  data_device->data_source = NULL;
  if (data_device->seat->ray != NULL) zen_ray_grab_end(data_device->seat->ray);
}

WL_EXPORT void
zen_data_device_data_offer(
    struct zen_data_device *data_device, struct zen_data_offer *data_offer)
{
  if (data_device->focus_resource == NULL) return;

  zgn_data_device_send_data_offer(
      data_device->focus_resource, data_offer->resource);
}

WL_EXPORT void
zen_data_device_enter(struct zen_data_device *data_device,
    struct zen_virtual_object *virtual_object, struct zen_ray *ray,
    struct zen_data_offer *data_offer)
{
  struct wl_array origin_array, direction_array;
  uint32_t serial;

  if (data_device->focus_resource == NULL) return;

  wl_array_init(&origin_array);
  wl_array_init(&direction_array);

  glm_vec3_to_wl_array(ray->local_origin, &origin_array);
  glm_vec3_to_wl_array(ray->local_direction, &direction_array);

  serial = wl_display_next_serial(virtual_object->compositor->display);

  zgn_data_device_send_enter(data_device->focus_resource, serial,
      virtual_object->resource, &origin_array, &direction_array,
      data_offer->resource);

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

WL_EXPORT void
zen_data_device_leave(struct zen_data_device *data_device)
{
  if (data_device->focus_resource == NULL) return;

  zgn_data_device_send_leave(data_device->focus_resource);
}

WL_EXPORT void
zen_data_device_motion(struct zen_data_device *data_device, struct zen_ray *ray,
    const struct timespec *time)
{
  struct wl_array origin_array, direction_array;
  uint32_t msecs;

  if (data_device->focus_resource == NULL) return;

  wl_array_init(&origin_array);
  wl_array_init(&direction_array);

  glm_vec3_to_wl_array(ray->local_origin, &origin_array);
  glm_vec3_to_wl_array(ray->local_direction, &direction_array);

  msecs = timespec_to_msec(time);

  zgn_data_device_send_motion(
      data_device->focus_resource, msecs, &origin_array, &direction_array);

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

WL_EXPORT void
zen_data_device_drop(struct zen_data_device *data_device)
{
  if (data_device->focus_resource == NULL) return;

  zgn_data_device_send_drop(data_device->focus_resource);
}

WL_EXPORT int
zen_data_device_add_resource(
    struct zen_data_device *data_device, struct wl_client *client, uint32_t id)
{
  struct wl_resource *resource;

  resource = wl_resource_create(client, &zgn_data_device_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("data device: failed to create a resource\n");
    goto err;
  }

  wl_resource_set_implementation(resource, &data_device_interface, data_device,
      zen_data_device_handle_destroy);

  wl_list_insert(&data_device->resource_list, wl_resource_get_link(resource));

  return 0;

err:
  return -1;
}

WL_EXPORT struct zen_data_device *
zen_data_device_create(struct zen_seat *seat)
{
  struct zen_data_device *data_device;

  data_device = zalloc(sizeof *data_device);
  if (data_device == NULL) {
    zen_log("data device: failed to allocate memory\n");
    goto err;
  }

  data_device->seat = seat;
  seat->data_device = data_device;

  data_device->data_source = NULL;

  data_device->focus_resource = NULL;
  zen_weak_link_init(&data_device->focus_virtual_object_link);
  wl_list_init(&data_device->resource_list);

  return data_device;

err:
  return NULL;
}

WL_EXPORT void
zen_data_device_clear_focus(struct zen_data_device *data_device)
{
  zen_data_device_leave(data_device);
  data_device->focus_resource = NULL;
  zen_weak_link_unset(&data_device->focus_virtual_object_link);
}

WL_EXPORT void
zen_data_device_destroy(struct zen_data_device *data_device)
{
  struct wl_resource *resource, *tmp;

  zen_weak_link_unset(&data_device->focus_virtual_object_link);

  wl_resource_for_each_safe(resource, tmp, &data_device->resource_list)
      wl_resource_set_user_data(resource, NULL);

  free(data_device);
}
