#include "pointer.h"

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_pointer.h>

#include "backend.h"
#include "cursor.h"
#include "seat.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/server.h"

static void zn_pointer_destroy(struct zn_pointer *self);

static void
zn_pointer_handle_motion(struct wl_listener *listener UNUSED, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_motion *event = data;

  zn_seat_notify_pointer_motion(server->seat, event);
}

static void
zn_pointer_handle_button(struct wl_listener *listener UNUSED, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_button *event = data;

  zn_seat_notify_pointer_button(server->seat, event);
}

static void
zn_pointer_handle_axis(struct wl_listener *listener UNUSED, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_axis *event = data;

  zn_seat_notify_pointer_axis(server->seat, event);
}

static void
zn_pointer_handle_frame(struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();

  zn_seat_notify_pointer_frame(server->seat);
}

static void
zn_pointer_handle_wlr_input_device_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_pointer *self =
      zn_container_of(listener, self, wlr_input_device_destroy_listener);
  zn_pointer_destroy(self);
}

struct zn_pointer *
zn_pointer_get(struct zn_input_device_base *base)
{
  struct zn_pointer *self = zn_container_of(base, self, base);
  return self;
}

struct zn_pointer *
zn_pointer_create(struct wlr_input_device *wlr_input_device)
{
  struct zn_pointer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.wlr_input_device = wlr_input_device;
  wl_list_init(&self->base.link);

  self->motion_listener.notify = zn_pointer_handle_motion;
  wl_signal_add(&self->base.wlr_input_device->pointer->events.motion,
      &self->motion_listener);

  self->button_listener.notify = zn_pointer_handle_button;
  wl_signal_add(&self->base.wlr_input_device->pointer->events.button,
      &self->button_listener);

  self->axis_listener.notify = zn_pointer_handle_axis;
  wl_signal_add(
      &self->base.wlr_input_device->pointer->events.axis, &self->axis_listener);

  self->frame_listener.notify = zn_pointer_handle_frame;
  wl_signal_add(&self->base.wlr_input_device->pointer->events.frame,
      &self->frame_listener);

  self->wlr_input_device_destroy_listener.notify =
      zn_pointer_handle_wlr_input_device_destroy;
  wl_signal_add(&self->base.wlr_input_device->events.destroy,
      &self->wlr_input_device_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_pointer_destroy(struct zn_pointer *self)
{
  wl_list_remove(&self->wlr_input_device_destroy_listener.link);
  wl_list_remove(&self->frame_listener.link);
  wl_list_remove(&self->axis_listener.link);
  wl_list_remove(&self->button_listener.link);
  wl_list_remove(&self->motion_listener.link);
  wl_list_remove(&self->base.link);
  free(self);

  struct zn_server *server = zn_server_get_singleton();
  struct zn_default_backend *backend = zn_default_backend_get(server->backend);
  zn_default_backend_update_capabilities(backend);
}
