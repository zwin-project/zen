#include "virtual-object.h"

#include <zen-common.h>

#include "remote.h"

void
znr_virtual_object_commit(struct znr_virtual_object* parent)
{
  znr_virtual_object_impl* self = zn_container_of(parent, self, base);
  self->proxy->Commit();
}

struct znr_virtual_object*
znr_virtual_object_create(struct znr_remote* remote)
{
  znr_virtual_object_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = new znr_virtual_object_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateVirtualObject(remote_impl->proxy);
  if (!self->proxy) {
    zn_error("Failed to create a virtual object");
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
znr_virtual_object_destroy(struct znr_virtual_object* parent)
{
  znr_virtual_object_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  delete self;
}
