#include "gl-buffer.h"

#include <zen-common.h>

#include "remote.h"

struct znr_gl_buffer*
znr_gl_buffer_create(struct znr_remote* remote)
{
  znr_gl_buffer_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = static_cast<znr_gl_buffer_impl*>(zalloc(sizeof *self));
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
  free(self);

err:
  return nullptr;
}

void
znr_gl_buffer_destroy(struct znr_gl_buffer* parent)
{
  znr_gl_buffer_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  free(self);
}
