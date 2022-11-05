#include "gl-base-technique.h"

#include "application.h"
#include "rendering-unit.h"

namespace zen::client {

bool
GlBaseTechnique::Init(RenderingUnit *unit)
{
  proxy_ =
      zgn_gles_v32_create_gl_base_technique(app_->gles_v32(), unit->proxy());
  if (proxy_ == nullptr) {
    LOG_ERROR("Failed to create gl base technique object proxy");
    return false;
  }

  return true;
}

GlBaseTechnique::GlBaseTechnique(Application *app) : app_(app) {}

GlBaseTechnique::~GlBaseTechnique()
{
  if (proxy_) {
    zgn_gl_base_technique_destroy(proxy_);
  }
}

std::unique_ptr<GlBaseTechnique>
CreateGlBaseTechnique(Application *app, RenderingUnit *unit)
{
  auto technique = std::make_unique<GlBaseTechnique>(app);

  if (!technique->Init(unit)) {
    return std::unique_ptr<GlBaseTechnique>();
  }

  return technique;
}

}  // namespace zen::client
