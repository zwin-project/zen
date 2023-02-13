#include "zen/screen.h"

#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

void
zn_screen_notify_frame(
    struct zn_screen *self, struct zn_screen_frame_event *event)
{
  wl_signal_emit(&self->events.frame, event);
}

struct zn_screen *
zn_screen_create(void *impl)
{
  struct zn_screen *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl = impl;
  wl_signal_init(&self->events.frame);
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->events.frame.listener_list);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
