#include "gl-texture.h"

#include <zen-common.h>

#include "loop.h"
#include "session.h"

void
znr_gl_texture_image_2d(struct znr_gl_texture *self, uint32_t target,
    int32_t level, int32_t internal_format, uint32_t width, uint32_t height,
    int32_t border, uint32_t format, uint32_t type,
    struct zgnr_mem_storage *storage)
{
  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(self->display));

  zgnr_mem_storage_ref(storage);

  auto buffer = zen::remote::server::CreateBuffer(
      storage->data, [storage] { zgnr_mem_storage_unref(storage); },
      std::move(loop));

  self->proxy->GlTexImage2D(target, level, internal_format, width, height,
      border, format, type, std::move(buffer));
}

struct znr_gl_texture *
znr_gl_texture_create(
    struct znr_session *session_base, struct wl_display *display)
{
  auto self = new znr_gl_texture();
  znr_session_impl *session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateGlTexture(session->proxy);
  if (!self->proxy) {
    zn_error("Failed to creat remote gl texture");
    goto err_delete;
  }

  self->display = display;

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_texture_destroy(struct znr_gl_texture *self)
{
  delete self;
}
