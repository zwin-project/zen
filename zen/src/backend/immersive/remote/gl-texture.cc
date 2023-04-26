#include "gl-texture.h"

#include "loop.h"
#include "zen-common/log.h"
#include "zen/buffer.h"

namespace zen::backend::immersive::remote {

const zn_gl_texture_interface GlTexture::c_implementation_ = {
    GlTexture::HandleImage2D,
    GlTexture::HandleGenerateMipmap,
};

GlTexture::~GlTexture()
{
  if (c_obj_ != nullptr) {
    zn_gl_texture_destroy(c_obj_);
  }
}

GlTexture::GlTexture(wl_display *display) : display_(display) {}

std::unique_ptr<GlTexture>
GlTexture::New(
    std::shared_ptr<zen::remote::server::IChannel> channel, wl_display *display)
{
  auto self = std::unique_ptr<GlTexture>(new GlTexture(display));
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel))) {
    zn_error("Failed to initialize a remote GlTexture");
    return nullptr;
  }

  return self;
}

bool
GlTexture::Init(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  c_obj_ = zn_gl_texture_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_texture");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlTexture(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlTexture");
    zn_gl_texture_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
GlTexture::HandleImage2D(struct zn_gl_texture *c_obj, uint32_t target,
    int32_t level, int32_t internal_format, uint32_t width, uint32_t height,
    int32_t border, uint32_t format, uint32_t type, struct zn_buffer *data)
{
  auto *self = static_cast<GlTexture *>(c_obj->impl_data);

  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(self->display_));

  auto remote_buffer = zen::remote::server::CreateBuffer(
      [data]() { return zn_buffer_begin_access(data); },
      [data]() { return zn_buffer_end_access(data); },
      [data]() { zn_buffer_unref(data); }, std::move(loop));

  if (!remote_buffer) {
    zn_abort("Failed to create remote buffer");
    return;
  }

  zn_buffer_ref(data);

  self->remote_obj_->GlTexImage2D(target, level, internal_format, width, height,
      border, format, type, std::move(remote_buffer));
}

void
GlTexture::HandleGenerateMipmap(struct zn_gl_texture *c_obj, uint32_t target)
{
  auto *self = static_cast<GlTexture *>(c_obj->impl_data);

  self->remote_obj_->GlGenerateMipmap(target);
}

}  // namespace zen::backend::immersive::remote
