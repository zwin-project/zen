#include "drag-grab.h"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "virtual-object.h"

static void zen_drag_grab_destroy(struct zen_drag_grab* drag_grab);

static void
drag_grab_ray_focus(struct zen_ray_grab* grab)
{
  struct zen_ray* ray = grab->ray;
  struct zen_data_device* data_device = ray->seat->data_device;
  struct zen_shell_base* shell_base = ray->seat->compositor->shell_base;
  struct zen_virtual_object *new_focus, *current_focus;
  struct zen_data_offer* data_offer = NULL;
  struct wl_resource* new_focus_data_device_resource;
  char** type;

  current_focus =
      zen_weak_link_get_user_data(&data_device->focus_virtual_object_link);

  new_focus = shell_base->pick_virtual_object(shell_base, ray,
      ray->local_origin, ray->local_direction, &ray->target_distance);

  if (current_focus == new_focus) return;

  if (data_device->focus_resource) zen_data_device_clear_focus(data_device);

  if (data_device->data_source && data_device->data_source->data_offer) {
    // Unlink the offer from the source
    data_offer = data_device->data_source->data_offer;
    data_offer->data_source = NULL;
    data_device->data_source->data_offer = NULL;
    wl_list_remove(&data_offer->data_source_destroy_listener.link);
  }

  if (new_focus == NULL) {
    if (current_focus) zen_virtual_object_render_commit(current_focus);
    return;
  }

  new_focus_data_device_resource = wl_resource_find_for_client(
      &data_device->resource_list, wl_resource_get_client(new_focus->resource));
  if (new_focus_data_device_resource == NULL) return;

  data_device->focus_resource = new_focus_data_device_resource;

  if (data_device->data_source) {
    data_offer = zen_data_offer_create(
        data_device->data_source, new_focus_data_device_resource);
    if (data_offer == NULL) {
      zen_log("drag grab: failed to create a data offer\n");
      return;
    }
    zen_data_device_data_offer(data_device, data_offer);
    wl_array_for_each(type, &data_device->data_source->mime_type_list)
        zen_data_offer_offer(data_offer, *type);
  }

  zen_data_device_enter(data_device, new_focus, ray, data_offer);

  zen_weak_link_set(
      &data_device->focus_virtual_object_link, new_focus->resource);

  if (current_focus) zen_virtual_object_render_commit(current_focus);
  zen_virtual_object_render_commit(new_focus);
}

static void
drag_grab_ray_motion(struct zen_ray_grab* grab, const struct timespec* time,
    struct zen_ray_motion_event* event)
{
  struct zen_ray* ray = grab->ray;
  struct zen_data_device* data_device = ray->seat->data_device;

  // TODO: To move icon

  zen_ray_move(ray, event);

  zen_data_device_motion(data_device, ray, time);
}

static void
drag_grab_ray_button(struct zen_ray_grab* grab, const struct timespec* time,
    uint32_t button, uint32_t state)
{
  UNUSED(time);
  struct zen_ray* ray = grab->ray;
  struct zen_drag_grab* drag_grab = wl_container_of(grab, drag_grab, base);
  struct zen_data_device* data_device = ray->seat->data_device;
  struct zen_virtual_object* focus =
      zen_weak_link_get_user_data(&data_device->focus_virtual_object_link);

  if (data_device->data_source && ray->grab_button == button &&
      state == ZGN_RAY_BUTTON_STATE_RELEASED) {
    if (focus) {
      zen_data_device_drop(data_device);
      zen_data_source_dnd_drop_performed(data_device->data_source);
    } else {
      zen_data_source_cancelled(data_device->data_source);
    }
  }

  if (ray->button_count == 0 && state == ZGN_RAY_BUTTON_STATE_RELEASED) {
    zen_data_device_end_drag(data_device);
    zen_drag_grab_destroy(drag_grab);
  }
}

static void
drag_grab_ray_cancel(struct zen_ray_grab* grab)
{
  struct zen_ray* ray = grab->ray;
  struct zen_data_device* data_device = ray->seat->data_device;
  struct zen_drag_grab* drag_grab = wl_container_of(grab, drag_grab, base);

  zen_data_device_end_drag(data_device);
  zen_drag_grab_destroy(drag_grab);
}

static const struct zen_ray_grab_interface drag_grab_interface = {
    .focus = drag_grab_ray_focus,
    .motion = drag_grab_ray_motion,
    .button = drag_grab_ray_button,
    .cancel = drag_grab_ray_cancel,
};

WL_EXPORT struct zen_drag_grab*
zen_drag_grab_create(struct zen_data_device* data_device)
{
  struct zen_drag_grab* drag_grab;

  if (data_device->seat == NULL || data_device->seat->ray == NULL) goto err;

  drag_grab = zalloc(sizeof *drag_grab);
  if (drag_grab == NULL) {
    zen_log("drag grab: failed to allocate memory\n");
    goto err;
  }

  drag_grab->base.interface = &drag_grab_interface;
  drag_grab->base.ray = data_device->seat->ray;

  return drag_grab;

err:
  return NULL;
}

static void
zen_drag_grab_destroy(struct zen_drag_grab* drag_grab)
{
  free(drag_grab);
}
