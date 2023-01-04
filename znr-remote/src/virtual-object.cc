#include "virtual-object.h"

#include <zen-common.h>

#include "dispatcher.h"

void
znr_virtual_object_move(
    struct znr_virtual_object *self, vec3 position, versor quaternion)
{
  self->proxy->Move(position, quaternion);
}

void
znr_virtual_object_commit(struct znr_virtual_object *self)
{
  self->proxy->Commit();
}

znr_virtual_object *
znr_virtual_object_create(znr_dispatcher *dispatcher_base)
{
  auto self = new znr_virtual_object();
  znr_dispatcher_impl *dispatcher =
      zn_container_of(dispatcher_base, dispatcher, base);

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateVirtualObject(dispatcher->channel);
  if (!self->proxy) {
    zn_error("Failed to create remote virtual object");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_virtual_object_destroy(znr_virtual_object *self)
{
  delete self;
}
