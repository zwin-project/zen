#pragma once

#include <zigen-client-protocol.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

#include "common.h"

namespace zen::client {

class Application;
class VirtualObject;

class RenderingUnit
{
 public:
  DISABLE_MOVE_AND_COPY(RenderingUnit);
  RenderingUnit(Application* app);
  ~RenderingUnit();

  bool Init(VirtualObject* virtual_object);

 private:
  Application* app_;
  zgn_rendering_unit* proxy_ = nullptr;  // nonnull after initialization
};

std::unique_ptr<RenderingUnit> CreateRenderingUnit(
    Application* app, VirtualObject* virtual_object);

}  // namespace zen::client
