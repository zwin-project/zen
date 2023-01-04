#include "gl-texture.h"

#include <zen-common.h>

#include "dispatcher.h"
#include "loop.h"

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

void
znr_gl_texture_generate_mipmap(struct znr_gl_texture *self, uint32_t target)
{
  self->proxy->GlGenerateMipmap(target);
}

struct znr_gl_texture *
znr_gl_texture_create(
    struct znr_dispatcher *dispatcher_base, struct wl_display *display)
{
  auto self = new znr_gl_texture();
  znr_dispatcher_impl *dispatcher =
      zn_container_of(dispatcher_base, dispatcher, base);

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlTexture(dispatcher->channel);
  if (!self->proxy) {
    zn_error("Failed to create remote gl texture");
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
