#include "zen/input/pointer.h"

#include <cglm/vec3.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_surface.h>

#include "zen-common.h"
#include "zen/cursor.h"
#include "zen/ray.h"
#include "zen/server.h"

static void
zn_pointer_handle_motion(struct wl_listener *listener, void *data)
{
  UNUSED(listener);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_motion *event = data;

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    struct zn_cursor *cursor = server->scene->cursor;
    cursor->grab->impl->motion_relative(
        cursor->grab, event->delta_x, event->delta_y, event->time_msec);
  } else {
    struct zn_ray *ray = server->scene->ray;

    ray->grab->impl->motion_relative(ray->grab, GLM_VEC3_ZERO,
        event->delta_y * 0.001, -event->delta_x * 0.001, event->time_msec);
  }
}

static void
zn_pointer_handle_button(struct wl_listener *listener, void *data)
{
  UNUSED(listener);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_button *event = data;

  if (event->state == WLR_BUTTON_PRESSED) {
    server->input_manager->seat->pressing_button_count++;
  } else {
    server->input_manager->seat->pressing_button_count--;
  }

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    struct zn_cursor *cursor = server->scene->cursor;
    cursor->grab->impl->button(
        cursor->grab, event->time_msec, event->button, event->state);
  } else {
    struct zn_ray *ray = server->scene->ray;

    ray->grab->impl->button(
        ray->grab, event->time_msec, event->button, event->state);
  }
}

static void
zn_pointer_handle_axis(struct wl_listener *listener, void *data)
{
  UNUSED(listener);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_axis *event = data;

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    struct zn_cursor *cursor = server->scene->cursor;
    cursor->grab->impl->axis(cursor->grab, event->time_msec, event->source,
        event->orientation, event->delta, event->delta_discrete);
  } else {
    struct zn_ray *ray = server->scene->ray;
    ray->grab->impl->axis(ray->grab, event->time_msec, event->source,
        event->orientation, event->delta, event->delta_discrete);
  }
}

static void
zn_pointer_handle_frame(struct wl_listener *listener, void *data)
{
  UNUSED(listener);
  UNUSED(data);
  struct zn_server *server = zn_server_get_singleton();

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    struct zn_cursor *cursor = server->scene->cursor;
    cursor->grab->impl->frame(cursor->grab);
  } else {
    struct zn_ray *ray = server->scene->ray;
    ray->grab->impl->frame(ray->grab);
  }
}

struct zn_pointer *
zn_pointer_create(struct wlr_input_device *wlr_input_device)
{
  struct zn_pointer *self;

  if (!zn_assert(wlr_input_device->type == WLR_INPUT_DEVICE_POINTER,
          "Wrong type - expect: %d, actual: %d", WLR_INPUT_DEVICE_POINTER,
          wlr_input_device->type)) {
    goto err;
  }

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->motion_listener.notify = zn_pointer_handle_motion;
  wl_signal_add(
      &wlr_input_device->pointer->events.motion, &self->motion_listener);

  self->button_listener.notify = zn_pointer_handle_button;
  wl_signal_add(
      &wlr_input_device->pointer->events.button, &self->button_listener);

  self->axis_listener.notify = zn_pointer_handle_axis;
  wl_signal_add(&wlr_input_device->pointer->events.axis, &self->axis_listener);

  self->frame_listener.notify = zn_pointer_handle_frame;
  wl_signal_add(
      &wlr_input_device->pointer->events.frame, &self->frame_listener);

  return self;

err:
  return NULL;
}

void
zn_pointer_destroy(struct zn_pointer *self)
{
  wl_list_remove(&self->motion_listener.link);
  wl_list_remove(&self->button_listener.link);
  wl_list_remove(&self->axis_listener.link);
  wl_list_remove(&self->frame_listener.link);
  free(self);
}
