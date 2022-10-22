#include "gl-buffer.h"

#include <zen-common.h>

#include "buffer.h"
#include "remote.h"

void
znr_gl_buffer_gl_buffer_data(struct znr_gl_buffer* parent,
    struct znr_buffer* buffer, size_t size, uint64_t usage)
{
  znr_gl_buffer_impl* self = zn_container_of(parent, self, base);

  znr_buffer_impl* buffer_impl = zn_container_of(buffer, buffer_impl, base);

  self->proxy->GlBufferData(znr_buffer_impl_use(buffer_impl), size, usage);
}

struct znr_gl_buffer*
znr_gl_buffer_create(struct znr_remote* remote)
{
  znr_gl_buffer_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = new znr_gl_buffer_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlBuffer(remote_impl->proxy);
  if (!self->proxy) {
    zn_error("Failed to create a gl buffer");
    goto err_free;
  }

  self->base.id = self->proxy->id();

  return &self->base;

err_free:
  delete self;

err:
  return nullptr;
}

void
znr_gl_buffer_destroy(struct znr_gl_buffer* parent)
{
  znr_gl_buffer_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  delete self;
}
