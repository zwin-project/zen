#include "cursor.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

void
zn_cursor_impl_notify_motion(
    struct zn_cursor_impl *self, struct zn_cursor_motion_event *event)
{
  wl_signal_emit(&self->base.events.motion, event);
}

struct zn_cursor_impl *
zn_cursor_impl_get(struct zn_cursor *base)
{
  struct zn_cursor_impl *self = zn_container_of(base, self, base);
  return self;
}

struct zn_cursor_impl *
zn_cursor_impl_create(void)
{
  struct zn_cursor_impl *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.motion);

  return self;

err:
  return NULL;
}

void
zn_cursor_impl_destroy(struct zn_cursor_impl *self)
{
  wl_list_remove(&self->base.events.motion.listener_list);
  free(self);
}
