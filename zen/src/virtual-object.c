#include "zen/virtual-object.h"

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"

struct zn_virtual_object *
zn_virtual_object_create(void *impl_data)
{
  struct zn_virtual_object *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl_data = impl_data;
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_virtual_object_destroy(struct zn_virtual_object *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}