#include "zen/view.h"

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

void
zn_view_notify_unmap(struct zn_view *self)
{
  zn_snode_set_position(self->snode, NULL, (vec2){0, 0});

  wl_signal_emit(&self->events.unmap, NULL);
}

struct zn_view *
zn_view_create(void *impl_data, const struct zn_view_interface *implementation)
{
  struct zn_view *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->impl_data = impl_data;
  self->impl = implementation;

  wl_signal_init(&self->events.unmap);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_view_destroy(struct zn_view *self)
{
  wl_list_remove(&self->events.unmap.listener_list);
  zn_snode_destroy(self->snode);
  free(self);
}
