#include "rendering-unit.h"

#include <zen-common.h>

#include "session.h"
#include "virtual-object.h"

struct znr_rendering_unit*
znr_rendering_unit_create(
    struct znr_session* session_base, struct znr_virtual_object* virtual_object)
{
  auto self = new znr_rendering_unit();
  znr_session_impl* session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateRenderingUnit(
      session->proxy, virtual_object->proxy->id());
  if (!self->proxy) {
    zn_error("Failed to create remote rendering unit");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_rendering_unit_destroy(struct znr_rendering_unit* self)
{
  delete self;
}
