#include "pointer.h"

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_pointer.h>

#include "backend.h"
#include "cursor.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/seat.h"
#include "zen/server.h"

static void
zn_pointer_handle_motion(struct wl_listener *listener UNUSED, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_event_pointer_motion *wlr_event = data;
  struct zn_cursor_motion_event event;

  event.time_msec = wlr_event->time_msec;
  event.delta_x = wlr_event->delta_x;
  event.delta_y = wlr_event->delta_y;
  event.unaccel_dx = wlr_event->unaccel_dx;
  event.unaccel_dy = wlr_event->unaccel_dy;

  zn_cursor_notify_motion(server->seat->cursor, &event);
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
zn_pointer_handle_backend_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_pointer *self =
      zn_container_of(listener, self, backend_destroy_listener);
  zn_pointer_destroy(self);
}

struct zn_pointer *
zn_pointer_get(struct zn_input_device_base *base)
{
  struct zn_pointer *self = zn_container_of(base, self, base);
  return self;
}

struct zn_pointer *
zn_pointer_create(
    struct zn_backend *backend, struct wlr_input_device *wlr_input_device)
{
  struct zn_pointer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->backend = backend;

  self->base.wlr_input_device = wlr_input_device;
  wl_list_init(&self->base.link);

  self->motion_listener.notify = zn_pointer_handle_motion;
  wl_signal_add(&self->base.wlr_input_device->pointer->events.motion,
      &self->motion_listener);

  self->wlr_input_device_destroy_listener.notify =
      zn_pointer_handle_wlr_input_device_destroy;
  wl_signal_add(&self->base.wlr_input_device->events.destroy,
      &self->wlr_input_device_destroy_listener);

  self->backend_destroy_listener.notify = zn_pointer_handle_backend_destroy;
  wl_signal_add(
      &self->backend->events.destroy, &self->backend_destroy_listener);

  return self;

err:
  return NULL;
}

void
zn_pointer_destroy(struct zn_pointer *self)
{
  struct zn_backend *backend = self->backend;
  wl_list_remove(&self->backend_destroy_listener.link);
  wl_list_remove(&self->wlr_input_device_destroy_listener.link);
  wl_list_remove(&self->motion_listener.link);
  wl_list_remove(&self->base.link);
  free(self);

  zn_backend_update_capabilities(backend);
}
