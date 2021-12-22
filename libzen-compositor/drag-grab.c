#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "drag-grab.h"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "virtual-object.h"

struct zen_drag_grab {
  struct zen_ray_grab base;
};

static void zen_drag_grab_destroy(struct zen_drag_grab* drag_grab);

static void
drag_grab_ray_focus(struct zen_ray_grab* grab)
{
  struct zen_ray* ray = grab->ray;
  struct zen_data_device* data_device = ray->seat->data_device;
  struct zen_shell_base* shell_base = ray->seat->compositor->shell_base;
  struct zen_virtual_object *virtual_object, *current_focus;
  struct zen_data_offer* data_offer = NULL;
  struct wl_resource* target;
  char** type;

  current_focus =
      zen_weak_link_get_user_data(&data_device->focus_virtual_object_link);

  virtual_object = shell_base->pick_virtual_object(shell_base, ray,
      ray->local_origin, ray->local_direction, &ray->target_distance);

  if (current_focus == virtual_object) return;

  if (current_focus) zen_data_device_leave(data_device);

  target = wl_resource_find_for_client(&data_device->resource_list,
      wl_resource_get_client(virtual_object->resource));
  if (!target) return;

  // TODO: targetをdata_deviceに入れるべき説濃厚, というかtargetは
  // TODO:
  // focusでやるべきこと整理してどこをどの場所でやるか後で再整理する(westonはあってるだろうけどコードとして最適解ではない)
  // TODO: data_deviceにどのタイミングでdata_sourceを代入するか?
  if (data_device->data_source) {
    data_offer = zen_data_offer_create(data_device->data_source, target);
    if (data_offer == NULL) return;

    zen_data_device_data_offer(data_device, target, data_offer);

    wl_array_for_each(type, &data_device->data_source->mime_type_list)
        zen_data_offer_offer(data_offer, *type);
  }

  if (virtual_object)
    zen_data_device_enter(data_device, target, virtual_object, ray, data_offer);

  if (virtual_object)
    zen_weak_link_set(
        &data_device->focus_virtual_object_link, virtual_object->resource);
  else
    zen_weak_link_unset(&data_device->focus_virtual_object_link);

  if (current_focus) zen_virtual_object_render_commit(current_focus);
  if (virtual_object) zen_virtual_object_render_commit(virtual_object);

  // FIXME: dnd終わった後のrayのfocus
}

static void
drag_grab_ray_motion(struct zen_ray_grab* grab, const struct timespec* time,
    struct zen_ray_motion_event* event)
{
  struct zen_ray* ray = grab->ray;
  struct zen_data_device* data_device = ray->seat->data_device;
  // FIXME: seat->data_deviceがないことあり得る？

  // ここでicon動かす

  zen_ray_move(ray, event);  // focus呼ばれる

  zen_data_device_motion(data_device, ray, time);
}

static void
drag_grab_ray_button(struct zen_ray_grab* grab, const struct timespec* time,
    uint32_t button, uint32_t state)
{
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
    zen_ray_grab_end(ray);
    zen_drag_grab_destroy(drag_grab);
  }
}

static void
drag_grab_ray_cancel(struct zen_ray_grab* grab)
{
  struct zen_ray* ray = grab->ray;
  struct zen_drag_grab* drag_grab = wl_container_of(grab, drag_grab, base);

  zen_ray_grab_end(ray);
  zen_drag_grab_destroy(drag_grab);
}

static const struct zen_ray_grab_interface drag_grab_interface = {
    .focus = drag_grab_ray_focus,
    .motion = drag_grab_ray_motion,
    .button = drag_grab_ray_button,
    .cancel = drag_grab_ray_cancel,
};

static struct zen_drag_grab*
zen_drag_grab_create(struct wl_client* client)
{
  struct zen_drag_grab* drag_grab;

  drag_grab = zalloc(sizeof *drag_grab);
  if (drag_grab == NULL) {
    zen_log("drag grab: failed to allocate memory\n");
    wl_client_post_no_memory(client);
    goto err;
  }

  drag_grab->base.interface = &drag_grab_interface;

  return drag_grab;

err:
  return NULL;
}

WL_EXPORT int
zen_ray_start_drag(struct zen_ray* ray, struct zen_data_source* data_source,
    struct zen_virtual_object* icon, struct wl_client* client)
{
  struct zen_drag_grab* drag_grab;

  drag_grab = zen_drag_grab_create(client);
  if (drag_grab == NULL) {
    return -1;
  }

  zen_ray_grab_start(ray, &drag_grab->base);

  return 0;
}

static void
zen_drag_grab_destroy(struct zen_drag_grab* drag_grab)
{
  free(drag_grab);
}
