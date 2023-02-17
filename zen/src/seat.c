#include "seat.h"

#include "cursor.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

void
zn_seat_notify_motion(
    struct zn_seat *self, struct wlr_event_pointer_motion *event)
{
  wl_signal_emit(&self->events.pointer_motion, event);
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

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat *self)
{
  wl_list_remove(&self->events.pointer_motion.listener_list);
  zn_cursor_destroy(self->cursor);
  free(self);
}
