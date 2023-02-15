#include "zen/backend/default/seat.h"

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend/default/input-device.h"
#include "zen/backend/default/pointer.h"

void
zn_default_backend_seat_notify_motion(struct zn_default_backend_seat *self,
    struct zn_seat_pointer_motion_event *event)
{
  wl_signal_emit(&self->base.events.pointer_motion, event);
}

void
zn_default_backend_seat_update_capabilities(
    struct zn_default_backend_seat *self)
{
  uint32_t wl_capabilities = 0;
  struct zn_input_device_base *device = NULL;

  wl_list_for_each (device, &self->device_list, link) {
    switch (device->wlr_input_device->type) {
      case WLR_INPUT_DEVICE_KEYBOARD:
        break;
      case WLR_INPUT_DEVICE_POINTER:
        wl_capabilities |= WL_SEAT_CAPABILITY_POINTER;
        break;
      case WLR_INPUT_DEVICE_TOUCH:        // fall through
      case WLR_INPUT_DEVICE_TABLET_TOOL:  // fall through
      case WLR_INPUT_DEVICE_TABLET_PAD:   // fall through
      case WLR_INPUT_DEVICE_SWITCH:
        break;
    }
  }

  wlr_seat_set_capabilities(self->wlr_seat, wl_capabilities);
  // TODO(@Aki-7): set capabilities for zwin
}

void
zn_default_backend_seat_handle_new_input(
    struct zn_default_backend_seat *self, struct wlr_input_device *device)
{
  switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
      break;
    case WLR_INPUT_DEVICE_POINTER: {
      struct zn_pointer *pointer = zn_pointer_create(self, device);
      wl_list_insert(&self->device_list, &pointer->base.link);
      break;
    }
    case WLR_INPUT_DEVICE_TOUCH:        // fall through
    case WLR_INPUT_DEVICE_TABLET_TOOL:  // fall through
    case WLR_INPUT_DEVICE_TABLET_PAD:   // fall through
    case WLR_INPUT_DEVICE_SWITCH:
      break;
  }

  zn_default_backend_seat_update_capabilities(self);
}

struct zn_default_backend_seat *
zn_default_backend_seat_get(struct zn_seat *base)
{
  struct zn_default_backend_seat *self = zn_container_of(base, self, base);
  return self;
}

struct zn_default_backend_seat *
zn_default_backend_seat_create(
    struct wl_display *display, const char *seat_name)
{
  struct zn_default_backend_seat *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_seat = wlr_seat_create(display, seat_name);
  if (self->wlr_seat == NULL) {
    zn_error("Failed to create a wlr_seat");
    goto err_free;
  }

  wl_signal_init(&self->base.events.pointer_motion);
  wl_list_init(&self->device_list);
  wl_signal_init(&self->events.destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_default_backend_seat_destroy(struct zn_default_backend_seat *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->device_list);
  wl_list_remove(&self->base.events.pointer_motion.listener_list);
  wlr_seat_destroy(self->wlr_seat);
  free(self);
}
