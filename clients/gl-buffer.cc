#include "gl-buffer.h"

#include "application.h"
#include "buffer.h"

namespace zen::client {

bool
GlBuffer::Init()
{
  proxy_ = zgn_gles_v32_create_gl_buffer(app_->gles_v32());
  if (proxy_ == nullptr) {
    zn_error("Failed to create gl buffer proxy");
    return false;
  }

  return true;
}

void
GlBuffer::Data(GLenum target, Buffer *buffer, GLenum usage)
{
  zgn_gl_buffer_data(proxy_, target, buffer->proxy(), usage);
}

GlBuffer::GlBuffer(Application *app) : app_(app) {}

GlBuffer::~GlBuffer()
{
  if (proxy_) {
    zgn_gl_buffer_destroy(proxy_);
  }
}

std::unique_ptr<GlBuffer>
CreateGlBuffer(Application *app)
{
  auto gl_buffer = std::make_unique<GlBuffer>(app);

  if (!gl_buffer->Init()) {
    return std::unique_ptr<GlBuffer>();
  }

  return gl_buffer;
}

}  // namespace zen::client
