#include <wayland-server-core.h>
#include <zen/display-system/remote/core/logger.h>
#include <zen/display-system/remote/server.h>

#include <memory>

#include "log-sink.h"
#include "loop.h"
#include "zen-common.h"
#include "zen/backend/immersive.h"

using namespace zen;

struct zn_remote_immersive_backend {
  struct zn_immersive_backend base;
  std::unique_ptr<display_system::remote::server::IRemote> remote;  // nonnull
};

void
zn_immersive_backend_start_repaint_loop(struct zn_immersive_backend* parent)
{
  UNUSED(parent);
}

bool
zn_immersive_backend_connect(struct zn_immersive_backend* parent)
{
  UNUSED(parent);
  return true;
}

void
zn_immersive_backend_disconnect(struct zn_immersive_backend* parent)
{
  UNUSED(parent);
}

struct zn_immersive_backend*
zn_immersive_backend_create(struct wl_event_loop* loop)
{
  struct zn_remote_immersive_backend* self;
  UNUSED(loop);

  display_system::remote::log::InitializeLogger(
      std::make_unique<backend::remote::RemoteLogSink>());

  self = static_cast<struct zn_remote_immersive_backend*>(zalloc(sizeof *self));
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  {
    auto remote_loop = std::make_unique<backend::remote::RemoteLoop>(loop);
    self->remote =
        display_system::remote::server::CreateRemote(std::move(remote_loop));
  }

  self->remote->Start();

  wl_signal_init(&self->base.events.disconnected);

  return &self->base;

err:
  return NULL;
}

void
zn_immersive_backend_destroy(struct zn_immersive_backend* parent)
{
  struct zn_remote_immersive_backend* self =
      zn_container_of(parent, self, base);

  wl_list_remove(&self->base.events.disconnected.listener_list);
  self->remote.reset();

  free(self);
}
