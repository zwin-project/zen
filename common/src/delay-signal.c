#include "zen-common/delay-signal.h"

#include "zen-common/signal.h"
#include "zen-common/util.h"

static void
zn_delay_signal_idle_timer(void *data)
{
  struct zn_delay_signal *self = data;

  self->idle = NULL;

  zn_signal_emit_mutable(&self->signal, NULL);
}

void
zn_delay_signal_schedule(
    struct zn_delay_signal *self, struct wl_display *display)
{
  if (self->idle) {
    return;
  }

  struct wl_event_loop *loop = wl_display_get_event_loop(display);

  self->idle = wl_event_loop_add_idle(loop, zn_delay_signal_idle_timer, self);
}

void
zn_delay_signal_add(struct zn_delay_signal *self, struct wl_listener *listener)
{
  wl_signal_add(&self->signal, listener);
}

void
zn_delay_signal_init(struct zn_delay_signal *self)
{
  wl_signal_init(&self->signal);
  self->idle = NULL;
}

void
zn_delay_signal_release(struct zn_delay_signal *self)
{
  wl_list_remove(&self->signal.listener_list);
  if (self->idle) {
    wl_event_source_remove(self->idle);
  }
}
