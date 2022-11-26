#pragma once

#include <GLES3/gl32.h>
#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;
class Buffer;

class GlTexture
{
 public:
  DISABLE_MOVE_AND_COPY(GlTexture);
  GlTexture() = delete;
  GlTexture(Application* app);
  ~GlTexture();

  bool Init();

  void Image2D(GLenum target, GLint level, GLint internal_format, GLsizei width,
      GLsizei height, GLint border, GLenum format, GLenum type, Buffer* buffer);

  inline zgn_gl_texture* proxy();

 private:
  Application* app_;
  zgn_gl_texture* proxy_ = nullptr;  // nonnull after initialization
};

inline zgn_gl_texture*
GlTexture::proxy()
{
  return proxy_;
}

std::unique_ptr<GlTexture> CreateGlTexture(Application* app);

}  // namespace zen::client
