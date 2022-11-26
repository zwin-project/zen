#include "gl-texture.h"

#include "application.h"
#include "buffer.h"

namespace zen::client {

bool
GlTexture::Init()
{
  proxy_ = zgn_gles_v32_create_gl_texture(app_->gles_v32());
  if (proxy_ == nullptr) {
    zn_error("Failed to create gl texture proxy");
    return false;
  }

  return true;
}

void
GlTexture::Image2D(GLenum target, GLint level, GLint internal_format,
    GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
    Buffer* buffer)
{
  zgn_gl_texture_image_2d(proxy_, target, level, internal_format, width, height,
      border, format, type, buffer->proxy());
}

GlTexture::GlTexture(Application* app) : app_(app) {}

GlTexture::~GlTexture()
{
  if (proxy_) {
    zgn_gl_texture_destroy(proxy_);
  }
}

std::unique_ptr<GlTexture>
CreateGlTexture(Application* app)
{
  auto gl_textue = std::make_unique<GlTexture>(app);

  if (!gl_textue->Init()) {
    return std::unique_ptr<GlTexture>();
  }

  return gl_textue;
}

}  // namespace zen::client
