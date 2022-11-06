#include "gl-buffer.h"

#include <zen-common.h>

#include "session.h"

struct znr_gl_buffer*
znr_gl_buffer_create(struct znr_session* session_base)
{
  auto self = new znr_gl_buffer();
  znr_session_impl* session;

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

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_buffer_destroy(struct znr_gl_buffer* self)
{
  delete self;
}
