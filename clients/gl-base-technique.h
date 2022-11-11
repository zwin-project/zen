#pragma once

#include <GLES3/gl32.h>
#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;
class RenderingUnit;
class GlVertexArray;

class GlBaseTechnique
{
 public:
  DISABLE_MOVE_AND_COPY(GlBaseTechnique);
  GlBaseTechnique(Application *app);
  ~GlBaseTechnique();

  bool Init(RenderingUnit *unit);

  void DrawArrays(GLenum mode, GLint first, GLsizei count);

  void Bind(GlVertexArray *vertex_array);

 private:
  Application *app_;
  zgn_gl_base_technique *proxy_ = nullptr;  // nonnull after initialization
};

std::unique_ptr<GlBaseTechnique> CreateGlBaseTechnique(
    Application *app, RenderingUnit *unit);

}  // namespace zen::client
