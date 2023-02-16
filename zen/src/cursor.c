#include "cursor.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

void
zn_cursor_notify_motion(
    struct zn_cursor *self, struct zn_cursor_motion_event *event)
{
  wl_signal_emit(&self->events.motion, event);
}

struct zn_cursor *
zn_cursor_create(void)
{
  struct zn_cursor *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->events.motion);

  return self;

err:
  return NULL;
}

void
zn_cursor_destroy(struct zn_cursor *self)
{
  wl_list_remove(&self->events.motion.listener_list);
  free(self);
}
