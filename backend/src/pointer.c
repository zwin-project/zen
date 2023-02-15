#include "zen/backend/default/pointer.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend/default/seat.h"

static void
zn_pointer_handle_motion(struct wl_listener *listener, void *data)
{
  struct zn_pointer *self = zn_container_of(listener, self, motion_listener);
  struct wlr_event_pointer_motion *wlr_event = data;
  struct zn_seat_pointer_motion_event event;

  event.time_msec = wlr_event->time_msec;
  event.delta_x = wlr_event->delta_x;
  event.delta_y = wlr_event->delta_y;
  event.unaccel_dx = wlr_event->unaccel_dx;
  event.unaccel_dy = wlr_event->unaccel_dy;

  zn_default_backend_seat_notify_motion(self->seat, &event);
}

static void
zn_pointer_handle_wlr_input_device_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_pointer *self =
      zn_container_of(listener, self, wlr_input_device_destroy_listener);
  zn_pointer_destroy(self);
}

static void
zn_pointer_handle_seat_destroy(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_pointer *self =
      zn_container_of(listener, self, seat_destroy_listener);
  zn_pointer_destroy(self);
}

struct zn_pointer *
zn_pointer_get(struct zn_input_device_base *base)
{
  struct zn_pointer *self = zn_container_of(base, self, base);
  return self;
}

struct zn_pointer *
zn_pointer_create(struct zn_default_backend_seat *seat,
    struct wlr_input_device *wlr_input_device)
{
  struct zn_pointer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.wlr_input_device = wlr_input_device;
  wl_list_init(&self->base.link);
  self->seat = seat;

  self->motion_listener.notify = zn_pointer_handle_motion;
  wl_signal_add(&self->base.wlr_input_device->pointer->events.motion,
      &self->motion_listener);

  self->wlr_input_device_destroy_listener.notify =
      zn_pointer_handle_wlr_input_device_destroy;
  wl_signal_add(&self->base.wlr_input_device->events.destroy,
      &self->wlr_input_device_destroy_listener);

  self->seat_destroy_listener.notify = zn_pointer_handle_seat_destroy;
  wl_signal_add(&self->seat->events.destroy, &self->seat_destroy_listener);

  return self;

err:
  return NULL;
}

void
zn_pointer_destroy(struct zn_pointer *self)
{
  struct zn_default_backend_seat *seat = self->seat;
  wl_list_remove(&self->base.link);
  wl_list_remove(&self->seat_destroy_listener.link);
  wl_list_remove(&self->wlr_input_device_destroy_listener.link);
  wl_list_remove(&self->motion_listener.link);
  free(self);

  zn_default_backend_seat_update_capabilities(seat);
}
