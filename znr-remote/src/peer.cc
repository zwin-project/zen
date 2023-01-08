#include "peer.h"

#include <string.h>
#include <zen-common.h>

znr_remote_peer_impl *
znr_remote_peer_create(std::shared_ptr<zen::remote::server::IPeer> proxy)
{
  auto self = new znr_remote_peer_impl();

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.destroy);
  self->base.host = strdup(proxy->host().c_str());
  self->base.wired = proxy->wired();
  self->proxy = proxy;

  return self;

err:
  return nullptr;
}

void
znr_remote_peer_destroy(znr_remote_peer_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, nullptr);

  free(self->base.host);
  wl_list_remove(&self->base.events.destroy.listener_list);
  delete self;
}
