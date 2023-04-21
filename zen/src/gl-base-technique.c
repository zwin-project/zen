#include "gl-base-technique-private.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"

struct zn_gl_base_technique *
zn_gl_base_technique_create(void *impl_data)
{
  struct zn_gl_base_technique *self = zalloc(sizeof *self);
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
zn_gl_base_technique_destroy(struct zn_gl_base_technique *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);

  free(self);
}
