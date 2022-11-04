#include "rendering-unit.h"

#include "application.h"
#include "virtual-object.h"

namespace zen::client {

bool
RenderingUnit::Init(VirtualObject* virtual_object)
{
  proxy_ = zgn_gles_v32_create_rendering_unit(
      app_->gles_v32(), virtual_object->proxy());
  if (proxy_ == nullptr) {
    LOG_ERROR("Failed to create rendering unit proxy");
    return false;
  }

  return true;
}

RenderingUnit::RenderingUnit(Application* app) : app_(app) {}

RenderingUnit::~RenderingUnit()
{
  if (proxy_) {
    zgn_rendering_unit_destroy(proxy_);
  }
}

std::unique_ptr<RenderingUnit>
CreateRenderingUnit(Application* app, VirtualObject* virtual_object)
{
  auto rendering_unit = std::make_unique<RenderingUnit>(app);

  if (!rendering_unit->Init(virtual_object)) {
    return std::unique_ptr<RenderingUnit>();
  }

  return rendering_unit;
}

}  // namespace zen::client
