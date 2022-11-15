#include "gl-shader.h"

#include <GLES3/gl32.h>

#include <cstring>

#include "application.h"
#include "buffer.h"
#include "fd.h"
#include "shm-pool.h"

namespace zen::client {

bool
GlShader::Init(std::string source)
{
  fd_ = create_anonymous_file(source.size());
  if (fd_ < 0) {
    zn_error("Failed to create anonymous file");
    return false;
  }

  {
    auto buffer = mmap(nullptr, source.size(), PROT_WRITE, MAP_SHARED, fd_, 0);
    std::memcpy(buffer, source.data(), source.size());
    munmap(buffer, source.size());
  }

  pool_ = CreateShmPool(app_, fd_, source.size());
  buffer_ = CreateBuffer(pool_.get(), 0, source.size());

  proxy_ = zgn_gles_v32_create_gl_shader(
      app_->gles_v32(), buffer_->proxy(), GL_VERTEX_SHADER);

  if (proxy_ == nullptr) {
    zn_error("Failed to create gl shader proxy");
    return false;
  }

  return true;
}

GlShader::GlShader(Application *app) : app_(app) {}

GlShader::~GlShader()
{
  if (proxy_) {
    zgn_gl_shader_destroy(proxy_);
  }

  if (fd_ > 0) {
    close(fd_);
  }
}

std::unique_ptr<GlShader>
CreateGlShader(Application *app, std::string source)
{
  auto gl_shader = std::make_unique<GlShader>(app);

  if (!gl_shader->Init(std::move(source))) {
    return std::unique_ptr<GlShader>();
  }

  return gl_shader;
}

}  // namespace zen::client
