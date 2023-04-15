#pragma once

#include "gl-buffer-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlBuffer
{
 public:
  DISABLE_MOVE_AND_COPY(GlBuffer);
  ~GlBuffer();

  static std::unique_ptr<GlBuffer> New(
      std::shared_ptr<zen::remote::server::IChannel> channel);

  inline zn_gl_buffer *c_obj() const;

 private:
  GlBuffer() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  std::unique_ptr<zen::remote::server::IGlBuffer> remote_obj_;

  zn_gl_buffer *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_buffer *
GlBuffer::c_obj() const
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
