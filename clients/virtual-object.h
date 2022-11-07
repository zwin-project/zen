#pragma once

#include <zigen-client-protocol.h>

#include <memory>

#include "common.h"

namespace zen::client {

class Application;

class VirtualObject
{
 public:
  DISABLE_MOVE_AND_COPY(VirtualObject);
  VirtualObject(Application *app);
  ~VirtualObject();

  bool Init();

  void Commit();

  inline zgn_virtual_object *proxy();

 private:
  Application *app_;
  zgn_virtual_object *proxy_ = nullptr;  // nonnull after initialization
};

inline zgn_virtual_object *
VirtualObject::proxy()
{
  return proxy_;
}

std::unique_ptr<VirtualObject> CreateVirtualObject(Application *app);

}  // namespace zen::client
