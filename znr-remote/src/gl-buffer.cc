#include "gl-buffer.h"

#include <zen-common.h>

#include "loop.h"
#include "session.h"

void
znr_gl_buffer_data(struct znr_gl_buffer *self, uint32_t target,
    struct zgnr_mem_storage *storage, uint32_t usage)
{
  auto loop = std::make_unique<Loop>(wl_display_get_event_loop(self->display));

  zgnr_mem_storage_ref(storage);

  auto buffer = zen::remote::server::CreateBuffer(
      storage->data, [storage] { zgnr_mem_storage_unref(storage); },
      std::move(loop));

  self->proxy->GlBufferData(std::move(buffer), target, storage->size, usage);
}

struct znr_gl_buffer *
znr_gl_buffer_create(
    struct znr_session *session_base, struct wl_display *display)
{
  auto self = new znr_gl_buffer();
  znr_session_impl *session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateGlBuffer(session->proxy);
  if (!self->proxy) {
    zn_error("Failed to create remote gl buffer");
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
znr_gl_buffer_destroy(struct znr_gl_buffer *self)
{
  delete self;
}
