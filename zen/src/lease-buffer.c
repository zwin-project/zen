#include "zen/lease-buffer.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_lease_buffer *
zn_lease_buffer_create(struct zn_buffer *buffer,
    zn_lease_buffer_release_callback callback, void *user_data)
{
  struct zn_lease_buffer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->user_data = user_data;
  self->buffer = buffer;
  self->callback = callback;

  return self;

err:
  return NULL;
}

void
zn_lease_buffer_release(struct zn_lease_buffer *self)
{
  self->callback(self->buffer, self->user_data);

  free(self);
}
