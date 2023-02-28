#include "seat.h"

#include "cursor.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

void
zn_seat_notify_pointer_motion(
    struct zn_seat *self, struct wlr_event_pointer_motion *event)
{
  wl_signal_emit(&self->events.pointer_motion, event);
}

void
zn_seat_notify_pointer_button(
    struct zn_seat *self, struct wlr_event_pointer_button *event)
{
  wl_signal_emit(&self->events.pointer_button, event);
}

void
zn_seat_notify_pointer_axis(
    struct zn_seat *self, struct wlr_event_pointer_axis *event)
{
  wl_signal_emit(&self->events.pointer_axis, event);
}

void
zn_seat_notify_pointer_frame(struct zn_seat *self)
{
  wl_signal_emit(&self->events.pointer_frame, NULL);
}

struct zn_seat *
zn_seat_create(void)
{
  struct zn_seat *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->cursor = zn_cursor_create();
  if (self->cursor == NULL) {
    zn_error("Failed to create a zn_cursor");
    goto err_free;
  }

  wl_signal_init(&self->events.pointer_motion);
  wl_signal_init(&self->events.pointer_button);
  wl_signal_init(&self->events.pointer_axis);
  wl_signal_init(&self->events.pointer_frame);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat *self)
{
  wl_list_remove(&self->events.pointer_frame.listener_list);
  wl_list_remove(&self->events.pointer_axis.listener_list);
  wl_list_remove(&self->events.pointer_button.listener_list);
  wl_list_remove(&self->events.pointer_motion.listener_list);
  zn_cursor_destroy(self->cursor);
  free(self);
}
