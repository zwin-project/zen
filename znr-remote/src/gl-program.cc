#include "gl-program.h"

#include <zen-common.h>

#include "dispatcher.h"
#include "gl-shader.h"

void
znr_gl_program_attach_shader(
    struct znr_gl_program *self, struct znr_gl_shader *shader)
{
  self->proxy->GlAttachShader(shader->proxy->id());
}

void
znr_gl_program_link(struct znr_gl_program *self)
{
  self->proxy->GlLinkProgram();
}

struct znr_gl_program *
znr_gl_program_create(struct znr_dispatcher *dispatcher_base)
{
  auto self = new znr_gl_program();
  znr_dispatcher_impl *dispatcher =
      zn_container_of(dispatcher_base, dispatcher, base);

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlProgram(dispatcher->channel);
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
znr_gl_program_destroy(struct znr_gl_program *self)
{
  delete self;
}
