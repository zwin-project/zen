#include "session.h"

#include <zen-common.h>

static void
znr_session_handle_disconnect(znr_session_impl* self)
{
  wl_signal_emit(&self->base.events.disconnected, NULL);
}

znr_session_impl*
znr_session_create(std::unique_ptr<zen::remote::server::ISession> proxy)
{
  auto self = new znr_session_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = std::move(proxy);
  wl_signal_init(&self->base.events.disconnected);
  self->disconnect_signal_connection = self->proxy->on_disconnect.Connect(
      std::bind(znr_session_handle_disconnect, self));

  return self;

err:
  return nullptr;
}

void
znr_session_destroy(znr_session* parent)
{
  struct znr_session_impl* self = zn_container_of(parent, self, base);

  self->disconnect_signal_connection->Disconnect();

  wl_list_remove(&self->base.events.disconnected.listener_list);
  delete self;
}
