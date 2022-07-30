#include <wayland-server-core.h>

#include <memory>

#include "vr-system.h"
#include "zen-common.h"
#include "zen-immersive-backend.h"

struct zn_openvr_immersive_backend {
  struct zn_immersive_backend base;
  std::unique_ptr<zen::VrSystem> vr_system;  // nonnull
};

static void
zn_openvr_immersive_backend_vr_system_disconnected_handler(
    struct zn_openvr_immersive_backend* self)
{
  wl_signal_emit(&self->base.events.disconnected, NULL);
}

void
zn_immersive_backend_start_repaint_loop(struct zn_immersive_backend* parent)
{
  struct zn_openvr_immersive_backend* self =
      zn_container_of(parent, self, base);

  self->vr_system->StartRepaintLoop();
}

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
zn_immersive_backend_create(struct wl_event_loop* loop)
{
  struct zn_openvr_immersive_backend* self;

  self = static_cast<struct zn_openvr_immersive_backend*>(zalloc(sizeof *self));
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->vr_system = std::make_unique<zen::VrSystem>();
  if (self->vr_system->Init(loop) == false) {
    zn_error("Failed to init vr system");
    goto err_free;
  }

  self->vr_system->callbacks.Disconnected = std::bind(
      zn_openvr_immersive_backend_vr_system_disconnected_handler, self);

  wl_signal_init(&self->base.events.disconnected);

  return &self->base;

err_free:
  free(self);

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
