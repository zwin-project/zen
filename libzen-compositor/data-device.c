#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "drag-grab.h"

WL_EXPORT void
zen_data_device_data_offer(struct zen_data_device *data_device,
    struct wl_resource *target, struct zen_data_offer *data_offer)
{
  zgn_data_device_send_data_offer(target, data_offer->resource);
}

WL_EXPORT void
zen_data_device_enter(struct zen_data_device *data_device,
    struct wl_resource *target, struct zen_virtual_object *virtual_object,
    struct zen_ray *ray, struct zen_data_offer *data_offer)
{
  struct wl_array origin_array, direction_array;
  uint32_t serial;

  wl_array_init(&origin_array);
  wl_array_init(&direction_array);

  glm_vec3_to_wl_array(ray->local_origin, &origin_array);
  glm_vec3_to_wl_array(ray->local_direction, &direction_array);

  serial = wl_display_next_serial(virtual_object->compositor->display);

  zgn_data_device_send_enter(target, serial, virtual_object->resource,
      &origin_array, &direction_array, data_offer->resource);

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

WL_EXPORT void
zen_data_device_leave(struct zen_data_device *data_device)
{
  struct zen_virtual_object *focus_virtual_object;

  focus_virtual_object =
      zen_weak_link_get_user_data(&data_device->focus_virtual_object_link);
  if (focus_virtual_object == NULL) return;

  zgn_data_device_send_leave(focus_virtual_object->resource);
}

WL_EXPORT void
zen_data_device_motion(struct zen_data_device *data_device, struct zen_ray *ray,
    const struct timespec *time)
{
  struct zen_virtual_object *focus;
  struct wl_array origin_array, direction_array;
  uint32_t msecs;

  focus = zen_weak_link_get_user_data(&data_device->focus_virtual_object_link);
  if (focus == NULL) return;

  wl_array_init(&origin_array);
  wl_array_init(&direction_array);

  glm_vec3_to_wl_array(ray->local_origin, &origin_array);
  glm_vec3_to_wl_array(ray->local_direction, &direction_array);

  msecs = timespec_to_msec(time);

  zgn_data_device_send_motion(
      focus->resource, msecs, &origin_array, &direction_array);

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

WL_EXPORT void
zen_data_device_drop(struct zen_data_device *data_device)
{
  struct zen_virtual_object *focus;

  focus = zen_weak_link_get_user_data(&data_device->focus_virtual_object_link);
  if (focus == NULL) return;

  zgn_data_device_send_drop(focus->resource);
}

static void zen_data_device_destroy(struct zen_data_device *data_device);

static void
zgn_data_device_protocol_start_drag(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *source_resource,
    struct wl_resource *origin_resource, struct wl_resource *icon_resource,
    uint32_t serial)
{
  struct zen_data_device *data_device;
  struct zen_seat *seat;
  struct zen_ray *ray;
  struct zen_data_source *data_source;
  struct zen_virtual_object *origin;
  struct zen_virtual_object *icon;
  struct zen_virtual_object *current_focus;

  data_device = wl_resource_get_user_data(resource);
  if (data_device == NULL) return;

  seat = data_device->seat;
  if (seat == NULL) return;

  ray = seat->ray;
  if (!ray || ray->button_count != 1 || ray->grab_serial != serial) return;

  data_source = wl_resource_get_user_data(source_resource);
  origin = wl_resource_get_user_data(origin_resource);
  icon = wl_resource_get_user_data(icon_resource);
  current_focus = zen_weak_link_get_user_data(&ray->focus_virtual_object_link);

  if (!current_focus || current_focus != origin) return;

  // FIXME: set icon role

  zen_ray_start_drag(ray, data_source, icon, client);
}

static void
zgn_data_device_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  // TODO
}

static const struct zgn_data_device_interface data_device_interface = {
    .start_drag = zgn_data_device_protocol_start_drag,
    .release = zgn_data_device_protocol_release,
};

static void
zen_data_device_handle_destroy(struct wl_resource *resource)
{
  // TODO: destroyの処理をちゃんと考える
  struct zen_data_device *data_device;

  data_device = wl_resource_get_user_data(resource);

  wl_list_remove(wl_resource_get_link(resource));

  if (data_device && wl_list_empty(&data_device->resource_list)) {
    zen_data_device_destroy(data_device);
  }
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

static struct zen_data_device *
zen_data_device_create(struct wl_client *client, struct zen_seat *seat)
{
  struct zen_data_device *data_device;

  data_device = zalloc(sizeof *data_device);
  if (data_device == NULL) {
    wl_client_post_no_memory(client);
    zen_log("data device: failed to allocate memory\n");
    goto err;
  }

  data_device->seat = seat;
  seat->data_device = data_device;

  wl_list_init(&data_device->resource_list);

  return data_device;

err:
  return NULL;
}

WL_EXPORT struct zen_data_device *
zen_data_device_ensure(struct wl_client *client, struct zen_seat *seat)
{
  if (seat->data_device) {
    return seat->data_device;
  } else {
    return zen_data_device_create(client, seat);
  }
}

WL_EXPORT struct wl_resource *
zen_data_device_create_insert_resource(struct wl_client *client, uint32_t id)
{
  // TODO
  struct wl_resource *resource;

  resource = wl_resource_create(client, &zgn_data_device_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("data device: failed to create a resource\n");
    goto err;
  }

  wl_resource_set_implementation(resource, &data_device_interface, NULL, NULL);

  return resource;

err:
  return NULL;
}

static void
zen_data_device_destroy(struct zen_data_device *data_device)
{
  // TODO: ちゃんと考える
  struct wl_resource *resource, *tmp;
  wl_resource_for_each_safe(resource, tmp, &data_device->resource_list)
  {
    wl_resource_set_user_data(resource, NULL);
    wl_resource_set_destructor(resource, NULL);
    wl_list_init(wl_resource_get_link(resource));
  }

  free(data_device);
}
