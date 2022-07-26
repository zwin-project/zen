#include <memory>

#include "vr-system.h"
#include "zen-common.h"
#include "zen-immersive-backend.h"

struct zn_openvr_immersive_backend {
  struct zn_immersive_backend base;
  std::unique_ptr<zen::VrSystem> vr_system;  // nonnull
};

bool
zn_immersive_backend_connect(struct zn_immersive_backend* parent)
{
  struct zn_openvr_immersive_backend* self =
      zn_container_of(parent, self, base);

  return self->vr_system->Connect();
}

void
zn_immersive_backend_disconnect(struct zn_immersive_backend* parent)
{
  struct zn_openvr_immersive_backend* self =
      zn_container_of(parent, self, base);

  self->vr_system->Disconnect();
}

struct zn_immersive_backend*
zn_immersive_backend_create()
{
  struct zn_openvr_immersive_backend* self;

  self = static_cast<struct zn_openvr_immersive_backend*>(zalloc(sizeof *self));
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->vr_system = std::make_unique<zen::VrSystem>();

  return &self->base;

err:
  return NULL;
}

void
zn_immersive_backend_destroy(struct zn_immersive_backend* parent)
{
  struct zn_openvr_immersive_backend* self =
      zn_container_of(parent, self, base);

  self->vr_system.reset();
  free(self);
}
