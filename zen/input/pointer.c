#include "zen/input/pointer.h"

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_surface.h>

#include "zen-common.h"
#include "zen/cursor.h"
#include "zen/scene/view.h"
#include "zen/server.h"

static void
zn_pointer_handle_motion(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;
  struct wlr_event_pointer_motion* event = data;

  if (server->display_system == ZEN_DISPLAY_SYSTEM_TYPE_SCREEN) {
    cursor->grab->interface->motion(cursor->grab, event);
  } else {
    // TODO: ray
  }
}

static void
zn_pointer_handle_button(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;
  struct wlr_event_pointer_button* event = data;

  if (server->display_system == ZEN_DISPLAY_SYSTEM_TYPE_SCREEN) {
    cursor->grab->interface->button(cursor->grab, event);
  } else {
    // TODO: ray
  }
}

static void
zn_pointer_handle_axis(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;
  struct wlr_event_pointer_axis* event = data;

  if (server->display_system == ZEN_DISPLAY_SYSTEM_TYPE_SCREEN) {
    cursor->grab->interface->axis(cursor->grab, event);
  } else {
    // TODO: ray
  }
}

static void
zn_pointer_handle_frame(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  UNUSED(data);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;

  if (server->display_system == ZEN_DISPLAY_SYSTEM_TYPE_SCREEN) {
    cursor->grab->interface->frame(cursor->grab);
  } else {
    // TODO: ray
  }
}

struct zn_pointer*
zn_pointer_create(struct wlr_input_device* wlr_input_device)
{
  struct zn_pointer* self;

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
zn_pointer_destroy(struct zn_pointer* self)
{
  wl_list_remove(&self->motion_listener.link);
  wl_list_remove(&self->button_listener.link);
  wl_list_remove(&self->axis_listener.link);
  wl_list_remove(&self->frame_listener.link);
  free(self);
}
