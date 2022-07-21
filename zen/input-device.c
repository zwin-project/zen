#include "zen/input-device.h"

#include <wayland-server.h>

#include "zen-common.h"
#include "zen/seat.h"

static void
zn_input_device_handle_device_destroy(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_input_device* self =
      zn_container_of(listener, self, device_destroy_listener);
  zn_input_device_destroy(self);
}

struct zn_input_device*
zn_input_device_create(struct zn_seat* seat, struct wlr_input_device* wlr_input)
{
  struct zn_input_device* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->seat = seat;
  self->wlr_input = wlr_input;
  wlr_input->data = self;

  self->device_destroy_listener.notify = zn_input_device_handle_device_destroy;
  wl_signal_add(&wlr_input->events.destroy, &self->device_destroy_listener);

  zn_seat_add_device(self->seat, self);

  return self;

err:
  return NULL;
}

void
zn_input_device_destroy(struct zn_input_device* self)
{
  zn_seat_remove_device(self->seat, self);
  wl_list_remove(&self->device_destroy_listener.link);
  free(self);
}
