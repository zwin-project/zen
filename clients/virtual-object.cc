#include "virtual-object.h"

#include "application.h"

namespace zen::client {

bool
VirtualObject::Init()
{
  proxy_ = zgn_compositor_create_virtual_object(app_->compositor());
  if (proxy_ == nullptr) {
    zn_error("Failed to create virtual object proxy");
    return false;
  }

  wl_proxy_set_user_data((wl_proxy *)proxy_, this);

  return true;
}

void
VirtualObject::Commit()
{
  zgn_virtual_object_commit(proxy_);
}

const struct wl_callback_listener VirtualObject::callback_listener_ = {
    VirtualObject::Callback,
};  // namespace zen::client

void
VirtualObject::NextFrame(void)
{
  if (callback_) return;

  callback_ = zgn_virtual_object_frame(proxy_);
  wl_callback_add_listener(callback_, &VirtualObject::callback_listener_, this);
}

void
VirtualObject::Callback(
    void *data, struct wl_callback * /*wl_callback*/, uint32_t callback_data)
{
  auto self = static_cast<VirtualObject *>(data);

  wl_callback_destroy(self->callback_);
  self->callback_ = nullptr;

  self->Frame(callback_data);
}

VirtualObject::VirtualObject(Application *app) : app_(app) {}

VirtualObject::~VirtualObject()
{
  if (proxy_) {
    zgn_virtual_object_destroy(proxy_);
  }
}

}  // namespace zen::client
