#include "seat-device.h"

#include <wayland-server.h>

#include "zen-common.h"

struct zn_seat_device*
zn_seat_device_create(struct zn_seat* seat,
    struct zn_input_device* input_device, struct wl_list* devices)
{
  struct zn_seat_device* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->seat = seat;
  self->input_device = input_device;
  wl_list_insert(devices, &self->link);

  return self;

err:
  return NULL;
}

void
zn_seat_device_destroy(struct zn_seat_device* self)
{
  wl_list_remove(&self->link);
  free(self);
}
