#include "gl-buffer.h"

#include "loop.h"
#include "zen-common/log.h"
#include "zen/buffer.h"

namespace zen::backend::immersive::remote {

const zn_gl_buffer_interface GlBuffer::c_implementation_ = {
    GlBuffer::HandleData,
};

GlBuffer::~GlBuffer()
{
  if (c_obj_ != nullptr) {
    zn_gl_buffer_destroy(c_obj_);
  }
}

GlBuffer::GlBuffer(wl_display *display) : display_(display) {}

std::unique_ptr<GlBuffer>
GlBuffer::New(
    std::shared_ptr<zen::remote::server::IChannel> channel, wl_display *display)
{
  auto self = std::unique_ptr<GlBuffer>(new GlBuffer(display));
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel))) {
    zn_error("Failed to initialize a remote GlBuffer");
    return nullptr;
  }

  return self;
}

bool
GlBuffer::Init(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  c_obj_ = zn_gl_buffer_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_buffer");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlBuffer(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlBuffer");
    zn_gl_buffer_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
GlBuffer::HandleData(struct zn_gl_buffer *c_obj, uint32_t target,
    struct zn_buffer *buffer, uint32_t usage)
{
  auto *self = static_cast<GlBuffer *>(c_obj->impl_data);

  ssize_t size = zn_buffer_get_size(buffer);

  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(self->display_));

  auto remote_buffer = zen::remote::server::CreateBuffer(
      [buffer]() { return zn_buffer_begin_access(buffer); },
      [buffer]() { return zn_buffer_end_access(buffer); },
      [buffer]() { zn_buffer_unref(buffer); }, std::move(loop));

  if (!remote_buffer) {
    zn_abort("Failed to create remote buffer");
    return;
  }

  zn_buffer_ref(buffer);

  self->remote_obj_->GlBufferData(
      std::move(remote_buffer), target, size, usage);
}

}  // namespace zen::backend::immersive::remote
