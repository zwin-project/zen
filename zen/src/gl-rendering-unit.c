#include "gl-rendering-unit.h"

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"

struct zn_gl_rendering_unit *
zn_gl_rendering_unit_create(void *impl_data,
    const struct zn_gl_rendering_unit_interface *implementation)
{
  struct zn_gl_rendering_unit *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_gl_rendering_unit_destroy(struct zn_gl_rendering_unit *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);

  free(self);
}
