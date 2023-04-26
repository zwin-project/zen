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
      std::shared_ptr<zen::remote::server::IChannel> channel,
      wl_display *display);

  inline zn_gl_buffer *c_obj() const;

 private:
  explicit GlBuffer(wl_display *display);

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  static void HandleData(struct zn_gl_buffer *c_obj, uint32_t target,
      struct zn_buffer *buffer, uint32_t usage);

  wl_display *display_;  // @nonnull, @outlive

  std::unique_ptr<zen::remote::server::IGlBuffer> remote_obj_;

  static const zn_gl_buffer_interface c_implementation_;

  zn_gl_buffer *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_buffer *
GlBuffer::c_obj() const
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
