#include "gl-program.h"

#include <zen-common.h>

#include "gl-shader.h"
#include "session.h"
void
znr_gl_program_attach_shader(
    struct znr_gl_program* self, struct znr_gl_shader* shader)
{
  self->proxy->GlAttachShader(shader->proxy->id());
}

void
znr_gl_program_link(struct znr_gl_program* self)
{
  self->proxy->GlLinkProgram();
}

struct znr_gl_program*
znr_gl_program_create(struct znr_session* session_base)
{
  auto self = new znr_gl_program();
  znr_session_impl* session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateGlProgram(session->proxy);
  if (!self->proxy) {
    zn_error("Failed to create remote gl program");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_program_destroy(struct znr_gl_program* self)
{
  delete self;
}
