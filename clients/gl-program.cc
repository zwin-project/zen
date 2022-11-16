#include "gl-program.h"

#include "application.h"

namespace zen::client {

bool
GlProgram::Init()
{
  proxy_ = zgn_gles_v32_create_gl_program(app_->gles_v32());
  if (proxy_ == nullptr) {
    zn_error("Failed to creat gl program proxy");
    return false;
  }

  return true;
}

GlProgram::GlProgram(Application *app) : app_(app) {}

GlProgram::~GlProgram()
{
  if (proxy_) {
    zgn_gl_program_destroy(proxy_);
  }
}

std::unique_ptr<GlProgram>
CreateGlProgram(Application *app)
{
  auto gl_program = std::make_unique<GlProgram>(app);

  if (!gl_program->Init()) {
    return std::unique_ptr<GlProgram>();
  }

  return gl_program;
}

}  // namespace zen::client
