#include "zen/screen.h"

#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

void
zn_screen_damage(struct zn_screen *self, struct wlr_fbox *fbox)
{
  self->impl->damage(self->impl_data, fbox);
}

void
zn_screen_notify_frame(struct zn_screen *self, struct timespec *when)
{
  wl_signal_emit(&self->events.frame, when);
}

struct zn_screen *
zn_screen_create(
    void *impl_data, const struct zn_screen_interface *implementation)
{
  struct zn_screen *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode_root = zn_snode_create_root(self);
  if (self->snode_root == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  wl_signal_init(&self->events.frame);
  wl_signal_init(&self->events.destroy);

  return self;

err_free:
  free(self);

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
