#include "session.h"

#include <zen-common.h>

znr_session_impl*
znr_session_create(std::unique_ptr<zen::remote::server::ISession> proxy)
{
  auto self = new znr_session_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = std::move(proxy);
  wl_signal_init(&self->base.events.destroy);

  return self;

err:
  return nullptr;
}

void
znr_session_destroy(znr_session_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->base.events.destroy.listener_list);
  delete self;
}
