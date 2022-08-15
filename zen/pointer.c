#include "zen/pointer.h"

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
  struct wlr_event_pointer_motion* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;
  struct wlr_surface* surface;
  struct zn_view* view;
  int view_x, view_y;

  zn_cursor_move_relative(cursor, event->delta_x, event->delta_y);

  view = zn_screen_get_view_at(cursor->screen, cursor->x, cursor->y);
  if (!view) {
    wlr_seat_pointer_notify_clear_focus(seat);
    return;
  }

  surface = view->impl->get_wlr_surface(view);
  if (surface) {
    view_x = cursor->x - view->x;
    view_y = cursor->y - view->y;
    wlr_seat_pointer_notify_enter(seat, surface, view_x, view_y);
    wlr_seat_pointer_notify_motion(seat, event->time_msec, view_x, view_y);
  } else {
    wlr_seat_pointer_notify_clear_focus(seat);
  }
}

static void
zn_pointer_handle_button(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct wlr_event_pointer_button* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;
  struct zn_view* view;

  wlr_seat_pointer_notify_button(
      seat, event->time_msec, event->button, event->state);

  if (event->state == WLR_BUTTON_RELEASED) {
    return;
  }

  view = zn_screen_get_view_at(cursor->screen, cursor->x, cursor->y);
  if (view) {
    zn_view_focus(view);
  }
}

static void
zn_pointer_handle_axis(struct wl_listener* listener, void* data)
{
  struct zn_pointer* self = zn_container_of(listener, self, axis_listener);
  struct wlr_event_pointer_axis* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;
  wlr_seat_pointer_notify_axis(seat, event->time_msec, event->orientation,
      event->delta, event->delta_discrete, event->source);
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

  self->axis_listener.notify = zn_pointer_handle_axis;
  wl_signal_add(&wlr_input_device->pointer->events.axis, &self->axis_listener);

  self->button_listener.notify = zn_pointer_handle_button;
  wl_signal_add(
      &wlr_input_device->pointer->events.button, &self->button_listener);

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
  free(self);
}
