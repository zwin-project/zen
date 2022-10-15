#include "znr/remote.h"

#include <zen-common.h>
#include <zen-remote/server/remote.h>

#include <cstdlib>
#include <memory>

#include "loop.h"
#include "remote.h"

void
znr_remote_start(struct znr_remote* parent)
{
  znr_remote_impl* self = zn_container_of(parent, self, base);
  self->proxy->Start();
}

void
znr_remote_stop(struct znr_remote* parent)
{
  znr_remote_impl* self = zn_container_of(parent, self, base);
  self->proxy->Stop();
}

znr_remote*
znr_remote_create(struct wl_display* display)
{
  znr_remote_impl* self;
  wl_event_loop* loop = wl_display_get_event_loop(display);

  self = static_cast<znr_remote_impl*>(zalloc(sizeof *self));
  if (self == nullptr) {
    goto err;
  }

  self->proxy = zen::remote::server::CreateRemote(std::make_unique<Loop>(loop));

  return &self->base;

err:
  return nullptr;
}

void
znr_remote_destroy(struct znr_remote* parent)
{
  znr_remote_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  free(self);
}
