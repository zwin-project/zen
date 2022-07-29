#include "zen/seat.h"

#include <wlr/types/wlr_seat.h>

#include "zen-common.h"
#include "zen/input-device.h"

static struct zn_input_device*
zn_seat_get_device(struct zn_seat* self, struct zn_input_device* input_device)
{
  struct zn_input_device* dev;
  wl_list_for_each(dev, &self->devices, link)
  {
    if (dev == input_device) {
      return input_device;
    }
  }

  // TODO: keyboard group

  return NULL;
}

void
zn_seat_configure_device(
    struct zn_seat* self, struct zn_input_device* input_device)
{
  UNUSED(self);
  switch (input_device->wlr_input->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
      // TODO: keyboard configure
      break;
    case WLR_INPUT_DEVICE_POINTER:
      // TODO: pointer configure
      break;
    case WLR_INPUT_DEVICE_TOUCH:
    case WLR_INPUT_DEVICE_TABLET_TOOL:
    case WLR_INPUT_DEVICE_TABLET_PAD:
    case WLR_INPUT_DEVICE_SWITCH:
      break;
  }
}

static void
zn_seat_update_capabilities(struct zn_seat* self)
{
  uint32_t caps = 0;

  struct zn_input_device* input_device;
  wl_list_for_each(input_device, &self->devices, link)
  {
    switch (input_device->wlr_input->type) {
      case WLR_INPUT_DEVICE_KEYBOARD:
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
        break;
      case WLR_INPUT_DEVICE_POINTER:
        caps |= WL_SEAT_CAPABILITY_POINTER;
        break;
      case WLR_INPUT_DEVICE_TOUCH:
        // TODO: support touch device
        break;
      case WLR_INPUT_DEVICE_TABLET_TOOL:
      case WLR_INPUT_DEVICE_TABLET_PAD:
      case WLR_INPUT_DEVICE_SWITCH:
        break;
    }
  }

  wlr_seat_set_capabilities(self->wlr_seat, caps);
}

void
zn_seat_add_device(struct zn_seat* self, struct zn_input_device* input_device)
{
  if (zn_seat_get_device(self, input_device)) {
    zn_seat_configure_device(self, input_device);
    return;
  }

  wl_list_insert(&self->devices, &input_device->link);

  zn_seat_configure_device(self, input_device);

  zn_seat_update_capabilities(self);
}

void
zn_seat_remove_device(
    struct zn_seat* self, struct zn_input_device* input_device)
{
  wl_list_remove(&input_device->link);
  zn_seat_update_capabilities(self);
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
