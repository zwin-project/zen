#include "seat.h"

#include <wlr/types/wlr_seat.h>

#include "input-device.h"
#include "seat-device.h"
#include "zen-common.h"

struct zn_seat {
  struct wlr_seat* wlr_seat;
  struct wl_list devices;  // zn_seat_device::link
};

static struct zn_seat_device*
zn_seat_get_device(struct zn_seat* self, struct zn_input_device* input_device)
{
  struct zn_seat_device* seat_device;
  wl_list_for_each(seat_device, &self->devices, link)
  {
    if (seat_device->input_device == input_device) {
      return seat_device;
    }
  }

  // TODO: keyboard group

  return NULL;
}

static void
zn_seat_update_capabilities(struct zn_seat* self)
{
  uint32_t caps = 0;
  uint32_t prev_caps = self->wlr_seat->capabilities;

  struct zn_seat_device* seat_device;
  wl_list_for_each(seat_device, &self->devices, link)
  {
    // TODO
    switch (zn_input_device_get_type(seat_device->input_device)) {
      case WLR_INPUT_DEVICE_KEYBOARD:
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
        break;
      case WLR_INPUT_DEVICE_POINTER:
        caps |= WL_SEAT_CAPABILITY_POINTER;
        break;
      case WLR_INPUT_DEVICE_TOUCH:
        break;
      case WLR_INPUT_DEVICE_TABLET_TOOL:
        caps |= WL_SEAT_CAPABILITY_POINTER;
        break;
      case WLR_INPUT_DEVICE_TABLET_PAD:
      case WLR_INPUT_DEVICE_SWITCH:
        break;
    }
  }

  wlr_seat_set_capabilities(self->wlr_seat, caps);
  if (zn_seat_has_capabilities(self, WL_SEAT_CAPABILITY_POINTER)) {
    // pointer device is connected just now, so show cursor image
    if (!(prev_caps & WL_SEAT_CAPABILITY_POINTER)) {
      // TODO: show cursor image
    }
  } else {
    // no pointer devices are connected, so hide cursor image
    // TODO: hide cursor image
  }
}

void
zn_seat_configure_device(
    struct zn_seat* self, struct zn_input_device* input_device)
{
  struct zn_seat_device* seat_device = zn_seat_get_device(self, input_device);

  if (seat_device) {
    wl_list_for_each(seat_device, &self->devices, link)
    {
      // TODO
      switch (zn_input_device_get_type(seat_device->input_device)) {
        case WLR_INPUT_DEVICE_KEYBOARD:
          break;
        case WLR_INPUT_DEVICE_POINTER:
          break;
        case WLR_INPUT_DEVICE_TOUCH:
          break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
          break;
        case WLR_INPUT_DEVICE_TABLET_PAD:
          break;
        case WLR_INPUT_DEVICE_SWITCH:
          break;
      }
    }
  }
}

void
zn_seat_add_device(struct zn_seat* self, struct zn_input_device* input_device)
{
  if (zn_seat_get_device(self, input_device)) {
    zn_seat_configure_device(self, input_device);
    return;
  }

  struct zn_seat_device* seat_device =
      zn_seat_device_create(self, input_device, &self->devices);
  if (seat_device == NULL) {
    zn_error("Failed to create zn_seat_device");
    return;
  }

  zn_seat_configure_device(self, input_device);
  zn_seat_update_capabilities(self);
}

bool
zn_seat_has_capabilities(struct zn_seat* self, enum wl_seat_capability type)
{
  return (self->wlr_seat->capabilities & type) != 0;
}

struct zn_seat*
zn_seat_create(struct wl_display* display, const char* seat_name)
{
  struct zn_seat* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_seat = wlr_seat_create(display, seat_name);
  if (self->wlr_seat == NULL) {
    zn_error("Failed to create wlr_seat");
    goto err_free;
  }

  wl_list_init(&self->devices);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat* self)
{
  wl_list_remove(&self->devices);
  wlr_seat_destroy(self->wlr_seat);
  free(self);
}
