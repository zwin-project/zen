#include "zen/screen.h"

#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

static struct wlr_texture *
zn_screen_snode_get_texture(void *user_data UNUSED)
{
  return NULL;
}

static const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_screen_snode_get_texture,
};

void
zn_screen_notify_frame(struct zn_screen *self, struct timespec *when)
{
  wl_signal_emit(&self->events.frame, when);
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
  self->snode_root = zn_snode_create(self, &snode_implementation);
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

  zn_snode_destroy(self->snode_root);
  wl_list_remove(&self->events.frame.listener_list);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
