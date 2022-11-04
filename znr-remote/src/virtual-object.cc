#include "virtual-object.h"

#include <zen-common.h>

#include "session.h"

znr_virtual_object*
znr_virtual_object_create(znr_session* session_base)
{
  auto self = new znr_virtual_object();
  znr_session_impl* session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateVirtualObject(session->proxy);

  return self;

err:
  return nullptr;
}

void
znr_virtual_object_destroy(znr_virtual_object* self)
{
  delete self;
}
