#pragma once

#include <zigen-gles-v32-client-protocol.h>

#include <memory>

#include "common.h"

namespace zen::client {

class Application;
class RenderingUnit;

class GlBaseTechnique
{
 public:
  DISABLE_MOVE_AND_COPY(GlBaseTechnique);
  GlBaseTechnique(Application *app);
  ~GlBaseTechnique();

  bool Init(RenderingUnit *unit);

 private:
  Application *app_;
  zgn_gl_base_technique *proxy_ = nullptr;  // nonnull after initialization
};

std::unique_ptr<GlBaseTechnique> CreateGlBaseTechnique(
    Application *app, RenderingUnit *unit);

}  // namespace zen::client
