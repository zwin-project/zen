#include "zen/buffer.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_buffer *
zn_buffer_create(
    void *impl_data, const struct zn_buffer_interface *implementation)
{
  struct zn_buffer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl_data = impl_data;
  self->impl = implementation;

  return self;

err:
  return NULL;
}

void
zn_buffer_destroy(struct zn_buffer *self)
{
  free(self);
}
