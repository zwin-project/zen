#pragma once

#include "gl-texture.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlTexture
{
 public:
  DISABLE_MOVE_AND_COPY(GlTexture);
  ~GlTexture();

  static std::unique_ptr<GlTexture> New(
      std::shared_ptr<zen::remote::server::IChannel> channel,
      wl_display *display);

  inline std::unique_ptr<zen::remote::server::IGlTexture> &remote_obj();
  inline zn_gl_texture *c_obj() const;

 private:
  explicit GlTexture(wl_display *display);

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  static void HandleImage2D(struct zn_gl_texture *c_obj, uint32_t target,
      int32_t level, int32_t internal_format, uint32_t width, uint32_t height,
      int32_t border, uint32_t format, uint32_t type, struct zn_buffer *data);

  static void HandleGenerateMipmap(
      struct zn_gl_texture *c_obj, uint32_t target);

  wl_display *display_;  // @nonnull, @outlive

  std::unique_ptr<zen::remote::server::IGlTexture> remote_obj_;

  static const zn_gl_texture_interface c_implementation_;

  zn_gl_texture *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline std::unique_ptr<zen::remote::server::IGlTexture> &
GlTexture::remote_obj()
{
  return remote_obj_;
}

inline zn_gl_texture *
GlTexture::c_obj() const
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
