#include "buffer.h"

#include <zen-common.h>

#include "remote.h"

std::unique_ptr<zen::remote::server::IBuffer>
znr_buffer_impl_use(struct znr_buffer_impl* self)
{
  return zen::remote::server::CreateBuffer(
      self->data,
      [self]() { wl_signal_emit(&self->base.events.release, nullptr); },
      self->remote->proxy);
}

struct znr_buffer*
znr_buffer_create(void* data, struct znr_remote* remote)
{
  znr_buffer_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = new znr_buffer_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.release);
  self->remote = remote_impl;
  self->data = data;

  return &self->base;

err:
  return nullptr;
}

void
znr_buffer_destroy(struct znr_buffer* parent)
{
  znr_buffer_impl* self = zn_container_of(parent, self, base);
  delete self;
}
