#include "virtual-object.h"

#include "application.h"

namespace zen::client {

bool
VirtualObject::Init()
{
  proxy = zgn_compositor_create_virtual_object(app_->compositor());
  if (proxy == nullptr) {
    LOG_ERROR("Failed to create virtual object proxy");
    return false;
  }

  return true;
}

VirtualObject::VirtualObject(Application *app) : app_(app) {}

VirtualObject::~VirtualObject()
{
  if (proxy) {
    zgn_virtual_object_destroy(proxy);
  }
}

std::unique_ptr<VirtualObject>
CreateVirtualObject(Application *app)
{
  auto virtual_object = std::make_unique<VirtualObject>(app);
  virtual_object->Init();
  return virtual_object;
}

}  // namespace zen::client
