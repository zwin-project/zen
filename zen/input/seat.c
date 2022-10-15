#include "zen/input/seat.h"

#include <wlr/types/wlr_seat.h>

#include "zen-common.h"
#include "zen/input/input-device.h"

static void
zn_seat_handle_request_set_cursor(struct wl_listener* listener, void* data)
{
  struct zn_seat* self =
      zn_container_of(listener, self, request_set_cursor_listener);
  struct wlr_seat_pointer_request_set_cursor_event* event = data;
  struct wlr_surface* focused_surface =
      self->wlr_seat->pointer_state.focused_surface;
  struct wl_client* focused_client = NULL;
  if (focused_surface != NULL) {
    focused_client = wl_resource_get_client(focused_surface->resource);
  }

  if (event->seat_client->client == focused_client) {
    zn_cursor_set_surface(
        self->cursor, event->surface, event->hotspot_x, event->hotspot_y);
  }
}

static void
zn_seat_update_capabilities(struct zn_seat* self)
{
  uint32_t caps = 0;
  struct zn_server* server = zn_server_get_singleton();

  struct zn_input_device* input_device;
  wl_list_for_each (input_device, &self->devices, link) {
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

  if (caps & WL_SEAT_CAPABILITY_POINTER) {
    zn_scene_ensure_ray(server->scene);
  } else {
    zn_scene_destroy_ray(server->scene);
  }

  wlr_seat_set_capabilities(self->wlr_seat, caps);
}

void
zn_seat_add_device(struct zn_seat* self, struct zn_input_device* input_device)
{
  wl_list_insert(&self->devices, &input_device->link);
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

  self->cursor = zn_cursor_create();
  if (self->cursor == NULL) {
    zn_error("Failed to create zn_cursor");
    goto err_wlr_seat;
  }

  wl_list_init(&self->devices);
  wl_signal_init(&self->events.destroy);

  self->request_set_cursor_listener.notify = zn_seat_handle_request_set_cursor;
  wl_signal_add(&self->wlr_seat->events.request_set_cursor,
      &self->request_set_cursor_listener);

  return self;

err_wlr_seat:
  wlr_seat_destroy(self->wlr_seat);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat* self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->request_set_cursor_listener.link);
  wl_list_remove(&self->devices);
  zn_cursor_destroy(self->cursor);
  wlr_seat_destroy(self->wlr_seat);
  free(self);
}
