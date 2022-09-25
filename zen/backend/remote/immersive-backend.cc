#include <wayland-server-core.h>
#include <zen/display-system/remote/core/logger.h>
#include <zen/display-system/remote/server.h>

#include <memory>

#include "log-sink.h"
#include "zen-common.h"
#include "zen/backend/immersive.h"

namespace remote = zen::display_system::remote;

struct zn_remote_immersive_backend {
  struct zn_immersive_backend base;
  std::unique_ptr<remote::server::ISession> session;  // nonnull
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
  std::unique_ptr<remote::server::ISession> session;
  UNUSED(loop);

  zen::display_system::remote::log::InitializeLogger(
      std::make_unique<zen::backend::remote::RemoteLogSink>());

  self = static_cast<struct zn_remote_immersive_backend*>(zalloc(sizeof *self));
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = remote::server::SessionCreate();
  if (!session->Init()) {
    zn_error("Failed to allocate memory");
    goto err_free;
  }

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
  struct zn_remote_immersive_backend* self =
      zn_container_of(parent, self, base);

  wl_list_remove(&self->base.events.disconnected.listener_list);
  self->session.reset();

  free(self);
}
