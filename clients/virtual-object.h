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

 private:
  Application *app_;
  zgn_virtual_object *proxy = nullptr;
};

std::unique_ptr<VirtualObject> CreateVirtualObject(Application *app);

}  // namespace zen::client
