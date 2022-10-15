#include "rendering-unit.h"

#include <zen-common.h>
#include <zen-remote/server/rendering-unit.h>

#include <memory>

#include "remote.h"

struct znr_rendering_unit*
znr_rendering_unit_create(struct znr_remote* remote)
{
  znr_rendering_unit_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = static_cast<znr_rendering_unit_impl*>(zalloc(sizeof *self));
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateRenderingUnit(remote_impl->proxy);
  if (!self->proxy) {
    zn_error("Failed to create a rendering unit");
    goto err_free;
  }

  return &self->base;

err_free:
  free(self);

err:
  return nullptr;
}

void
znr_rendering_unit_destroy(struct znr_rendering_unit* parent)
{
  znr_rendering_unit_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  free(self);
}
